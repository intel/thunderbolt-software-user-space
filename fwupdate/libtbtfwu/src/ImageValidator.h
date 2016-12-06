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

#include "FwInfo.h"
#include "Exceptions.h"
#include "Controller.h"
#include "tbt/tbt_fwu_err.h"
#include <memory>
#include <vector>
#include <list>

namespace tbt
{
namespace fwu
{

class ImageValidator
{
public:
   /// <summary>
   /// Validates FW image file compatibility with the controller
   /// </summary>
   void Validate();

protected:
   /// <summary>
   /// This class is used for passing information about FW locations to compare between the
   /// controller and the FW image file to validate compatibility.
   /// </summary>
   class CheckLocation
   {
   public:
      uint32_t GetOffset() const { return m_Offset; }
      uint32_t GetLength() const { return m_Length; }
      uint8_t GetMask() const { return m_Mask; }
      Sections GetSection() const { return m_Section; }
      /// <summary>
      /// Used in the exception thrown in case of a mismatch
      /// </summary>
      tbt::TbtStatus GetErrorCode() const { return m_ErrorCode; }
      void SetErrorCode(tbt::TbtStatus s) { m_ErrorCode = s; }

      virtual ~CheckLocation() {}

      /// <summary>
      /// Used in the error message in case of a mismatch
      /// </summary>
      std::string GetDescription() const { return m_Description; }

      const static uint8_t FullMask;

      CheckLocation(uint32_t offset,
                    uint32_t length,
                    uint8_t mask             = FullMask,
                    Sections section         = Digital,
                    tbt::TbtStatus errorCode = TBT_SDK_IMAGE_VALIDATION_ERROR,
                    std::string description  = std::string());

   private:
      uint32_t m_Offset;
      uint32_t m_Length;
      uint8_t m_Mask;
      Sections m_Section;
      tbt::TbtStatus m_ErrorCode;
      std::string m_Description;
   };

   /// <summary>
   /// Returns check locations for validating FW image file compatibility with the controller
   /// </summary>
   /// <param name="generation">Controller generation</param>
   /// <returns>Container with check locations to compare</returns>
   typedef std::list<std::shared_ptr<CheckLocation>> location_list_t;
   virtual location_list_t GetCheckLocations(HwGeneration generation) = 0;

   /// <summary>
   /// Returns if DROM existence must be enforced
   /// </summary>
   /// <param name="generation">Controller generation</param>
   /// <returns>True to enforce DROM existence; otherwise - false</returns>
   virtual bool MustHaveDROM(HwGeneration generation) = 0;

   /// <summary>
   /// Helper function with common check location that needs error code customization in
   /// derived classes
   /// </summary>
   /// <returns>Check location in FW to know if it's host controller or device</returns>
   static std::shared_ptr<CheckLocation> GetIsHostCheckLocation()
   {
      return std::make_shared<CheckLocation>(0x10, 1, 0x02);
   }

   ImageValidator(tbt::Controller& controller,
                  const std::vector<uint8_t>& rImage,
                  const FwInfo::section_map_t& rControllerSections,
                  const FwInfo::section_map_t& rFileSections,
                  const HwInfo& rHwInfo);

   const HwInfo& GetHwInfo() const { return m_rHwInfo; }

private:
   tbt::Controller& m_rController;
   const std::vector<uint8_t>& m_rImage;
   const FwInfo::section_map_t& m_rControllerSections;
   const FwInfo::section_map_t& m_rFileSections;
   const HwInfo& m_rHwInfo;

private:
   /// <summary>
   /// Validates that image file fits into HW chip size
   /// </summary>
   void ValidateChipSize();

   /// <summary>
   /// Validates that either both or neither controller and FW image file contain PD FW
   /// </summary>
   void ComparePdExistence();

   /// <summary>
   /// Validates that both controller and FW image file include DROM or neither of them has
   /// it, enforces DROM existence if needed, and compares DROM information if applied
   /// </summary>
   void CompareDROM();

   /// <summary>
   /// Compares each of the given check locations to validate FW image file compatibility with
   /// the controller
   /// </summary>
   /// <param name="checkLocations">List of FW locations to compare</param>
   void Compare(const location_list_t& checkLocations);

   std::vector<uint8_t> ReadFromFw(const CheckLocation& loc);
   std::vector<uint8_t> ReadFromFile(const CheckLocation& loc);

   static void ApplyMask(std::vector<uint8_t>& val, uint8_t mask);
};
}
}
