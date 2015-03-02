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

#include <algorithm>
#include "Utils.h"
#include "boost/crc.hpp"
#include "tbtException.h"
//#include "stdafx.h"


std::wstring StringToWString(const std::string& Str)
{
	return std::wstring(Str.begin(), Str.end());
}

std::string WStringToString(const std::wstring& Str)
{
	return std::string(Str.begin(), Str.end());
}

void ArrayDwordSwap( uint8_t* Array, size_t Size )
{
	if ((Size % 4) != 0)
	{
		TbtServiceLogger::LogError("Error: Trying to swap array of size that is not a multiple of 4!");
		throw TbtException("Trying to swap array of size that is not a multiple of 4!");
	}

	for (uint32_t i = 0; i < Size; i += sizeof(uint32_t))
	{
		std::reverse(Array + i, Array + i + sizeof(uint32_t));
	}
}

uint32_t CalculateCrc(uint8_t* Array, size_t Size)
{
	boost::crc_optimal<32, 0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, true, true> CrcVal;
	CrcVal.process_bytes(Array, Size);
	return CrcVal.checksum();
}

uint32_t ControllerSettingsToEnum( bool certifiedOnly,bool overrideFirstDepth )
{
	uint32_t flags = SET_FW_MODE_FD1_D1_CERT_COMMAND_CODE;
	flags |= (!certifiedOnly)<<0;
	flags |= overrideFirstDepth<<2;
	return flags;
}

std::string BoolToString(bool val)
{
	return std::string((val) ? "True" : "False");
}

std::wstring Backshlashify(const std::wstring& input)
{
	static const std::wstring from = L"\\";
	static const std::wstring to = L"\\\\";

	size_t start_pos = 0;
	std::wstring Res = input;

	while ((start_pos = Res.find(from, start_pos)) != std::wstring::npos) {
		Res.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return Res;
}

std::wstring RemoveoDoubleSlash(const std::wstring& input)
{
	static const std::wstring from = L"\\\\";
	static const std::wstring to = L"\\";

	size_t start_pos = 0;
	std::wstring Res = input;

	while ((start_pos = Res.find(from, start_pos)) != std::wstring::npos) {
		Res.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return Res;
}


uint32_t GetNomOfPortsFromControllerID(const controlleriD& ID)
{
	auto devID = GetDeviceIDFromControllerID(ID);
	switch (devID)
	{
	case 0x1566:
	case 0x156a:
	case 0x157d:
	case 0x1575:
		return 1;
	case 0x1568:
	case 0x156c:
	case 0x1577:
		return 2;
	default:
		throw TbtException("Unknown device type");
	}
}

ThunderboltGeneration GetGenerationFromControllerID(const controlleriD& ID)
{
	auto devID = GetDeviceIDFromControllerID(ID);
	switch (devID)
	{
	case 0x1566:
	case 0x1568:
		return ThunderboltGeneration::THUNDERBOLT_1;
	case 0x156a:
	case 0x156c:
	case 0x157d:
		return ThunderboltGeneration::THUNDERBOLT_2;
	case 0x1575:
	case 0x1577:
		return ThunderboltGeneration::THUNDERBOLT_3;
	default:
		throw TbtException("Unknown device ID");
	}
}

