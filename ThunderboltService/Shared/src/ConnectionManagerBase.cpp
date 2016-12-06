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

#include <cinttypes>
#include "ConnectionManagerBase.h"
#include "Controller.h"
#include "Port.h"
#include "P2PDevice.h"
#include "Utils.h"
#include "ControllerSettings.h"
#include "ControllerDetails.h"
#include "tbtException.h"
#include "MessageUtils.h"

ConnectionManagerBase::ConnectionManagerBase(std::shared_ptr<IControllerCommandSender>& sender)
   : m_ControllerCommandSender(sender),
     m_isPreShutdownMode(false),
     m_ControllersSettings(new ControllerSettings(false, false, false)), // TODO: put default in the settings?
     m_DuringPowerCycle(false),
     m_DuringFwUpadte(false)
{
}

/**
 * this callback is called when a physical connection is made between the two Thunderbolt hosts ( Windows <--> Linux
 * <--> Apple)
 */
void ConnectionManagerBase::OnInterDomainConnect(const controlleriD& ControllerId,
                                                 const INTER_DOMAIN_CONNECTED_NOTIFICATION& e)
{
   auto PortNum = TBT_GET_PORT_BY_LINK(e.Link);
   TbtServiceLogger::LogInfo(
      "Inter domain connected, Controller: %s, Port %d", WStringToString(ControllerId).c_str(), PortNum);

   std::shared_ptr<IController> pController;
   {
      std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
      // Check that the controller id does exist in the controllers map
      if (m_Controllers.find(ControllerId) == m_Controllers.end())
      {
         TbtServiceLogger::LogError(
            "Error: Inter domain connected to a controller that doesn't exist in the controllers' "
            "map in the connection manager");
         throw TbtException("Inter domain connected to a controller that doesn't exist in the controllers' map in the "
                            "connection manager");
      }
      pController = m_Controllers[ControllerId];
      // Drop lock
   }

   // Create a port if it doesn't exist
   if (!pController->HasPort(PortNum))
   {
      pController->AddPort(PortNum, std::make_shared<Port>(ControllerId, PortNum));
   }
   const auto pPort = pController->GetPortAt(PortNum);

   auto pP2PDeviceToAdd = CreateP2PDevice(ControllerId, e);

   pController->AddP2PDevice(PortNum, pP2PDeviceToAdd);
}

/**
 * this callback is called when the cable between two Thunderbolt hosts is disconnected
 */
void ConnectionManagerBase::OnInterDomainDisconnected(const controlleriD& ControllerId,
                                                      const INTER_DOMAIN_DISCONNECTED_NOTIFICATION& e)
{
   TbtServiceLogger::LogInfo("Inter domain disconnected: Controller: %s", WStringToString(ControllerId).c_str());

   std::shared_ptr<IController> pController;
   {
      std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
      if (m_Controllers.find(ControllerId) == m_Controllers.end())
      {
         TbtServiceLogger::LogWarning("Warning: Inter domain disconnection at a controller that doesn't exist");
         return;
      }
      pController = m_Controllers.at(ControllerId);
      // Drop lock
   }

   uint32_t PortNum = TBT_GET_PORT_BY_LINK(e.Link);
   if (!pController->HasPort(PortNum))
   {
      TbtServiceLogger::LogWarning("Warning: Inter domain disconnection at a port that doesn't exist");
      return;
   }
   const auto pPort = pController->GetPortAt(PortNum);

   pPort->removeP2PDevice();
}

/**
 * this callback is the first message that received from the FW, it contains some details about the connected
 * controller. After this message is processed the daemon will receive messages about other devices and P2P
 */
void ConnectionManagerBase::OnDriverReadyResponse(const controlleriD& cId, const DRIVER_READY_RESPONSE& e)
{
   TbtServiceLogger::LogInfo("New controller detected: %s ", WStringToString(cId).c_str());
   // if controller is not exist we creating it (should happen only first time ever TBT device attached)
   std::shared_ptr<IController> pController;
   {
      std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
      if (m_Controllers.find(cId) == m_Controllers.end())
      {
         TbtServiceLogger::LogDebug("Create new controller class");
         m_Controllers[cId] = shared_ptr<IController>(new Controller(cId, e));
      }
      pController = m_Controllers.at(cId);
      // Drop lock
   }

   // Reset controller semaphore in case we are after FW update, when we do power cycle and don't
   // want to count on FW response to the power cycle command, but on the other hand have to handle
   // the cases where we do not get such a response
   pController->GetFWUpdateResponseLock().ResetSemaphore();

   pController->GetControllerData()->SetSecurityLevel(e.SecurityLevel);

   // Get the current settings from the controller
   std::shared_ptr<ControllerSettings> ContSett(new ControllerSettings(
      !e.AllowAnyThunderboltDevice, e.Allow1stDepthDevicesToConnectAtAnyDepth, e.AllowDockDevicesToConnectAtAnyDepth));

   auto details = pController->GetControllerData();
   TbtServiceLogger::LogInfo("Controller device ID: %X, TBT generation: %d, port count: %d, security level: %d",
                             GetDeviceIDFromControllerID(cId),
                             details->GetGeneration(),
                             details->GetNumOfPorts(),
                             details->GetSecurityLevel());
   TbtServiceLogger::LogInfo(
      "Controller settings: Certified only: %s, Override first depth: %s, Allow dock at any depth: %s",
      BoolToString(ContSett->GetCertifiedOnly()).c_str(),
      BoolToString(ContSett->GetOverrideFirstDepth()).c_str(),
      BoolToString(ContSett->GetAllowDockAtAnyDepth()).c_str());

   QueryDriverInformation(cId);
}

/**
 * this callback is called before the daemon is shutting down
 */
void ConnectionManagerBase::OnServiceDown()
{
   if (m_ExitCallback)
   {
      TbtServiceLogger::LogInfo("service stopped");
      m_ExitCallback();
   }
}

/**
 * returns controller by its id
 */
shared_ptr<IController> ConnectionManagerBase::GetController(const controlleriD& Cid) const
{
   shared_ptr<IController> pController = nullptr;

   try
   {
      std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
      pController = m_Controllers.at(Cid);
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: Unable to find the requested controller");
      throw;
   }

   return pController;
}

// In this case, the FW only responds to FW update. For this we only need the controller ID, so a controller is
// created with minimal data.
void ConnectionManagerBase::OnFwIsInSafeMode(const controlleriD& cId)
{
   m_DuringPowerCycle = false;
   TbtServiceLogger::LogInfo("************  NEW SAFE MODE controller detected: %s ", WStringToString(cId).c_str());
   TbtServiceLogger::LogInfo(
      "Controller %s is in safe mode. Only FW update action is possible, no devices will be connected.",
      WStringToString(cId).c_str());

   {
      std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
      // if controller is not exist we creating it (should happen only first time ever TBT device attached)
      if (m_Controllers.find(cId) == m_Controllers.end())
      {
         TbtServiceLogger::LogDebug("Create new controller class");
         m_Controllers[cId] = std::unique_ptr<IController>(new Controller(cId));
      }
      if (!m_Controllers.at(cId)->GetControllerData()->GetIsInSafeMode())
      {
         m_Controllers.at(cId)->SetControllerData(std::shared_ptr<ControllerDetails>(new ControllerDetails()));
      }
      m_Controllers.at(cId)->SetIsUp(true);
   }

#if 0
   { // this is to prevent FW update tool from getting stuck when it waits for all controllers to be up.
      std::lock_guard<std::mutex> lk(m_WaitForDriverReadyMutex);
      m_WaitForDriverReadyTimeout.notify_all();
   }
#endif
   QueryDriverInformation(cId);
   // StoreControllerData(*m_Controllers.at(cId));
   // StoreSWDetails();
}

/**
 * returns all controllers known to the daemon
 */
map<controlleriD, shared_ptr<IController>> ConnectionManagerBase::GetControllers() const
{
   std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
   return m_Controllers;
}

IControllerCommandSender& ConnectionManagerBase::GetControllerCommandSender() const
{
   return *m_ControllerCommandSender;
}
// FIXME: Do we need lock here?
void ConnectionManagerBase::SetFwUpdateStatus(bool isInFwUpdateProcess)
{
   m_DuringFwUpadte = isInFwUpdateProcess;
   TbtServiceLogger::LogInfo("FwUpdateStatus = %s", isInFwUpdateProcess ? "Active" : "Not Active");
}

bool ConnectionManagerBase::IsDuringFwUpdate() const noexcept
{
   return m_DuringFwUpadte;
}

/**
 * returns p2p device by requested port
 */
const std::shared_ptr<P2PDevice> ConnectionManagerBase::GetP2PDevice(controlleriD cId, uint32_t portNum) const
{
   std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
   return m_Controllers.at(cId)->GetPortAt(portNum)->getP2PDevice();
}

// FIXME: Remove duplicate code
std::shared_ptr<P2PDevice> ConnectionManagerBase::GetP2PDevice(controlleriD cId, uint32_t portNum)
{
   std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
   return m_Controllers.at(cId)->GetPortAt(portNum)->getP2PDevice();
}

/**
 * registering exit callback that will be executed to stop the daemon activity.
 * usually used to exit third party event loop
 */
void ConnectionManagerBase::RegisterExitCallback(std::function<void()> ExitCallback)
{
   m_ExitCallback = ExitCallback;
}

/**
 * this callback will be executed before the system is shutting down or restarted.
 * Note: will not be called when daemon only is restarted / stoped
 */
void ConnectionManagerBase::OnSystemPreShutdown()
{
   try
   {
      // setting pre shutdown mode
      setPreShutdownMode();

      for (auto controller : GetControllers())
      {
         for (auto port : controller.second->GetPorts())
         {
            if (port.second->hasP2PDevice())
            {
               port.second->getP2PDevice()->OnSystemPreShutdown();
            }
         }
      }
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: Failed sending properties change notification %s", e.what());
      throw;
   }
   catch (...)
   {
      TbtServiceLogger::LogError("Error: Failed sending properties change notification");
      throw;
   }
}

/**
 * returns the policy for all the controllers
 */
std::shared_ptr<ControllerSettings> ConnectionManagerBase::GetControllersSettings() const
{
   return m_ControllersSettings;
}

/**
 * setting policy for all the controllers
 */
void ConnectionManagerBase::SetControllersSettings(const std::shared_ptr<ControllerSettings> Settings)
{
   m_ControllersSettings = Settings;
}

#include "GenlWrapper.h"

/**
 * this function is getting the information from the driver
 */
void ConnectionManagerBase::QueryDriverInformation(const controlleriD& Cid)
{
   using std::wstring;
   using std::to_wstring;

   try
   {
      GenlWrapper::Instance().send_message_sync(ControllerIDToToInt(Cid), NHI_CMD_QUERY_INFORMATION);
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: ConnectionManagerBase::QueryDriverInformation failed! Error : %s", e.what());
   }
   catch (...)
   {
      TbtServiceLogger::LogError("Error: ConnectionManagerBase::QueryDriverInformation failed! Error : Unknown error");
   }
}
