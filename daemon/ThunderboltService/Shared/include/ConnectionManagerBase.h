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


#ifndef _CONNECTION_MANAGER_BASE_
#define _CONNECTION_MANAGER_BASE_

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <condition_variable>
#include "IConnectionManager.h"
#include "IController.h"
#include "UniqueID.h"
#include "Utils.h"
#include "logger.h"

using std::map;
using std::unique_ptr;

/**
 * This class contains the main logic of the daemon
 */
class ConnectionManagerBase :
   public IConnectionManager
{
public: 

	ConnectionManagerBase(std::shared_ptr<IControllerCommandSender>& sender);

	virtual ~ConnectionManagerBase()
	{
	}

    void RegisterExitCallback(std::function<void()> ExitCallback);
	// Notifications from FW
   virtual void OnInterDomainConnect( const controlleriD&,const INTER_DOMAIN_CONNECTED_NOTIFICATION& e );
   virtual void OnInterDomainDisconnected( const controlleriD&,const INTER_DOMAIN_DISCONNECTED_NOTIFICATION& e );
	// Responses from FW
   virtual void OnDriverReadyResponse( const controlleriD&,const DRIVER_READY_RESPONSE& e );
   virtual void OnServiceDown();
   virtual shared_ptr<IController> GetController( const controlleriD& Cid) const;
   virtual const map<controlleriD,shared_ptr<IController>>& GetControllers() const;
   virtual IControllerCommandSender& GetControllerCommandSender() const;
   virtual void QueryDriverInformation(const controlleriD& Cid) ;
	virtual std::shared_ptr<ControllerSettings> GetControllersSettings() const;
	virtual void SetControllersSettings(const std::shared_ptr<ControllerSettings> Settings);
	virtual void setPreShutdownMode() {m_isPreShutdownMode = true;};
	virtual bool isPreShutdownMode() const {return m_isPreShutdownMode;};
   std::shared_ptr<IP2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum);
   const std::shared_ptr<IP2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum) const;
   virtual void OnSystemPreShutdown();
protected:
   shared_ptr<IControllerCommandSender> m_ControllerCommandSender;
   map<controlleriD,shared_ptr<IController>> m_Controllers;
	bool m_isPreShutdownMode;
   std::function<void()> m_ExitCallback;
protected:
   std::shared_ptr<ControllerSettings> m_ControllersSettings;

private:
	// No Copy or assignment to prevent memory dup
	ConnectionManagerBase(const ConnectionManagerBase&);
	ConnectionManagerBase& operator=(const ConnectionManagerBase&);
};


#endif // !_CONNECTION_MANAGER_BASE_
