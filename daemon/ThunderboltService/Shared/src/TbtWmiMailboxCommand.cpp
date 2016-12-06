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

#include "TbtWmiMailboxCommand.h"
#include "logger.h"

TbtWmiMailboxCommand::TbtWmiMailboxCommand(const std::wstring& InstanceName /*= L""*/,
                                           SW_TO_FW_INMAILCMD_CODE cmd /*= 0*/)
   : m_InstanceName(InstanceName), m_Command(cmd)
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
   PropertiesMap Properties          = GetReadOnlyProperties();
   PropertiesMap WriteableProperties = GetWriteableProperties();

   Properties.insert(WriteableProperties.begin(), WriteableProperties.end());

   return Properties;
}

// Deserialize the object from the given PropertiesMap.
void TbtWmiMailboxCommand::LoadFromSerializationMap(const PropertiesMap& PropertiesToLoadMap)
{
   try
   {
      m_InstanceName = boost::get<std::wstring>(PropertiesToLoadMap.at(L"InstanceName"));
      m_Command      = boost::get<uint32_t>(PropertiesToLoadMap.at(L"Command"));
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
