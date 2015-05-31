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


#include <cstring>
#include <memory>
#include <thread>
#include <sstream>
#include <future>
#include <unistd.h>
#include <stdlib.h>
#include <mutex>
#include "LinuxControllerCommandSender.h"
#include "TbtWmiMessageToFW.h"
#include "GenetlinkInterface.h"
#include "logger.h"
#include "tbtException.h"
#include "GenlWrapper.h"
#include "MessageFromFwEvent.h"
#include "ConnectionManager.h"
#include "Utils.h"
#include "TbtWmiApproveP2P.h"
#include "TbtWmiMailboxCommand.h"
#include "ControllerDetails.h"

/**
 * registering received event with the netlink wrapper
 */
LinuxControllerCommandSender::LinuxControllerCommandSender()
{
	GenlWrapper::Instance().register_events_callback(LinuxControllerCommandSender::OnEvent);
}

/**
 * this is the receiving callback for notifications and messages from driver,
 * every message that arrives from driver/FW is going through this callback
 */
void LinuxControllerCommandSender::OnEvent(uint32_t controller_id,PDF_VALUE pdf,const std::vector<uint8_t>& data)
{
	TbtServiceLogger::LogDebug("LinuxControllerCommandSender::OnEvent entry, PDF: %d",pdf);
		try{
		char buffer [100];
		int cx;

		//converting uint to string in its Hex form with 8 digits
		cx = snprintf ( buffer, 100, "%.8x", controller_id );
		if(ConnectionManager::GetInstance()->isPreShutdownMode()) {
			TbtServiceLogger::LogDebug("System is shutting down, ignoring message");
			return;
		}
		const controlleriD cId(buffer, buffer+cx+1);

		switch (pdf) {

		case PDF_FW_TO_SW_RESPONSE:
			TbtServiceLogger::LogDebug("Message from FW: PDF_FW_TO_SW_RESPONSE, Controller: %x", controller_id);
			MessageFromFwEvent::HandleFwToSwResponse(cId, data);
			break;
		case PDF_FW_TO_SW_NOTIFICATION:
			TbtServiceLogger::LogDebug("Message from FW: PDF_FW_TO_SW_NOTIFICATION, Controller: %x", controller_id);
			MessageFromFwEvent::HandleFwToSwNotification(cId, data);
			break;
		case PDF_INTER_DOMAIN_REQUEST:
			TbtServiceLogger::LogDebug("Message from FW: PDF_INTER_DOMAIN_REQUEST, Controller: %x", controller_id);
			ConnectionManager::GetInstance()->GetP2PDevice(cId, TBT_GET_PORT_BY_LINK(GetBufStruct<XDOMAIN_PROPERTIES_READ_RESPONSE>(data)->RouteString.Level0PortNum))->HandleInterDomainRequest(data);
			break;
		case PDF_INTER_DOMAIN_RESPONSE:
			TbtServiceLogger::LogDebug("Message from FW: PDF_INTER_DOMAIN_RESPONSE, Controller: %x", controller_id);
			ConnectionManager::GetInstance()->GetP2PDevice(cId, TBT_GET_PORT_BY_LINK(GetBufStruct<XDOMAIN_PROPERTIES_READ_RESPONSE>(data)->RouteString.Level0PortNum))->HandleInterDomainResponse(data);
			break;
		case PDF_ERROR_NOTIFICATION:
			TbtServiceLogger::LogError("Error: Message from FW: PDF_ERROR_NOTIFICATION, Controller: %x", controller_id);
			break;
		case PDF_DRIVER_QUERY_DRIVER_INFORMATION: //NOTE: currently only message from driver is Query information
		{
			TbtServiceLogger::LogInfo("Netlink message received from Driver");
			QueryDriverInformation driver_information = *reinterpret_cast<const QueryDriverInformation*>(&data.front());

      //updating controller details with driver and device information
			ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->SetDriverVersion(StringToWString(driver_information.driver_version));
			ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->SetGeneration(GetGenerationFromControllerID(cId));
			ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->SetNumOfPorts(GetNomOfPortsFromControllerID(cId));
			ConnectionManager::GetInstance()->GetController(cId)->SetDmaPort(driver_information.dma_port);
			ConnectionManager::GetInstance()->GetController(cId)->SetNvmVersionOffset(driver_information.nvm_offset);
         ConnectionManager::GetInstance()->GetController(cId)->SetSupportsFullE2E(driver_information.supportsFullE2E);
			TbtServiceLogger::LogInfo("Driver version: %s, FW version: %s, Security level: %d, Generation: Thunderbolt %d, NumOfPorts:%d",
							StringToWString(driver_information.driver_version).c_str(),
							ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->GetFWVersion().c_str(),
							ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->GetSecurityLevel(),
							GetGenerationFromControllerID(cId),
							GetNomOfPortsFromControllerID(cId));

				break;
		}
			default:
				TbtServiceLogger::LogWarning("Warning: Message from FW: Unknown message PDF = %x", pdf);
				break;
		}

		TbtServiceLogger::LogInfo("Netlink message received");
		}
		catch (const std::exception& e) {
			TbtServiceLogger::LogError("Error: LinuxControllerCommandSender::OnEvent failed, PDF: %d, (Exception: %s)",pdf,e.what());
		}
		TbtServiceLogger::LogDebug("LinuxControllerCommandSender::OnEvent exit, PDF: %d",pdf);
}

/**
 * sending command to driver
 */
void LinuxControllerCommandSender::SendDriverCommand(const controlleriD& cID,
		SW_TO_FW_INMAILCMD_CODE cmd) const {
	std::shared_ptr<TbtWmiMailboxCommand> msg = std::make_shared<TbtWmiMailboxCommand>() ;
	msg->SetCommand(cmd);
	GenlWrapper::Instance().send_message_sync(ControllerIDToToInt(cID),NHI_CMD_MAILBOX,msg);
}

/**
 * sending message to driver
 */
void LinuxControllerCommandSender::SendTbtWmiMessageToFW(
		const controlleriD& Cid, uint8_t* message, size_t messageSize,
		PDF_VALUE pdf) const {
	std::shared_ptr<TbtWmiMessageToFW> msg =std::make_shared<TbtWmiMessageToFW>() ;
	msg->SetMessage(static_cast<unsigned char*>(message),messageSize);
	msg->SetPdf(pdf);
	GenlWrapper::Instance().send_message_sync(ControllerIDToToInt(Cid),NHI_CMD_MSG_TO_ICM,msg);
}

/**
 * this message is sending to driver after exchance P2P XDomain properties and
 * verifying that the remote host support Thunderbolt IP.
 * after driver receive this message it will create an Ethernet adapter for the
 * Thunderbolt P2P adapter
 */
void LinuxControllerCommandSender::SendTbtWmiApproveP2P(
		const IP2PDevice& Device) const {
	TbtServiceLogger::LogInfo("Send approve P2P command for %s",Device.RemoteHostName().c_str());
   auto msg = std::make_shared<TbtWmiApproveP2P>(Device.LocalRouteString(),
                                                 Device.LocalHostRouterUniqueID(),
                                                 Device.RemoteRouterUniqueID(),
                                                 0xFFFFF,
                                                 Device.Depth(),
                                                 Device.GetEnableFullE2E(),
                                                 Device.GetMatchFragmentsID());

	GenlWrapper::Instance().send_message_sync(ControllerIDToToInt(Device.ControllerID()),NHI_CMD_APPROVE_TBT_NETWORKING,msg);
   TbtServiceLogger::LogDebug("Approve P2P message has been sent to the driver");
}
