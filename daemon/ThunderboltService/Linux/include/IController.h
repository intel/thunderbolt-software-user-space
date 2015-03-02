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


#ifndef _ICONTROLLER_
#define _ICONTROLLER_

#include <map>
#include <memory>
#include <string>
#include "IPort.h"

class ControllerDetails;
class ControllerSettings;

using std::unique_ptr;
using std::shared_ptr;
using std::string;

typedef std::map<uint32_t, shared_ptr<IPort>> PortsMap;

/* IController Interface
 * ---------------------
 * Represents a Thunderbolt controller on the machine
 */
class IController
{
public:

   virtual ~IController(){};

   //Methods
   virtual const controlleriD& GetControllerID() const = 0;

   virtual void AddPort(uint32_t position, std::shared_ptr<IPort> PortToAdd) = 0;

   virtual SECURITY_LEVEL GetSecurityLevel() const = 0;

   virtual const PortsMap& GetPorts() const = 0;

   virtual shared_ptr<IPort> GetPortAt(uint32_t position) const = 0;

   virtual int GetNumOfPorts() const = 0;

	virtual bool HasPort(uint32_t PortNum) const = 0;

	virtual shared_ptr<ControllerDetails> GetControllerData() const = 0;

	virtual void SetControllerData(shared_ptr<ControllerDetails> ControllerData) = 0;

	virtual int GetDmaPort() const = 0;

	virtual void SetDmaPort(int port) = 0;

    virtual void SetNvmVersionOffset(uint32_t address) = 0;

	virtual void Clear() = 0;

   virtual void AddP2PDevice(uint32_t port, std::shared_ptr<IP2PDevice> pDevice) = 0;

};


#endif // !_ICONTROLLER_

