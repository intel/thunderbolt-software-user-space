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

#include "MessageFromFwEvent.h"
#include "ConnectionManager.h"
#include "logger.h"
#include "Utils.h"
#include "tbtException.h"

/**
 * this function is handling response received from the FW
 */
void MessageFromFwEvent::HandleFwToSwResponse(controlleriD cId, const std::vector<uint8_t>& Msg)
{
   switch (static_cast<uint8_t>(Msg[MESSAGE_CODE_OFFSET]))
   {
   case DRIVER_READY_RESPONSE_CODE:
   {
      TbtServiceLogger::LogInfo("FW response: DRIVER_READY_RESPONSE, Controller: %s", WStringToString(cId).c_str());
      std::shared_ptr<DRIVER_READY_RESPONSE> pDrvRdyRes = GetBufStruct<DRIVER_READY_RESPONSE>(Msg);
      ConnectionManager::GetInstance()->OnDriverReadyResponse(cId, *pDrvRdyRes);
      break;
   }
   case I2C_REGISTER_ACCESS_RESPONSE_CODE:
   {
      TbtServiceLogger::LogInfo("Ignoring FW response: I2C_REGISTER_ACCESS_RESPONSE, Controller: %s",
                                WStringToString(cId).c_str());
   }
   break;

   case APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE_CODE:
   {
      TbtServiceLogger::LogInfo("FW response: APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE, Controller: %s",
                                WStringToString(cId).c_str());
      std::shared_ptr<APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE> pApproveInterDomainConnectionResponse =
         GetBufStruct<APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE>(Msg);
      ConnectionManager::GetInstance()->OnApproveInterDomainConnectionResponse(cId,
                                                                               *pApproveInterDomainConnectionResponse);
      break;
   }
   case INTER_DOMAIN_PACKET_SENT_RESPONSE_CODE:
      TbtServiceLogger::LogInfo("FW response: INTER_DOMAIN_PACKET_SENT_RESPONSE, Controller: %s",
                                WStringToString(cId).c_str());
      break;
   default:
      TbtServiceLogger::LogInfo("FW response: Unknown response code (%u0), Controller: %s",
                                Msg[MESSAGE_CODE_OFFSET],
                                WStringToString(cId).c_str());
      throw TbtException("Unknown response from FW");
      break;
   }
}

/**
 * this function is handling notifications that received from the FW
 */
void MessageFromFwEvent::HandleFwToSwNotification(controlleriD cId, const std::vector<uint8_t>& Msg)
{
   if (ConnectionManager::GetInstance()->IsDuringFwUpdate())
   {
      TbtServiceLogger::LogDebug("FW in progress, ignoring message");
      return;
   }

   switch (Msg[MESSAGE_CODE_OFFSET])
   {
   case INTER_DOMAIN_CONNECTED_NOTIFICATION_CODE:
   {

      TbtServiceLogger::LogInfo("FW Notification: INTER_DOMAIN_CONNECTED_NOTIFICATION, Controller: %s",
                                WStringToString(cId).c_str());
      std::shared_ptr<INTER_DOMAIN_CONNECTED_NOTIFICATION> pInterDomainConnectedNotification =
         GetBufStruct<INTER_DOMAIN_CONNECTED_NOTIFICATION>(Msg);
      std::thread(
         [](std::shared_ptr<INTER_DOMAIN_CONNECTED_NOTIFICATION> data, controlleriD controller_id) {
            ConnectionManager::GetInstance()->OnInterDomainConnect(controller_id, *data);
         },
         pInterDomainConnectedNotification,
         cId)
         .detach();
      break;
   }
   case INTER_DOMAIN_DISCONNECTED_NOTIFICATION_CODE:
   {
      TbtServiceLogger::LogInfo("FW Notification: INTER_DOMAIN_DISCONNECTED_NOTIFICATION, Controller: %s",
                                WStringToString(cId).c_str());
      std::shared_ptr<INTER_DOMAIN_DISCONNECTED_NOTIFICATION> pInterDomainDisconnectedNotification =
         GetBufStruct<INTER_DOMAIN_DISCONNECTED_NOTIFICATION>(Msg);
      ConnectionManager::GetInstance()->OnInterDomainDisconnected(cId, *pInterDomainDisconnectedNotification);
   }
   break;
   default:
      TbtServiceLogger::LogWarning("Warning: FW Notification: Unknown, Controller: %s", WStringToString(cId).c_str());
      throw TbtException(
         (std::string("Unknown notification from FW") + std::to_string(Msg[MESSAGE_CODE_OFFSET])).c_str());
   }
}
