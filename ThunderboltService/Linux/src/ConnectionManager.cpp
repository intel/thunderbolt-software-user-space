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
#include <unistd.h>
#include <stdlib.h>
#include "P2PDevice.h"
#include "ConnectionManager.h"
#include "ConnectionManagerBase.h"
#include "LinuxControllerCommandSender.h"
#include "ControllerSettings.h"
#include "Utils.h"
#include "logger.h"

/**
 * return a shared_ptr for the connection manager singleton
 */
std::shared_ptr<ConnectionManager> ConnectionManager::GetInstance()
{
   static std::shared_ptr<ConnectionManager> Instance =
      std::make_shared<ConnectionManager>(std::make_shared<LinuxControllerCommandSender>());
   return Instance;
}

/**
 * constructor - initialize controller command sender
 */
ConnectionManager::ConnectionManager(std::shared_ptr<IControllerCommandSender> sender) : ConnectionManagerBase(sender)
{
}

DBus::Connection* ConnectionManager::GetDBusConnection()
{
   return m_pDBus;
}

void ConnectionManager::SetDBusConnection(DBus::Connection* pCx)
{
   m_pDBus = pCx;
}

/**
 * sending command to FW through the Thunderbolt driver
 */
void ConnectionManager::SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd)
{
   auto pControllerP = GetController(cID);

   if (pControllerP->IsUp())
   {
      GetControllerCommandSender().SendDriverCommand(cID, cmd);
      if (IS_SETTINGS_CMD(cmd)) // FIXME: add other commands
      {
         auto port = pControllerP->GetDmaPort();
         GetControllerCommandSender().SendPowerCycleCommand(cID, port, ROUTE_STRING{});
         // SendPowerCycleNotification();
      }
   }
   else
   {
      TbtServiceLogger::LogWarning("Warning: Driver is not up");
   }
}

/**
 * sending request for driver information
 */
void ConnectionManager::QueryDriverInformation(const controlleriD& Cid)
{
   std::thread([this](controlleriD Cid) { ConnectionManagerBase::QueryDriverInformation(Cid); }, Cid).detach();
}

/**
 * getting the localhost name
 */
std::string ConnectionManager::GetLocalHostUTF8ComputerName() const
{
   char buff[HOST_NAME_MAX];
   if (gethostname(buff, HOST_NAME_MAX) == -1)
   {
      TbtServiceLogger::LogError("Error: Fail resolving hostname. errno= %s", strerror(errno));
      throw TbtException("Fail resolving hostname");
   }

   return buff;
}

/**
 * the function creates P2P device
 */
std::shared_ptr<P2PDevice> ConnectionManager::CreateP2PDevice(const controlleriD& ControllerId,
                                                              const INTER_DOMAIN_CONNECTED_NOTIFICATION& e) const
{
   std::unique_lock<std::mutex> ctrl_lock(m_ControllersGuard);
   const auto pController = m_Controllers.at(ControllerId);

   return std::make_shared<P2PDevice>(ControllerId,
                                      e.RemoteHostRouterUniqueID,
                                      e.LocalRouteString,
                                      e.LocalHostRouterUniqueID,
                                      GetLocalHostUTF8ComputerName(),
                                      static_cast<uint8_t>(e.Depth),
                                      (e.Approve == 1),
                                      pController->GetSupportsFullE2E());
}

/**
 * this function is handling inter domain connect message (occurs when cable is connected between two hosts)
 */
void ConnectionManager::OnInterDomainConnect(const controlleriD& ControllerId,
                                             const INTER_DOMAIN_CONNECTED_NOTIFICATION& e)
{
   ConnectionManagerBase::OnInterDomainConnect(ControllerId, e);
   GetP2PDevice(ControllerId, TBT_GET_PORT_BY_LINK(e.Link))->StartHandshake();
   GetP2PDevice(ControllerId, TBT_GET_PORT_BY_LINK(e.Link))->SendPropertiesChangeRequest();
}

/**
 * this function is called when FW built the P2P rx/tx path.
 * in this case the P2P device is marked as "tunneled"
 */
void ConnectionManager::OnApproveInterDomainConnectionResponse(const controlleriD& cId,
                                                               const APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE& e)
{
   try
   {
      if (e.ErrorFlag)
      {
         TbtServiceLogger::LogError(
            "Error: Approve inter domain connection response for Controller %s returned with an error flag",
            WStringToString(cId).c_str());
         return;
      }
      GetP2PDevice(cId, TBT_GET_PORT_BY_LINK(e.LocalLink))->SetState(P2P_STATE::TUNNEL);
   }
   catch (const std::exception& ex)
   {
      TbtServiceLogger::LogError("Error: Exception: %s", ex.what());
   }
   catch (...)
   {
      TbtServiceLogger::LogError("Error: OnApproveInterDomainConnectionResponse: Unknown error, Controller: %s",
                                 WStringToString(cId).c_str());
   }
}
