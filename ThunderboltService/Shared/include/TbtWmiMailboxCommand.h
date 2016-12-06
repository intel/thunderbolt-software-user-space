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

#ifndef _TBT_WMI_MAILBOX_COMMAND_QUERIER_H_
#define _TBT_WMI_MAILBOX_COMMAND_QUERIER_H_

#include <string>
#include "ISerializable.h"
#include "MessagesWrapper.h"

/**
 * this class is a message that is sent to FW for performaing mailbox based commands
 */
class TbtWmiMailboxCommand : public ISerializable
{
public:
   TbtWmiMailboxCommand(const std::wstring& InstanceName = L"",
                        SW_TO_FW_INMAILCMD_CODE cmd      = static_cast<SW_TO_FW_INMAILCMD_CODE>(0));

   virtual PropertiesMap GetReadOnlyProperties() const;

   virtual PropertiesMap GetWriteableProperties() const;

   virtual PropertiesMap GetAllProperties() const;

   // Deserialize the object from the given PropertiesMap.
   virtual void LoadFromSerializationMap(const PropertiesMap& PropertiesToLoadMap);

   // Getters
   std::wstring GetInstanceName() const;

   // Setters
   void SetCommand(uint32_t Command);

   uint32_t GetCommand() const;

private:
   // TODO: Check if the Active member is important
   std::wstring m_InstanceName;
   uint32_t m_Command;
};

#endif // !_TBT_WMI_MAILBOX_COMMAND_QUERIER_H_
