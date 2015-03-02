/*******************************************************************************
 *
 * Intel Thunderbolt(TM) daemon
 * Copyright(c) 2014 - 2015 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * Intel Thunderbolt Mailing List <thunderbolt-software@lists.01.org>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/

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
		case APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE_CODE:
		{
			TbtServiceLogger::LogInfo("FW response: APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE, Controller: %s", WStringToString(cId).c_str());
			std::shared_ptr<APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE> pApproveInterDomainConnectionResponse = GetBufStruct<APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE>(Msg);
			ConnectionManager::GetInstance()->OnApproveInterDomainConnectionResponse(cId, *pApproveInterDomainConnectionResponse);
			break;
		}
		case INTER_DOMAIN_PACKET_SENT_RESPONSE_CODE:
			TbtServiceLogger::LogInfo("FW response: INTER_DOMAIN_PACKET_SENT_RESPONSE, Controller: %s", WStringToString(cId).c_str());
			break;
		default:
			TbtServiceLogger::LogInfo("FW response: Unknown response code (%u0), Controller: %s", Msg[MESSAGE_CODE_OFFSET], WStringToString(cId).c_str());
			throw TbtException("Unknown response from FW");
			break;
	}
}

/**
 * this function is handling notifications that received from the FW
 */
void MessageFromFwEvent::HandleFwToSwNotification(controlleriD cId, const std::vector<uint8_t>& Msg)
{
	switch (Msg[MESSAGE_CODE_OFFSET])
	{
		case INTER_DOMAIN_CONNECTED_NOTIFICATION_CODE:
		{

			TbtServiceLogger::LogInfo("FW Notification: INTER_DOMAIN_CONNECTED_NOTIFICATION, Controller: %s", WStringToString(cId).c_str());
			std::shared_ptr<INTER_DOMAIN_CONNECTED_NOTIFICATION> pInterDomainConnectedNotification = GetBufStruct<INTER_DOMAIN_CONNECTED_NOTIFICATION>(Msg);
			std::thread([](std::shared_ptr<INTER_DOMAIN_CONNECTED_NOTIFICATION> data, controlleriD controller_id){
				ConnectionManager::GetInstance()->OnInterDomainConnect(controller_id, *data);
			},pInterDomainConnectedNotification,cId).detach();
			break;
		}
		case INTER_DOMAIN_DISCONNECTED_NOTIFICATION_CODE:
		{
			TbtServiceLogger::LogInfo("FW Notification: INTER_DOMAIN_DISCONNECTED_NOTIFICATION, Controller: %s", WStringToString(cId).c_str());
			std::shared_ptr<INTER_DOMAIN_DISCONNECTED_NOTIFICATION> pInterDomainDisconnectedNotification = GetBufStruct<INTER_DOMAIN_DISCONNECTED_NOTIFICATION>(Msg);
			ConnectionManager::GetInstance()->OnInterDomainDisconnected(cId, *pInterDomainDisconnectedNotification);
		}
			break;
		default:
			TbtServiceLogger::LogWarning("Warning: FW Notification: Unknown, Controller: %s", WStringToString(cId).c_str());
			throw TbtException( (std::string("Unknown notification from FW") + std::to_string(Msg[MESSAGE_CODE_OFFSET])).c_str());
	}
}
