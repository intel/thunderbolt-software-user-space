/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2014 - 2016 Intel Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Intel Corporation nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#ifndef _TBT_MESSAGE_TO_FW_H_
#define _TBT_MESSAGE_TO_FW_H_

#include "ISerializable.h"
#include "MessagesWrapper.h"

// This class wrappes a message to the FW.
// It handles the crc and the DWORDs swaps.
class TbtWmiMessageToFW : public ISerializable
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
   TbtWmiMessageToFW(const std::wstring& InstanceName, PDF_VALUE Pdf, uint32_t MessageLength, PUCHAR Message);

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
   virtual void LoadFromSerializationMap(const PropertiesMap& PropertiesMapToLoad);

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
   void SetMessage(unsigned char* Message, uint32_t Length);

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
