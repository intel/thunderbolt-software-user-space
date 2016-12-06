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

#include <cinttypes>
#include "FwInfo.h"
#include "util.h"
#include "tbt/tbt_fwu_err.h"
#include "Exceptions.h"

namespace
{
const uint32_t SizeOfSizeField = sizeof(uint16_t);
}

namespace tbt
{
namespace fwu
{

FwInfo::FwInfo(FwInfoSource& source) noexcept : m_rSource(source)
{
}

const HwInfo& FwInfo::GetHwInfo() const noexcept
{
   return const_cast<const FwInfoSource&>(m_rSource).Info();
}

uint32_t FwInfo::SectionSize16Bit(uint32_t sectionOffset)
{
   auto buffer = m_rSource.Read(sectionOffset, SizeOfSizeField);
   // The "+2" part is because the size doesn't include the size field itself
   return tbt::ToUInt16(buffer, 0) + SizeOfSizeField;
}

uint32_t FwInfo::SectionSize16BitDw(uint32_t sectionOffset)
{
   return (SectionSize16Bit(sectionOffset) - SizeOfSizeField) * DW + SizeOfSizeField;
}

FwInfo::section_map_t FwInfo::GetSectionInfo()
{
   if (const_cast<const FwInfoSource&>(m_rSource).Info().GetGeneration() == HW_GEN_UNKNOWN)
   {
      TBT_THROW(TBT_SDK_INTERNAL_ERROR, "Hardware information is not available");
   }

   FwInfo::section_map_t dict;

   uint32_t digital_offset = m_rSource.DigitalSectionOffset();
   dict[Digital]           = SectionDetails(digital_offset, SectionSize16Bit(digital_offset));

   uint32_t arc_params_offset = 0x0075;
   uint32_t arc_params_length = 4;
   uint32_t arcStartOffset    = tbt::ToUInt32(m_rSource.Read(digital_offset + arc_params_offset, arc_params_length), 0);

   dict[ArcParams] = SectionDetails(digital_offset + arcStartOffset,
                                    tbt::ToUInt32(m_rSource.Read(digital_offset + arcStartOffset, 4), 0) * DW);

   auto pdPointer = tbt::ToUInt32(m_rSource.Read(dict[ArcParams].Offset + 0x10c, 4), 0);
   if (tbt::ValidNVMPointer(pdPointer, 4))
   {
      dict[Pd] = SectionDetails(pdPointer + digital_offset, 0);
   }

   return dict;
}

void FwInfo::GetDROMInfo(section_map_t& dict)
{
   uint32_t dromOffset      = 0x010E;
   uint32_t dromLength      = 4;
   uint32_t dromStartOffset = tbt::ToUInt32(m_rSource.Read(dict[Digital].Offset + dromOffset, dromLength), 0);

   if (dromStartOffset == 0)
   {
      return;
   }

   uint32_t dromSectionOffset = dict[Digital].Offset + dromStartOffset;

   dict[DROM] = SectionDetails(dromSectionOffset,
                               // DROM length isn't implemented yet and should be ignored for now
                               tbt::ToUInt32(m_rSource.Read(dromSectionOffset, 4), 0) * DW);
}

} // namespace fwu

} // namespace tbt
