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


#ifndef _IP2P_DEVICE_H_
#define _IP2P_DEVICE_H_
#include <atomic>
#include "MessagesWrapper.h"
#include "UniqueID.h"

enum class P2P_STATE{
	NOT_READY,					//the device is not ready, on start and time_out
	READING_PROPERTIES,	//the device is currently reading or handling remote host properties
	TUNNEL,						  //the device is tunneled, Ethernet adapter is up
	NOT_SUPPORTED,			//the device is not support Thunderbolt IP
	PENDING_TUNNEL			//the device support Thunderbolt IP and wait for driver to bring the Ethernet adapter up
};

/**
 * this interface if for holding a P2P device object created when connecting two
 * Thunderbolt hosts (Windows <--> Linux <--> Apple)
 */
class IP2PDevice
{
public:
	virtual ~IP2PDevice(){};
	virtual void HandleInterDomainRequest(const std::vector<uint8_t>& Msg) = 0 ;
	virtual void HandleInterDomainResponse(const std::vector<uint8_t>& Msg)= 0;
	virtual void SendPropertiesChangeResponse(const XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION& Msg) const = 0;
	virtual void SendReadPropertiesResponse(uint32_t Offset, uint8_t sn) const = 0;

   /**
    * \brief Returns if full-E2E mode should be enabled for this P2P device
    *
    * \return true if full-E2E mode should be enabled, otherwise - false
    */
   virtual bool GetEnableFullE2E() const = 0;

   /**
   * \brief Returns if P2P should match fragments ID
   *
   * \return true if match fragments ID, otherwise - false
   */
   virtual bool GetMatchFragmentsID() const = 0;

   virtual void StartHandshake() = 0;
	virtual void SendPropertiesChangeRequest() = 0;
	virtual void OnSystemPreShutdown() = 0;
	virtual const UniqueID& RemoteRouterUniqueID() const = 0;
	virtual const ROUTE_STRING& LocalRouteString() const = 0;
	virtual const UniqueID& LocalHostRouterUniqueID() const = 0;
	virtual std::string RemoteHostName() const = 0;
	virtual bool PathEstablished() const = 0;
	virtual const controlleriD& ControllerID() const = 0;
	virtual uint8_t Depth() const = 0;
	virtual const std::atomic<P2P_STATE>& State() const = 0;
	virtual void SetState(P2P_STATE value) = 0;

};
#endif // !_IP2P_DEVICE_H_
