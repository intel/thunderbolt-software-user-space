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


#pragma once

#include <memory>
#include <map>
#include <functional>
#include "defines.h"
#include "IP2PDevice.h"
#include "IControllerCommandSender.h"
#include "IController.h"

using std::unique_ptr;
using std::shared_ptr;
using std::map;

/* IConnectionManager Interface
 * ----------------------------
 * Main component, handle all responses and notifications from both application and driver/FW
 * 
 * Should be a singleton
 *
 */
class IConnectionManager
{
public:

	virtual ~IConnectionManager(){};
  virtual void RegisterExitCallback(std::function<void()> ExitCallback) = 0 ;

  //Notifications - FW
  virtual void OnInterDomainConnect(const controlleriD&,const INTER_DOMAIN_CONNECTED_NOTIFICATION& e)=0;
  virtual void OnInterDomainDisconnected(const controlleriD&,const INTER_DOMAIN_DISCONNECTED_NOTIFICATION& e)=0;

  //Responses
  virtual void OnDriverReadyResponse(const controlleriD&,const DRIVER_READY_RESPONSE& e) = 0;
  virtual void OnApproveInterDomainConnectionResponse(const controlleriD&, const APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE& e) = 0;
   
  //Methods
  virtual std::string GetLocalHostUTF8ComputerName() const = 0;
  virtual std::shared_ptr<IP2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum) = 0;
  virtual const std::shared_ptr<IP2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum) const = 0;
  virtual std::shared_ptr<IP2PDevice> CreateP2PDevice(const controlleriD& ControllerId, const INTER_DOMAIN_CONNECTED_NOTIFICATION& e) const = 0;
  virtual void SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd) = 0;
  virtual shared_ptr<IController> GetController(const controlleriD&) const=0;
  virtual const map<controlleriD,shared_ptr<IController>>&  GetControllers() const=0;
  virtual const IControllerCommandSender& GetControllerCommandSender() const=0;
  virtual void OnSystemPreShutdown() =0;
  virtual void setPreShutdownMode() = 0;
  virtual bool isPreShutdownMode() const = 0;
  virtual std::shared_ptr<ControllerSettings> GetControllersSettings() const = 0;
  virtual void SetControllersSettings(const std::shared_ptr<ControllerSettings> Settings) = 0;
};
