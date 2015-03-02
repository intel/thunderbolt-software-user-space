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
#include <thread>
#include <string>
#include "Port.h"
#include "Controller.h"
#include "ConnectionManager.h"
#include "ControllerDetails.h"
#include "UniqueID.h"
#include "tbtException.h"

Controller::Controller( controlleriD id,DRIVER_READY_RESPONSE controllerData ) : 
	m_Id(id),
	m_ControllerDetails(new ControllerDetails(controllerData, GetGenerationFromControllerID(id), GetNomOfPortsFromControllerID(id))),
	m_DmaPort(-1),	 // Uninitialized
	m_NvmOffset(-1), // Uninitialized
	m_IsUp(false)
{
	//TODO: create new Ports?
	m_ControllerDetails->SetFWVersion(std::to_wstring(controllerData.FwRamVersion) + L"." + std::to_wstring(controllerData.FwRomVersion));
	m_ControllerDetails->SetSecurityLevel(controllerData.SecurityLevel);

}

Controller::Controller(controlleriD id,shared_ptr<ControllerDetails> options) : 
	m_Id(id),
	m_ControllerDetails(options),
	m_DmaPort(-1),	// Uninitialized
	m_NvmOffset(-1), // Uninitialized
   m_IsUp(false)
{

}

/**
 * return identifier of the controller
 */
const controlleriD& Controller::GetControllerID() const
{
	return m_Id;
}

/**
 * return the security level of the controller
 */
SECURITY_LEVEL Controller::GetSecurityLevel() const
{
	return (SECURITY_LEVEL)m_ControllerDetails->GetSecurityLevel();
}

/**
 * return a map of ports of this controller
 */
const PortsMap& Controller::GetPorts() const
{
	return m_Ports;
}

/**
 * return specific port of this controller
 */
shared_ptr<IPort> Controller::GetPortAt( uint32_t position ) const
{
	if (!HasPort(position))
	{
		TbtServiceLogger::LogError("Error: Trying to get port number %u which doesn't exist", position);
		throw TbtException("Trying to get port with number that doesn't exists");
	}

	return m_Ports.at(position);
}

/**
 * adding new port to this controller
 */
void Controller::AddPort(uint32_t position, std::shared_ptr<IPort> PortToAdd)
{
	if (HasPort(position))
	{
		TbtServiceLogger::LogWarning("Warning: Trying to add port number %u which already exists", position);
	}

	m_Ports[position] = PortToAdd;
}

/**
 * return num of ports that this controller have
 */
int Controller::GetNumOfPorts() const
{
   return m_Ports.size();
}

/**
 * returning this controller details object
 */
shared_ptr<ControllerDetails> Controller::GetControllerData() const
{
	return m_ControllerDetails;
}

/**
 * updating this controller details
 */
void Controller::SetControllerData( shared_ptr<ControllerDetails> ControllerData )
{
	m_ControllerDetails = ControllerData;
}

/**
 * return the controller DMA port
 */
int Controller::GetDmaPort() const
{
	return m_DmaPort;
}

/**
 * set the DMA port for this controller
 */
void Controller::SetDmaPort(int port)
{
	m_DmaPort = port;
}

/**
 * clear all the ports of this controller
 */
void Controller::Clear()
{
	m_Ports.clear();
	TbtServiceLogger::LogDebug("Controller %s ports cleared",WStringToString(m_Id).c_str());
}

/**
 * check if a port exists in this controller
 */
bool Controller::HasPort( uint32_t PortNum ) const
{
	return m_Ports.find(PortNum) != m_Ports.end();
}

/**
 * this function add a p2p device under this controller in the specified port
 */
void Controller::AddP2PDevice(uint32_t Port, std::shared_ptr<IP2PDevice> pDevice)
{
   TbtServiceLogger::LogInfo("Adding P2P device to controller %s", WStringToString(m_Id).c_str());

   if (!HasPort(Port))
   {
      TbtServiceLogger::LogError("Error: Port %u does not exist!", Port);
      throw TbtException("Port does not exist!");
   }

   GetPortAt(Port)->setP2PDevice(pDevice);

}

/**
 * set the power mode of the controller
 */
void Controller::SetIsUp(bool isUp) 
{
	m_IsUp = isUp; 
	if (!isUp)
	{
		Clear();
	}
}
