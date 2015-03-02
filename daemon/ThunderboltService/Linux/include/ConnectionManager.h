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


#ifndef _LINUX_CONNECTION_MANAGER_H
#define _LINUX_CONNECTION_MANAGER_H

#include <memory>
#include "ConnectionManagerBase.h"

class  ControllerSettings;

/**
 * This class is used as singleton and contain the main logic of the daemon
 */
class ConnectionManager:
	public ConnectionManagerBase
{
public:
   ConnectionManager(std::shared_ptr<IControllerCommandSender> sender);
   static std::shared_ptr<ConnectionManager> GetInstance();
   void SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd);
   void QueryDriverInformation(const controlleriD& Cid) override;
   std::string GetLocalHostUTF8ComputerName() const;
   std::shared_ptr<IP2PDevice> CreateP2PDevice(const controlleriD& ControllerId, const INTER_DOMAIN_CONNECTED_NOTIFICATION& e) const;
   void OnApproveInterDomainConnectionResponse(const controlleriD&, const APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE&);
   void OnInterDomainConnect( const controlleriD& ControllerId,const INTER_DOMAIN_CONNECTED_NOTIFICATION& e );
};

#endif
