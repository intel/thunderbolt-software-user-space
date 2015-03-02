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

#include "TbtWmiMailboxCommand.h"
#include "logger.h"

TbtWmiMailboxCommand::TbtWmiMailboxCommand( const std::wstring& InstanceName /*= L""*/, SW_TO_FW_INMAILCMD_CODE cmd /*= 0*/ ) :
	m_InstanceName(InstanceName),
	m_Command(cmd)
{
}

PropertiesMap TbtWmiMailboxCommand::GetReadOnlyProperties() const
{
	PropertiesMap Properties;

	// Insert all properties that should be serialized
	Properties[L"InstanceName"] = m_InstanceName;

	return Properties;
}

PropertiesMap TbtWmiMailboxCommand::GetWriteableProperties() const
{
	PropertiesMap Properties;

	Properties[L"Command"] = m_Command;

	return Properties;
}

PropertiesMap TbtWmiMailboxCommand::GetAllProperties() const 
{
	PropertiesMap Properties = GetReadOnlyProperties();
	PropertiesMap WriteableProperties = GetWriteableProperties();

	Properties.insert(WriteableProperties.begin(), WriteableProperties.end());

	return Properties;
}

// Deserialize the object from the given PropertiesMap.
void TbtWmiMailboxCommand::LoadFromSerializationMap( const PropertiesMap& PropertiesToLoadMap )
{
	try
	{
		m_InstanceName = boost::get<std::wstring>(PropertiesToLoadMap.at(L"InstanceName"));
		m_Command = boost::get<uint32_t>(PropertiesToLoadMap.at(L"Command"));
	}
	catch (const std::out_of_range& e)
	{
		TbtServiceLogger::LogError("Error: Trying to access an unknown property : %s", e.what());
		throw;
	}
	catch (...)
	{
		TbtServiceLogger::LogError("Error: LoadFromSerializationMap Unknown error");
		throw;
	}
}

std::wstring TbtWmiMailboxCommand::GetInstanceName() const
{
	return m_InstanceName;
}

void TbtWmiMailboxCommand::SetCommand(uint32_t Command)
{
	m_Command = Command;	
}

uint32_t TbtWmiMailboxCommand::GetCommand() const
{
	return m_Command;
}
