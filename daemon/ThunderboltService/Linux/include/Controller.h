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


#ifndef _CONTROLLER_
#define _CONTROLLER_

#include <map>
#include <memory>
#include <future>
#include "IController.h"
#include "IP2PDevice.h"


/**
 * this class represents a Thunderbolt controller on the machine
 */
class Controller :
   public IController
{
public:
	Controller(controlleriD id,DRIVER_READY_RESPONSE controllerData);
	Controller(controlleriD id,shared_ptr<ControllerDetails> options);
   virtual const controlleriD& GetControllerID() const;
	virtual void SetNvmVersionOffset(uint32_t address) { m_NvmOffset = address; }
   virtual SECURITY_LEVEL GetSecurityLevel() const;
   virtual const PortsMap& GetPorts() const;
   virtual shared_ptr<IPort> GetPortAt(uint32_t position ) const;
      virtual void AddPort(uint32_t position, std::shared_ptr<IPort> PortToAdd);
   virtual int GetNumOfPorts() const;
	virtual shared_ptr<ControllerDetails> GetControllerData() const;
	virtual void SetControllerData( shared_ptr<ControllerDetails> ControllerData );
	virtual bool HasPort(uint32_t PortNum) const;
	virtual int GetDmaPort() const;
	virtual void SetDmaPort(int port);

   /**
    * \brief Returns if the controller supports full-E2E mode for better P2P performance
    *
    * \returns true if full-E2E mode is supported, otherwise - false
    */
   bool GetSupportsFullE2E() override;

   /**
    * \brief Sets if the controller supports full-E2E mode for better P2P performance
    *
    * \param[in]  fullE2ESupport    true if full-E2E mode is supported
    */
   void SetSupportsFullE2E(bool fullE2ESupport) override;

	virtual void Clear();
	virtual void AddP2PDevice(uint32_t Port, std::shared_ptr<IP2PDevice> pDeviceToAdd);
   virtual void SetIsUp(bool isUp);
   virtual bool IsUp() const { return m_IsUp; }

private:
	controlleriD m_Id; 									//the controller identifier
	shared_ptr<ControllerDetails> m_ControllerDetails;	//controller details
	PortsMap m_Ports;									//ports that hold the connected devices
	int m_DmaPort;										//the controller DMA port number
	uint32_t m_NvmOffset;								//offset of the controller NVM
	bool m_IsUp;										//the power mode of the controller
   bool m_SupportsFullE2E = false;
};

#endif // !_CONTROLLER_
