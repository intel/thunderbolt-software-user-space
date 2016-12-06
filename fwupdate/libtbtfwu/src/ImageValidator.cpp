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

#include "ImageValidator.h"
#include "Exceptions.h"
#include "util.h"
#include <inttypes.h>

namespace tbt
{
namespace fwu
{

const uint8_t ImageValidator::CheckLocation::FullMask = 0xff;

ImageValidator::ImageValidator(tbt::Controller& controller,
                               const std::vector<uint8_t>& rImage,
                               const FwInfo::section_map_t& rControllerSections,
                               const FwInfo::section_map_t& rFileSections,
                               const HwInfo& rHwInfo)
   : m_rController(controller),
     m_rImage(rImage),
     m_rControllerSections(rControllerSections),
     m_rFileSections(rFileSections),
     m_rHwInfo(rHwInfo)
{
}

ImageValidator::CheckLocation::CheckLocation(uint32_t offset,
                                             uint32_t length,
                                             uint8_t mask,
                                             Sections section,
                                             tbt::TbtStatus errorCode,
                                             std::string description)
   : m_Offset(offset),
     m_Length(length),
     m_Mask(mask),
     m_Section(section),
     m_ErrorCode(errorCode),
     m_Description(description)
{
   if (mask != FullMask && length > 1)
   {
      // We expect mask to be applied for 1 byte only.
      // If we find such cases in the future, other code must be
      // changed to remove this assumption.
      TBT_THROW(TBT_SDK_INTERNAL_ERROR, "Mask isn't supported for data with length > 1.");
   }
}

void ImageValidator::Validate()
{
   ValidateChipSize();
   ComparePdExistence();
   CompareDROM();
   Compare(GetCheckLocations(GetHwInfo().GetGeneration()));
}

void ImageValidator::ValidateChipSize()
{
   // 0x45[2:0] is flash chip size in FR and AR FW
   CheckLocation flashSizeCheckLocation(0x45, 1, 0b00000111, Digital, TBT_SDK_CHIP_SIZE_ERROR);

   uint8_t chipSize      = ReadFromFw(flashSizeCheckLocation)[0];
   uint8_t imageChipSize = ReadFromFile(flashSizeCheckLocation)[0];
   if (chipSize < imageChipSize)
   {
      TBT_THROW(flashSizeCheckLocation.GetErrorCode());
   }
}

void ImageValidator::ComparePdExistence()
{
   CheckLocation pdPointer(0x010C, 4, CheckLocation::FullMask, ArcParams);

   bool controllerHasPd = ValidNVMPointer(ToUInt32(ReadFromFw(pdPointer), 0), pdPointer.GetLength());
   bool imageHasPd      = ValidNVMPointer(ToUInt32(ReadFromFile(pdPointer), 0), pdPointer.GetLength());

   if (controllerHasPd != imageHasPd)
   {
      TBT_THROW(TBT_SDK_PD_MISMATCH);
   }
}

void ImageValidator::CompareDROM()
{
   bool fileHasDROM       = m_rFileSections.find(DROM) != m_rFileSections.end();
   bool controllerHasDROM = m_rControllerSections.find(DROM) != m_rControllerSections.end();

   if (fileHasDROM != controllerHasDROM)
   {
      TBT_THROW(TBT_SDK_DROM_MISMATCH);
   }

   if (!fileHasDROM)
   {
      if (!MustHaveDROM(GetHwInfo().GetGeneration()))
      {
         return;
      }
      TBT_THROW(TBT_SDK_NO_DROM_IN_FILE_ERROR);

      // For internal validation, with devices without DROM, comment out the previous
      // line and uncomment the following lines

      // Console.WriteLine(Resources.NoDromWarning);
      // return;
   }

   location_list_t dromCheckLocations = {
      std::make_shared<CheckLocation>(0x10, 2, CheckLocation::FullMask, DROM, TBT_SDK_VENDOR_MISMATCH),
      std::make_shared<CheckLocation>(0x12, 2, CheckLocation::FullMask, DROM, TBT_SDK_MODEL_MISMATCH),
   };

   Compare(dromCheckLocations);
}

void ImageValidator::Compare(const ImageValidator::location_list_t& checkLocations)
{
   for (auto val : checkLocations)
   {
      auto valueFromFw   = ReadFromFw(*val);
      auto valueFromFile = ReadFromFile(*val);

      if (valueFromFw != valueFromFile)
      {
         if (val->GetDescription() != "")
         {
            TBT_THROW(val->GetErrorCode(),
                      "Firmware image does not match current hardware: " + val->GetDescription() + "\n"
                         + "Please supply a valid image file.");
         }
         TBT_THROW(val->GetErrorCode());
      }
   }
}

std::vector<uint8_t> ImageValidator::ReadFromFw(const ImageValidator::CheckLocation& loc)
{
   std::vector<uint8_t> val =
      m_rController.ReadFirmware(m_rControllerSections.at(loc.GetSection()).Offset + loc.GetOffset(), loc.GetLength());
   ApplyMask(val, loc.GetMask());
   return val;
}

std::vector<uint8_t> ImageValidator::ReadFromFile(const ImageValidator::CheckLocation& loc)
{
   auto start = m_rImage.begin() + m_rFileSections.at(loc.GetSection()).Offset + loc.GetOffset();
   std::vector<uint8_t> val(start, start + loc.GetLength());
   ApplyMask(val, loc.GetMask());
   return val;
}

void ImageValidator::ApplyMask(std::vector<uint8_t>& val, uint8_t mask)
{
   if (mask != CheckLocation::FullMask)
   {
      val[0] &= mask;
   }
}

} // namespace fwu
} // namespace tbt
