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

#ifndef _TBT_MESSAGE_TO_FW_H_
#define _TBT_MESSAGE_TO_FW_H_

#include "ISerializable.h"
#include "MessagesWrapper.h"

// This class wrappes a message to the FW.
// It handles the crc and the DWORDs swaps.
class TbtWmiMessageToFW :
	public ISerializable
{
public:

	
	//************************************
	// Method:    TbtWmiMessageToFW
	// FullName:  TbtWmiMessageToFW::TbtWmiMessageToFW
	// Access:    public 
	// Returns:   
	// Qualifier:
	// Default c-tor
	//************************************
	TbtWmiMessageToFW();
	
	//************************************
	// Method:    TbtWmiMessageToFW
	// FullName:  TbtWmiMessageToFW::TbtWmiMessageToFW
	// Access:    public 
	// Returns:   
	// Qualifier:
	// Parameter: const std::wstring & InstanceName
	// Parameter: PDF_VALUE Pdf
	// Parameter: uint32_t MessageLength
	// Parameter: PUCHAR Message
	//************************************
	TbtWmiMessageToFW(const std::wstring& InstanceName,
							PDF_VALUE Pdf,
							uint32_t MessageLength,
							PUCHAR Message
							);


	//************************************
	// Method:    GetWriteableProperties
	// FullName:  TbtWmiMessageToFW::GetWriteableProperties
	// Access:    virtual public 
	// Returns:   PropertiesMap
	// Qualifier: const
	//************************************
	virtual PropertiesMap GetWriteableProperties() const;

	//************************************
	// Method:    GetReadOnlyProperties
	// FullName:  TbtWmiMessageToFW::GetReadOnlyProperties
	// Access:    virtual public 
	// Returns:   PropertiesMap
	// Qualifier: const
	//************************************
	virtual PropertiesMap GetReadOnlyProperties() const;

	//************************************
	// Method:    GetAllProperties
	// FullName:  TbtWmiMessageToFW::GetAllProperties
	// Access:    virtual public 
	// Returns:   PropertiesMap
	// Qualifier: const
	//************************************
	virtual PropertiesMap GetAllProperties() const;

	//************************************
	// Method:    LoadFromSerializationMap
	// FullName:  TbtWmiMessageToFW::LoadFromSerializationMap
	// Access:    virtual public 
	// Returns:   void
	// Qualifier:
	// Parameter: const PropertiesMap & PropertiesMapToLoad
	//************************************
	virtual void LoadFromSerializationMap( const PropertiesMap& PropertiesMapToLoad );

	// Getters
	//************************************
	// Method:    GetInstanceName
	// FullName:  TbtWmiMessageToFW::GetInstanceName
	// Access:    public 
	// Returns:   std::wstring
	// Qualifier: const
	//************************************
	std::wstring GetInstanceName() const;

	//************************************
	// Method:    GetPdf
	// FullName:  TbtWmiMessageToFW::GetPdf
	// Access:    public 
	// Returns:   PDF_VALUE
	// Qualifier: const
	//************************************
	PDF_VALUE GetPdf() const;


	std::vector<uint8_t> GetMessage() const;

	// Setters
	//************************************
	// Method:    SetPdf
	// FullName:  TbtWmiMessageToFW::SetPdf
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: PDF_VALUE Pdf
	//************************************
	void SetPdf(PDF_VALUE Pdf);

	//************************************
	// Method:    SetMessage
	// FullName:  TbtWmiMessageToFW::SetMessage
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: PUCHAR Message
	// Parameter: uint32_t Length
	// This method does the following :
	//		1. Swaps the DWORDs of the message.
	//		2. Calculate the CRC of the message.
	//		3. Swaps the bytes of the CRC.
	//		4. Put it all in a new allocated safe array.
	//		5. Swaps the bytes of the given message back to their original state.
	// This method may throw an exception!
	//************************************
	void SetMessage( unsigned char* Message, uint32_t Length);

private:

	// The instance name for the message
	std::wstring m_InstanceName;

	// The PDF value for the message
	PDF_VALUE m_Pdf;

	// The length of the message
	uint32_t m_MessageLength;

   std::vector<uint8_t> m_Message;
};

#endif // !_TBT_MESSAGE_TO_FW_H_
