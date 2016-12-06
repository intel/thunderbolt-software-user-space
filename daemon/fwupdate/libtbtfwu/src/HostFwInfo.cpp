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

#include "HostFwInfo.h"
#include "Exceptions.h"
#include "util.h"
#include "tbt/tbt_fwu_err.h"

namespace tbt
{
namespace fwu
{

HostFwInfo::HostFwInfo(FwInfoSource& source) : FwInfo(source)
{
}

FwInfo::section_map_t HostFwInfo::GetSectionInfo()
{
   section_map_t dict = FwInfo::GetSectionInfo();

   switch (const_cast<const FwInfoSource&>(m_rSource).Info().GetGeneration())
   {
   case DSL5520_5320:
   case DSL5110:
      // There is nothing else that is interesting for us
      break;

   case DSL6540_6340:
   case JHL6240:
   case JHL6540_6340:
   {
      GetDROMInfo(dict);

      /*
       * Algorithm:
       * To find the DRAM section, we have to jump from section to section in a
       * chain of sections.
       * readUcodeSections location tells what sections exist at all (with flags
       * as defined in SectionReadBit enum).
       * ee_ucode_start_addr location tells the offset of the first section in
       * the list according to the digital section start.
       * After having the offset of the first section, we have a loop over the
       * section list. If the section exists, we read its length (2 bytes at
       * section start) and add it to current offset to find the start of the
       * next section. Otherwise, we already have the next section offset at hand...
       */

      uint32_t readUcodeSections_offset = dict[Digital].Offset + 0x2;
      uint32_t readUcodeSections_length = 1;
      SectionReadBit sectionRead =
         *reinterpret_cast<SectionReadBit*>(&(m_rSource.Read(readUcodeSections_offset, readUcodeSections_length)[0]));
      if (sectionRead.DRAM)
      {
         uint32_t ee_ucode_start_addr_offset = dict[Digital].Offset + 0x3;
         uint32_t ee_ucode_start_addr_length = 2;
         uint32_t offset                     = dict[Digital].Offset
                           + tbt::ToUInt16(m_rSource.Read(ee_ucode_start_addr_offset, ee_ucode_start_addr_length), 0);

         if (sectionRead.CP)
         {
            offset += SectionSize16BitDw(offset);
         }
         if (sectionRead.DPOUT)
         {
            offset += SectionSize16BitDw(offset);
         }
         if (sectionRead.DPIN)
         {
            offset += SectionSize16BitDw(offset);
         }
         if (sectionRead.LC)
         {
            offset += SectionSize16BitDw(offset);
         }
         if (sectionRead.ARC)
         {
            offset += SectionSize16BitDw(offset);
         }
         if (sectionRead.IRAM)
         {
            offset += SectionSize16BitDw(offset);
         }

         dict[DRAMUCode] = SectionDetails(offset, SectionSize16BitDw(offset));
      }
      break;
   }

   default:
      TBT_THROW(TBT_SDK_UNKNOWN_CHIP);
   }
   return dict;
}

} // namespace fwu
} // namespace tbt
