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
#include "Controller.h"
#include "ImageValidator.h"
#include "FwInfo.h"

namespace tbt
{
namespace fwu
{

class HostImageValidator : public ImageValidator
{
public:
   HostImageValidator(Controller& rController,
                      const std::vector<uint8_t>& image,
                      const FwInfo::section_map_t& rControllerSections,
                      const FwInfo::section_map_t& rFileSections,
                      const HwInfo& rHwInfo)
      : ImageValidator(rController, image, rControllerSections, rFileSections, rHwInfo)
   {
   }

private:
   class HostCheckLocation : public CheckLocation
   {
   public:
      // In this context, 1Port type means that it's relevant for both 2C *and* 4C controllers
      // while 2Ports means that it's relevant only for 4C controllers.
      HwType GetHwType() const { return m_HwType; }

      HostCheckLocation(HwType type,
                        uint32_t offset,
                        uint32_t length,
                        std::string description,
                        uint8_t mask     = FullMask,
                        Sections section = Digital)
         : CheckLocation(offset, length, mask, section, TBT_SDK_IMAGE_VALIDATION_ERROR, description)
      {
#ifndef NDEBUG
         check();
#endif
         m_HwType = type;
      }

   private:
#ifndef NDEBUG
      static void check();
#endif
      HwType m_HwType;
   };

protected:
   /// <summary>
   /// This function includes the tables with locations in FW to compare to make sure the FW image file
   /// is compatible with the controller.
   /// The list of this locations comes from the release notes of the FW (aka NVM) and it's compatible with the
   /// current latest revision for each controller generation.
   /// </summary>
   /// <param name="gen">The controller generation</param>
   /// <returns>Table with locations to check for image compatibility with the controller</returns>
   virtual location_list_t GetCheckLocations(HwGeneration generation);

   virtual bool MustHaveDROM(HwGeneration generation);

private:
   std::list<std::shared_ptr<HostCheckLocation>> GetHostCheckLocations(HwGeneration gen);
};
}
}
