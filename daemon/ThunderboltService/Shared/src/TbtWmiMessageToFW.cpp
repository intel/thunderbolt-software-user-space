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
#include <string>
#include "TbtWmiMessageToFW.h"
#include "Utils.h"

#include "tbtException.h"


TbtWmiMessageToFW::TbtWmiMessageToFW() :
	m_InstanceName(L""),
	m_Pdf(static_cast<PDF_VALUE>(0)),
	m_MessageLength(0)
{
}

TbtWmiMessageToFW::TbtWmiMessageToFW( 
	const std::wstring& InstanceName,
	PDF_VALUE Pdf,
	uint32_t MessageLength,
	unsigned char* Message
	) :
	m_InstanceName(InstanceName),
	m_Pdf(Pdf),
	m_MessageLength(MessageLength)
{
	SetMessage(Message, MessageLength);
}

PropertiesMap TbtWmiMessageToFW::GetWriteableProperties() const
{
	PropertiesMap Properties;

	Properties[L"PDF"] = static_cast<uint32_t>(m_Pdf);
	Properties[L"MessageLength"] = m_MessageLength;
   Properties[L"Message"] = m_Message;

	return Properties;
}

PropertiesMap TbtWmiMessageToFW::GetReadOnlyProperties() const
{
	PropertiesMap Properties;

	Properties[L"InstanceName"] = m_InstanceName;

	return Properties;
}

PropertiesMap TbtWmiMessageToFW::GetAllProperties() const
{
	PropertiesMap Properties = GetReadOnlyProperties();
	PropertiesMap WriteableProperties = GetWriteableProperties();

	Properties.insert(WriteableProperties.begin(), WriteableProperties.end());

	return Properties;
}

void TbtWmiMessageToFW::LoadFromSerializationMap( const PropertiesMap& PropertiesMapToLoad )
{
	try
	{
      m_InstanceName = boost::get<std::wstring>(PropertiesMapToLoad.at(L"InstanceName"));
      m_Pdf = static_cast<PDF_VALUE>(boost::get<uint32_t>((PropertiesMapToLoad.at(L"PDF"))));
      m_MessageLength = boost::get<uint32_t>(PropertiesMapToLoad.at(L"MessageLength"));
      m_Message = boost::get<std::vector<uint8_t>>(PropertiesMapToLoad.at(L"Message"));
	}
	catch (const std::out_of_range& e)
	{
		TbtServiceLogger::LogError("Error: Exception - Unable to load from serialization map!");
		throw TbtException("Unable to load from serialization map!");
	}
	catch (...)
	{
		TbtServiceLogger::LogError("Error: Unknown error - Unable to load from serialization map!");
		throw TbtException("Unknown error - Unable to load from serialization map!");
	}
}

std::wstring TbtWmiMessageToFW::GetInstanceName() const
{
	return m_InstanceName;
}


PDF_VALUE TbtWmiMessageToFW::GetPdf() const
{
	return m_Pdf;
}

void TbtWmiMessageToFW::SetPdf( PDF_VALUE Pdf )
{
	m_Pdf = Pdf;
}

std::vector<uint8_t> TbtWmiMessageToFW::GetMessage() const
{
   return m_Message;
}

void TbtWmiMessageToFW::SetMessage( unsigned char* Message, uint32_t Size)
{

	uint32_t* pCrc =  reinterpret_cast<uint32_t*>(Message + Size - sizeof(uint32_t));
	uint32_t OldVal = *pCrc;

	// Swap all DWORDs of the given message except the last one that will contain the crc
	ArrayDwordSwap(Message, Size - sizeof(uint32_t));

	// Calculate CRC
	*pCrc = CalculateCrc(Message, Size - sizeof(uint32_t));

	// Swap the DWORDs of the crc
	ArrayDwordSwap((PUCHAR)pCrc, sizeof(uint32_t));

	// Insert the swapped message into the new allocated vector

	m_Message = std::vector<uint8_t>(Message,Message + Size);
	m_MessageLength = Size;
   /*m_Message.resize(Size);
	for ( long i = 0; i < (long)(Size); i++)
	{
      m_Message[i] = *(Message + i);
	}*/

	// Swap all DWORDs of the given message except the last one that contains the crc
	ArrayDwordSwap(Message, Size - sizeof(uint32_t));

	// Insert the original last DWORD of the message into the it's last DWORD
	*pCrc = OldVal;

}
