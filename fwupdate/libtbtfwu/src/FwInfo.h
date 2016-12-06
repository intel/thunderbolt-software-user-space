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

#pragma once

#include <stdint.h>
#include "FwInfoSource.h"
#include <unordered_map>
#include <string.h>
#include <limits.h>

namespace tbt
{

namespace fwu
{

enum Sections
{
   Digital, // (active region)
   DROM,
   DRAMUCode,
   ArcParams,
   Pd,
};

struct FwLocation
{
   uint32_t Offset;
   uint32_t Length;
};

class FwInfo
{
public:
   const HwInfo& GetHwInfo() const noexcept;
   struct SectionDetails
   {
      uint32_t Offset;
      uint32_t Length;
      void swap(SectionDetails& other) noexcept
      {
         std::swap(Offset, other.Offset);
         std::swap(Length, other.Length);
      }
      SectionDetails(uint32_t Offset_, uint32_t Length_) noexcept : Offset(Offset_), Length(Length_) {}
      SectionDetails(const SectionDetails& other) noexcept : Offset(other.Offset), Length(other.Length) {}
      SectionDetails& operator=(const SectionDetails& other) noexcept
      {
         SectionDetails(other).swap(*this);
         return *this;
      }
      SectionDetails() noexcept : Offset(uint32_t(-1)), Length(uint32_t(-1)) {}
   };

   struct HashSectionDetails
   {
      size_t operator()(const SectionDetails& s) const
      {
         size_t h = 0;
         uint8_t a[8];
         memcpy(&a[0], &(s.Offset), 4);
         memcpy(&a[4], &(s.Length), 4);
         for (int i = 0; i < 8; i++)
         {
            h = h ^ (h << 5) ^ a[i];
         }
         const static size_t mask = -1 ^ ((size_t)(1) << ((sizeof(size_t) * CHAR_BIT - 1)));
         return h & mask;
      }
   };
   struct HashSections
   {
      size_t operator()(Sections s) const { return (size_t)(s); }
   };

   typedef std::unordered_map<Sections, SectionDetails, HashSections> section_map_t;
   virtual section_map_t GetSectionInfo();

protected:
   FwInfo(FwInfoSource& source) noexcept;

   /// <summary>
   /// Gets section size by reading from given offset, assuming the size field is 16-bit long,
   /// doesn't include the size field itself and the size field is in DWORD (4-bytes) units
   /// </summary>
   /// <param name="sectionOffset">Offset to read from</param>
   /// <returns>Section size (in bytes) as read from FW + 2 for the size field itself</returns>
   uint32_t SectionSize16BitDw(uint32_t sectionOffset);

   void GetDROMInfo(section_map_t& dict);

private:
   uint32_t SectionSize16Bit(uint32_t sectionOffset);

   const static uint32_t DW = sizeof(uint32_t);

protected:
   FwInfoSource& m_rSource;
};
}
}
