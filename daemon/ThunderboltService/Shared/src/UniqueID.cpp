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

#include <sstream>
#include <iomanip>
#include "UniqueID.h"
#include "logger.h"
#include "tbtException.h"


UniqueID::UniqueID(const std::wstring& uid)
{
	std::wstringstream wss(uid);
	while (!wss.eof())
	{
		uint32_t v;
		wchar_t c;
		wss >>  std::hex >> std::setfill(L'0') >> std::setw(8) >> std::internal >> v;
		wss >> c;
		if (c != ':')
		{
			TbtServiceLogger::LogError("Error: Faild parsing the UUID string, %c is not a valid saparation char", c);
			throw TbtException("Faild parsing the UUID string, invalid sparator char in the uuid string");
		}
		m_uuid.push_back(v);
	}
}

std::wstring UniqueID::ToWString() const
{
	std::wstringstream  StrStream;
	for (auto c : m_uuid)
	{
		StrStream << std::hex << std::setfill(L'0') << std::setw(8) << std::internal << c << L':';
	}
	auto s = StrStream.str();
	s.pop_back();
	return s;
}

void UniqueID::ToBuffer(UNIQUE_ID& buffer) const
{
	std::copy(m_uuid.begin(), m_uuid.end(), buffer);
}
