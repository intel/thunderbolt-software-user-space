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
#include "ControllerDetails.h"
#include <string>


ControllerDetails::ControllerDetails(
	std::wstring FWVersion ,
	std::wstring DriverVersion ,
	uint32_t SecurityLevel  ,
	ThunderboltGeneration Generation, 
	uint32_t NumOfPorts) :
	m_NumOfPorts(NumOfPorts),
	m_Generation(Generation),
	m_SecurityLevel(SecurityLevel),
	m_DriverVersion(DriverVersion),
	m_FWVersion(FWVersion)
{
}

ControllerDetails::ControllerDetails(const DRIVER_READY_RESPONSE& e, ThunderboltGeneration Generation, uint32_t NumOfPorts) :m_NumOfPorts(NumOfPorts),
																												m_Generation(Generation),
																												m_SecurityLevel(e.SecurityLevel),
																												m_DriverVersion(L""),
																												m_FWVersion(std::to_wstring(e.FwRamVersion) + L"." + std::to_wstring(e.FwRomVersion))
{
	
}

ControllerDetails::~ControllerDetails(void)
{
}
