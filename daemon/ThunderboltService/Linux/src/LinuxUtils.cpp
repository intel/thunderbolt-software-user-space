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

#include "tbtException.h"
#include "Utils.h"
#include "logger.h"

std::wstring utf8stringToWstring(const std::string& ut8String)
 {
	return StringToWString(ut8String);
 }

/**
 * this function return if this P2P host support authentication
 * 0 - not supported, in this case automatic connection will be established
 *     no need for user approval
 * 1 - supported, currently not implemented
 */
uint32_t IsAuthenticationSupported()
{
	return 0;
}

uint32_t ControllerIDToToInt(const controlleriD& ID)
{
	uint32_t controller_id;
	std::wstringstream wss;
	wss << std::hex << ID;
	wss >> controller_id;
	return controller_id;
}
uint32_t GetDeviceIDFromControllerID(const controlleriD& ID)
{

	uint32_t DevIDInt;
	std::wstringstream wss;

	try
	{
		auto devIDstr = ID.substr(4, 4);;
		wss << std::hex << devIDstr;
		wss >> DevIDInt;
	}
	catch (const std::exception& e)
	{
		TbtServiceLogger::LogError("Error: GetDeviceIDFromControllerID failed (Exception: %s)", e.what());
		throw;
	}
	catch (...)
	{
		TbtServiceLogger::LogError("Error: GetDeviceIDFromControllerID unknown exception");
		throw;
	}
	return DevIDInt;
}
