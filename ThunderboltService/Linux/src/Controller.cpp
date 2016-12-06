/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2014 - 2016 Intel Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Intel Corporation nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#include <memory>
#include <thread>
#include <string>
#include "Port.h"
#include "Controller.h"
#include "ConnectionManager.h"
#include "ControllerDetails.h"
#include "UniqueID.h"
#include "tbtException.h"
#include "DBusController.h"
#include "ScopeGuard.h"
#include "MessageUtils.h"
#include "defines.h"

// Safe mode constructor.
Controller::Controller(controlleriD id)
   : m_Id(id),
     m_ControllerDetails(std::make_shared<ControllerDetails>()),
     m_Ports(),
     m_DmaPort(-1),   // Uninitialized
     m_NvmOffset(-1), // Uninitialized
     m_IsUp(false),
     m_isReady(false),
     // m_IsFirstConnected(false),
     m_FWUpdateResponseLock(std::chrono::minutes(1))
{
   auto cmgr             = ConnectionManager::GetInstance();
   DBus::Connection* pCx = cmgr->GetDBusConnection();
   if (pCx)
   {
      m_pDBusWrapper.reset(new DBusController(*pCx, *this));
   }
}

Controller::Controller(controlleriD id, DRIVER_READY_RESPONSE controllerData)
   : m_Id(id),
     m_ControllerDetails(
        new ControllerDetails(controllerData, GetGenerationFromControllerID(id), GetNomOfPortsFromControllerID(id))),
     m_Ports(),
     m_DmaPort(-1),   // Uninitialized
     m_NvmOffset(-1), // Uninitialized
     m_IsUp(false),
     m_isReady(false),
     m_pDBusWrapper(),
     m_FWUpdateResponseLock(std::chrono::minutes(1))
{
   auto cmgr             = ConnectionManager::GetInstance();
   DBus::Connection* pCx = cmgr->GetDBusConnection();
   if (pCx)
   {
      m_pDBusWrapper.reset(new DBusController(*pCx, *this));
   }
   // TODO: create new Ports?
   m_ControllerDetails->SetFWVersion(std::to_wstring(controllerData.FwRamVersion) + L"."
                                     + std::to_wstring(controllerData.FwRomVersion));
   m_ControllerDetails->SetSecurityLevel(controllerData.SecurityLevel);
}

DBusController* Controller::GetDBusWrapper()
{
   return m_pDBusWrapper.get();
}

/**
 * return identifier of the controller
 */
const controlleriD& Controller::GetControllerID() const
{
   return m_Id;
}

void Controller::SendDriverCommand(SW_TO_FW_INMAILCMD_CODE code)
{
   auto cmgr = ConnectionManager::GetInstance();
   cmgr->SendDriverCommand(GetControllerID(), code);
}

uint32_t Controller::ControllerFwUpdate(const std::vector<uint8_t>& fwImage)
{
   TBTDRV_STATUS status;

   if (ConnectionManager::GetInstance()->IsDuringFwUpdate())
   {
      return uint32_t(TBTDRV_STATUS::SDK_IN_USE);
   }
   try
   {
      if (m_ControllerDetails->GetGeneration() < ThunderboltGeneration::THUNDERBOLT_3)
      {
         return uint32_t(TBTDRV_STATUS::CONTROLLER_NOT_SUPPORTED);
      }
      ConnectionManager::GetInstance()->SetFwUpdateStatus(true);
      m_inFWUpdate          = true;
      const auto& flagGuard = Utils::makeScopeGuard([this] { m_inFWUpdate = false; });

      if (!GetControllerData()->GetIsInSafeMode())
      {
         // DisconnectPCIePaths
         // FIXME: change to check the result of the command
         TbtServiceLogger::LogDebug("FwUpdate: Disconnecting PCI-E paths");
         ConnectionManager::GetInstance()->SendDriverCommand(
            m_Id, SW_TO_FW_INMAILCMD_CODE::DISCONNECT_PCIE_PATHS_COMMAND_CODE);

         // Disconnect DMA paths (if P2P exists)

         for (auto portPair : m_Ports)
         {
            if (portPair.second->hasP2PDevice())
            {
               ConnectionManager::GetInstance()->SendDriverCommand(
                  m_Id,
                  (SW_TO_FW_INMAILCMD_CODE)((int)SW_TO_FW_INMAILCMD_CODE::DISCONNECT_PORT_A_INTER_DOMAIN_PATH
                                            + portPair.first));
               // After update is done, the FW should send a notification about this P2P device
               portPair.second->removeP2PDevice();
            }
         }
      }

      status = DoFwUpdate(fwImage, ROUTE_STRING{}, GetDmaPort());
   }
   catch (...)
   {
      // Allow these to propagate outward.  The DBus controller wrapper
      // does a nice job wrapping any what() string into the response
      // to the client, which makes problems easier to diagnose.
      throw;
   }

   ConnectionManager::GetInstance()->SetFwUpdateStatus(false);
   return uint32_t(status);
}

TBTDRV_STATUS Controller::DoFwUpdate(const std::vector<uint8_t>& Buffer, const ROUTE_STRING& routeString, int dmaPort)
{
   // ImageWrite ( if failed do power reset)
   TbtServiceLogger::LogInfo("Starting flash image to FW");

   // Don't think we need this --- another guard is kept in ControllerFwUpdate.
   // m_inFWUpdate          = true;
   // const auto& flagGuard = Utils::makeScopeGuard([this] { m_inFWUpdate = false; });

   TBTDRV_STATUS status = ConnectionManager::GetInstance()->GetControllerCommandSender().WriteNVMImage(
      m_Id,
      GetControllerData()->GetIsInSafeMode(),
      Buffer.size(),
      Buffer,
      routeString,
      dmaPort,
      GetControllerData()->GetGeneration());
   if (status != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
   {
      TbtServiceLogger::LogError("Error: WriteNVMImage failed (status: %u)", status);
   }

   if (!GetControllerData()->GetIsInSafeMode() && status == TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
   {
      // Authentication
      TbtServiceLogger::LogInfo("Starting flash authentication");
      GetControllerData()->SetNVMVersion(Version());
      status =
         ConnectionManager::GetInstance()->GetControllerCommandSender().FlashAuthentication(m_Id, routeString, dmaPort);
      if (status != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
      {
         TbtServiceLogger::LogError("Error: FlashAuthentication failed (status: %u)", status);
      }
   }

   try
   {
      // doing power cycle in both cases, success or failed
      ConnectionManager::GetInstance()->GetControllerCommandSender().SendPowerCycleCommand(m_Id, dmaPort, routeString);

      // Avoid reporting success til the controller comes back.  Simply
      // sleeping is obviously not as nice as actually observing the state,
      // but we don't want to change this up at the moment.  (The windows
      // service implements the same behavior here.)
      const int powerCycleSleep = 5;
      for (int i = 0; i < powerCycleSleep; ++i)
      {
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: Power cycle failed with exception: %s", e.what());
      status = TBTDRV_STATUS::POWER_CYCLE_FAILED_CODE;
   }
   catch (...)
   {
      TbtServiceLogger::LogError("Error: Power cycle failed, unknown error");
      status = TBTDRV_STATUS::POWER_CYCLE_FAILED_CODE;
   }
   return status;
}

uint32_t Controller::GetNVMVersion(Version& nvmVersion, bool noFWUcheck) const
{
   if (!noFWUcheck && ConnectionManager::GetInstance()->IsDuringFwUpdate())
   {
      return uint32_t(TBTDRV_STATUS::SDK_IN_USE);
   }
   nvmVersion = GetControllerData()->GetNVMVersion();
   if (nvmVersion.wstr() != Version::NA)
   {
      return uint32_t(TBTDRV_STATUS::SUCCESS_RESPONSE_CODE);
   }
   std::vector<uint8_t> tmp;
   auto& controllerCommandSender = ConnectionManager::GetInstance()->GetControllerCommandSender();
   TBTDRV_STATUS status =
      controllerCommandSender.ReadBlockFromFlash(m_Id, tmp, m_NvmOffset, 1, ROUTE_STRING{}, GetDmaPort());
   if (status == TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
   {
      nvmVersion = Version(tmp.at(m_NvmOffset % sizeof(uint32_t)), tmp.at((m_NvmOffset - 1) % sizeof(uint32_t)));
      if (*nvmVersion.major() == 0 && *nvmVersion.minor() == 0)
      {
         // Odd case we get sometimes during an intermediate phase after
         // a FW update, after the driver ready response, for just a few
         // seconds.  Return it but don't cache it.
      }
      else
      {
         GetControllerData()->SetNVMVersion(nvmVersion);
      }
   }
   return uint32_t(status);
}

uint32_t
Controller::ReadFirmware(uint32_t offset, uint32_t /* length -- not used at the moment */, std::vector<uint8_t>& data)
{
   if (ConnectionManager::GetInstance()->IsDuringFwUpdate())
   {
      return uint32_t(TBTDRV_STATUS::SDK_IN_USE);
   }
   // TODO: we always ignore the length that we get and instead - we always require 1 DWORD.
   // data.resize(1,0); //save space for data
   try
   {
#if 0
      std::unique_lock<std::mutex> lock(gSdkLocker, std::try_to_lock);
      if (!lock.owns_lock())
      {
         return static_cast<uint32_t>(TBTDRV_STATUS::SDK_IN_USE);
      }
#endif

      if (GetControllerData()->GetIsInSafeMode())
      {
         return static_cast<uint32_t>(TBTDRV_STATUS::INVALID_OPERATION_IN_SAFE_MODE);
      }
      auto connectionManager = ConnectionManager::GetInstance();
      std::wstring temp      = RemoveoDoubleSlash(m_Id);
      return static_cast<uint32_t>(connectionManager->GetControllerCommandSender().ReadBlockFromFlash(
         temp, data, offset, 1, ROUTE_STRING{}, connectionManager->GetController(temp)->GetDmaPort()));
   }
   catch (...)
   {
      // We used to translate this into
      // TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE.  But we would prefer for
      // what() to propagate upward a level to the D-Bus layer, where it can
      // be passed onward to the client.
      throw;
   }
}

/**
 * return the security level of the controller
 */
SECURITY_LEVEL Controller::GetSecurityLevel() const
{
   return (SECURITY_LEVEL)m_ControllerDetails->GetSecurityLevel();
}

/**
 * return a map of ports of this controller
 */
const PortsMap& Controller::GetPorts() const
{
   return m_Ports;
}

/**
 * return specific port of this controller
 */
shared_ptr<Port> Controller::GetPortAt(uint32_t position) const
{
   if (!HasPort(position))
   {
      TbtServiceLogger::LogError("Error: Trying to get port number %u which doesn't exist", position);
      throw TbtException("Trying to get port with number that doesn't exists");
   }

   return m_Ports.at(position);
}

/**
 * adding new port to this controller
 */
void Controller::AddPort(uint32_t position, std::shared_ptr<Port> PortToAdd)
{
   if (HasPort(position))
   {
      TbtServiceLogger::LogWarning("Warning: Trying to add port number %u which already exists", position);
   }

   m_Ports[position] = PortToAdd;
}

/**
 * return num of ports that this controller have
 */
int Controller::GetNumOfPorts() const
{
   return m_Ports.size();
}

/**
 * returning this controller details object
 */
shared_ptr<ControllerDetails> Controller::GetControllerData() const
{
   return m_ControllerDetails;
}

/**
 * updating this controller details
 */
void Controller::SetControllerData(shared_ptr<ControllerDetails> ControllerData)
{
   m_ControllerDetails = ControllerData;
}

/**
 * return the controller DMA port
 */
int Controller::GetDmaPort() const
{
   return m_DmaPort;
}

/**
 * set the DMA port for this controller
 */
void Controller::SetDmaPort(int port)
{
   m_DmaPort = port;
}

bool Controller::GetSupportsFullE2E()
{
   return m_SupportsFullE2E;
}

void Controller::SetSupportsFullE2E(bool fullE2ESupport)
{
   m_SupportsFullE2E = fullE2ESupport;
}

/**
 * clear all the ports of this controller
 */
void Controller::Clear()
{
   m_Ports.clear();
   TbtServiceLogger::LogDebug("Controller %s ports cleared", WStringToString(m_Id).c_str());
}

/**
 * check if a port exists in this controller
 */
bool Controller::HasPort(uint32_t PortNum) const
{
   return m_Ports.find(PortNum) != m_Ports.end();
}

/**
 * this function add a p2p device under this controller in the specified port
 */
void Controller::AddP2PDevice(uint32_t Port, std::shared_ptr<P2PDevice> pDevice)
{
   TbtServiceLogger::LogInfo("Adding P2P device to controller %s", WStringToString(m_Id).c_str());

   if (!HasPort(Port))
   {
      TbtServiceLogger::LogError("Error: Port %u does not exist!", Port);
      throw TbtException("Port does not exist!");
   }

   GetPortAt(Port)->setP2PDevice(pDevice);
}

void Controller::SetNvmVersionOffset(uint32_t address)
{
   m_NvmOffset = address;
}

/**
 * set the power mode of the controller
 */
void Controller::SetIsUp(bool isUp)
{
   m_IsUp = isUp;
   if (!isUp)
   {
      Clear();
   }
}

DriverCommandResponseLock& Controller::GetFWUpdateResponseLock()
{
   return m_FWUpdateResponseLock;
}
