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


#ifndef _TBT_WMI_MAILBOX_COMMAND_QUERIER_H_
#define _TBT_WMI_MAILBOX_COMMAND_QUERIER_H_

#include <string>
#include "ISerializable.h"
#include "MessagesWrapper.h"

/**
 * this class is a message that is sent to FW for performaing mailbox based commands
 */
class TbtWmiMailboxCommand:
	public ISerializable
{
public:

	TbtWmiMailboxCommand(
		const std::wstring& InstanceName = L"",
		SW_TO_FW_INMAILCMD_CODE cmd = static_cast<SW_TO_FW_INMAILCMD_CODE>(0)
		);
	
	virtual PropertiesMap GetReadOnlyProperties() const;

	virtual PropertiesMap GetWriteableProperties() const;

	virtual PropertiesMap GetAllProperties() const ;


	// Deserialize the object from the given PropertiesMap.
	virtual void LoadFromSerializationMap( const PropertiesMap& PropertiesToLoadMap );

	// Getters
	std::wstring GetInstanceName() const;

	// Setters
	void SetCommand(uint32_t Command);

	uint32_t GetCommand()const;

private:
	// TODO: Check if the Active member is important
	std::wstring m_InstanceName;
	uint32_t m_Command;

};

#endif // !_TBT_WMI_MAILBOX_COMMAND_QUERIER_H_
