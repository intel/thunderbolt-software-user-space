/********************************************************************************
 * Thunderbolt(TM) FW update library
 * This library is distributed under the following BSD-style license:
 *
 * Copyright(c) 2016 Intel Corporation.
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

#include "FileFwInfoSource.h"
#include "Exceptions.h"
#include "tbt/tbt_fwu_err.h"
#include "util.h"
#include <arpa/inet.h>

#include <stdio.h>

namespace tbt
{
namespace fwu
{

FileFwInfoSource::FileFwInfoSource(const std::vector<uint8_t>& image) : FwInfoSource(), m_rImage(image)
{
   std::unique_ptr<HwInfo> pInfo = GetHwConfiguration();
   Info().swap(*pInfo);
}

std::vector<uint8_t> FileFwInfoSource::Read(uint32_t offset, uint32_t length)
{
   if (m_rImage.size() < (offset + length))
   {
      TBT_THROW(TBT_SDK_INVALID_IMAGE_FILE);
   }
   auto start = m_rImage.begin() + offset;
   return std::vector<uint8_t>(start, start + length);
}

uint32_t FileFwInfoSource::DigitalSectionOffset()
{
   uint32_t f;
   f = GetFarb(0x000000);
   if (tbt::ValidNVMPointer(f, 3))
   {
      return f;
   }
   f = GetFarb(0x001000);
   if (tbt::ValidNVMPointer(f, 3))
   {
      return f;
   }
   TBT_THROW(TBT_SDK_INVALID_IMAGE_FILE);
}

uint32_t FileFwInfoSource::GetFarb(uint32_t offset)
{
   // Read a 24-bit, little-endian pointer.
   auto farb = Read(offset, 3);
   return ((uint32_t(farb.at(2)) << 16) | (uint32_t(farb.at(1)) << 8) | (uint32_t(farb.at(0)) << 0));
}
}
}
