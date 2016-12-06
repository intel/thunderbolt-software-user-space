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
   static const auto UNIQUE_ID_SIZE = sizeof(UNIQUE_ID) / sizeof(uint32_t);

public:
   UniqueID() : m_uuid() {}

   UniqueID(const UNIQUE_ID& uid) : m_uuid(uid, uid + UNIQUE_ID_SIZE) {}

   UniqueID(const std::wstring& uid);

   UniqueID(const std::vector<uint32_t>& uid) : m_uuid(uid.begin(), uid.begin() + UNIQUE_ID_SIZE){};

   bool operator==(const UniqueID& o) const { return (m_uuid == o.m_uuid); }
   bool operator!=(const UniqueID& o) const { return !(*this == o); }
   bool operator<(const UniqueID& o) const { return (m_uuid < o.m_uuid); }

   std::wstring ToWString() const;

   void ToBuffer(UNIQUE_ID& buffer) const;

   const uint32_t* data() const { return m_uuid.data(); }

private:
   std::vector<uint32_t> m_uuid;
};

#endif // UNIQE_ID_
