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
 
#ifndef CONTROLLER_DETAILS_H
#define CONTROLLER_DETAILS_H
#include "Utils.h"

/*
 * this class stores the controller details e.g
 * Driver & FW Version, TBT generation, # of ports etc...
 */
class ControllerDetails
{
public:
	ControllerDetails(
		std::wstring FWVersion ,
		std::wstring DriverVersion,
		uint32_t SecurityLevel,
		ThunderboltGeneration Generation,
		uint32_t NumOfPorts);

	ControllerDetails(const DRIVER_READY_RESPONSE& DriverReadyResponseNotification, ThunderboltGeneration Generation, uint32_t NumOfPorts);
	~ControllerDetails(void);
	void SetFWVersion(std::wstring version){m_FWVersion=version;};
	void SetDriverVersion(std::wstring version){m_DriverVersion=version;};
	void SetSecurityLevel(uint32_t val){m_SecurityLevel=val;};
	void SetGeneration(ThunderboltGeneration val){ m_Generation = val; };
	void SetNumOfPorts(uint32_t val){ m_NumOfPorts = val; };

	std::wstring GetFWVersion() const {return m_FWVersion;};
	std::wstring GetDriverVersion() const {return m_DriverVersion;};
	uint32_t GetSecurityLevel() const {return m_SecurityLevel;};
	ThunderboltGeneration GetGeneration() const { return m_Generation; };
	uint32_t GetNumOfPorts() const { return m_NumOfPorts; };
private:
	uint32_t m_NumOfPorts;			//number of ports for this controller
	ThunderboltGeneration m_Generation;	//Thunderbolt generetion (e.g. Thunderbolt 2)
	uint32_t m_SecurityLevel;		//the controller security level
	std::wstring m_DriverVersion;		//the version of the kernel module
	std::wstring m_FWVersion;		//the FW version for this controller
};

#endif //CONTROLLER_DETAILS_H
