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
      wss >> std::hex >> std::setfill(L'0') >> std::setw(8) >> std::internal >> v;
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
   std::wstringstream StrStream;
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
