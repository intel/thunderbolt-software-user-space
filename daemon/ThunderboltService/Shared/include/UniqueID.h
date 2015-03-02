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


#ifndef UNIQE_ID_
#define UNIQE_ID_

//#include <sstream>
#include <string>
#include <vector>
#include "MessagesWrapper.h"
#include "defines.h"

/**
 * represent UUID
 */
class UniqueID
{
private:
	static const auto UNIQUE_ID_SIZE = sizeof(UNIQUE_ID)/sizeof(uint32_t);

public:

	UniqueID(): m_uuid(){}

	UniqueID(const UNIQUE_ID& uid) : m_uuid(uid, uid + UNIQUE_ID_SIZE)	{}

	UniqueID(const std::wstring& uid);

   UniqueID(const std::vector<uint32_t>& uid) :m_uuid(uid.begin(), uid.begin() + UNIQUE_ID_SIZE){};

	bool operator== (const UniqueID &o) const {return  (m_uuid == o.m_uuid); }
	bool operator!=(const UniqueID &o) const {return !(*this==o); }
	bool operator< (const UniqueID &o) const {return  (m_uuid < o.m_uuid);}

	std::wstring ToWString() const;

	void ToBuffer(UNIQUE_ID& buffer) const;

   const uint32_t* data() const
   {
      return m_uuid.data();
   }

private:
	std::vector<uint32_t> m_uuid;
};

#endif // UNIQE_ID_
