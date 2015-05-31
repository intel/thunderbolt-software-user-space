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
	static std::shared_ptr<ConnectionManager> Instance = std::make_shared<ConnectionManager>(std::make_shared<LinuxControllerCommandSender>());
	return Instance;
}

/**
 * constructor - initialize controller command sender
 */
ConnectionManager::ConnectionManager(
		std::shared_ptr<IControllerCommandSender> sender) :
		ConnectionManagerBase(sender) {

}

/**
 * sending command to FW through the Thunderbolt driver
 */
void ConnectionManager::SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd)
{
	m_ControllerCommandSender->SendDriverCommand(cID,cmd);
}

/**
 * sending request for driver information
 */
void ConnectionManager::QueryDriverInformation(const controlleriD& Cid)  {
	std::thread([this](controlleriD Cid){
		ConnectionManagerBase::QueryDriverInformation(Cid);
	},Cid).detach();
}

/**
 * getting the localhost name
 */
std::string ConnectionManager::GetLocalHostUTF8ComputerName() const {
	char buff[HOST_NAME_MAX];
	if(gethostname(buff,HOST_NAME_MAX) == -1)
	{
		TbtServiceLogger::LogError("Error: Fail resolving hostname. errno= %s",strerror(errno));
		throw TbtException("Fail resolving hostname");
	}

	return buff;
}

/**
 * the function creates P2P device
 */
std::shared_ptr<IP2PDevice> ConnectionManager::CreateP2PDevice(const controlleriD& ControllerId, const INTER_DOMAIN_CONNECTED_NOTIFICATION& e) const
{
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
void ConnectionManager::OnInterDomainConnect( const controlleriD& ControllerId,const INTER_DOMAIN_CONNECTED_NOTIFICATION& e )
{
	ConnectionManagerBase::OnInterDomainConnect(ControllerId,e);
	GetP2PDevice(ControllerId, TBT_GET_PORT_BY_LINK(e.Link))->StartHandshake();
	GetP2PDevice(ControllerId, TBT_GET_PORT_BY_LINK(e.Link))->SendPropertiesChangeRequest();
}

/**
 * this function is called when FW built the P2P rx/tx path.
 * in this case the P2P device is marked as "tunneled"
 */
void ConnectionManager::OnApproveInterDomainConnectionResponse( const controlleriD& cId, const APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE& e )
{
	try
	{
		if (e.ErrorFlag)
		{
			TbtServiceLogger::LogError("Error: Approve inter domain connection response for Controller %s returned with an error flag", WStringToString(cId).c_str());
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
	   TbtServiceLogger::LogError("Error: OnApproveInterDomainConnectionResponse: Unknown error, Controller: %s", WStringToString(cId).c_str());
   }
   
   
}
