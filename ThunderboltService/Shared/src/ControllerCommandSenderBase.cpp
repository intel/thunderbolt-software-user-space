/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2009 - 2016 Intel Corporation.
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

#include "ControllerCommandSenderBase.h"
#include "Utils.h"
#include "logger.h"
#include "Utils.h"

const uint32_t NVM_IMAGE_START_OFFSET = 0x2000;
const uint32_t NVM_IMAGE_CSS_OFFSET   = 0x10;
const uint32_t NVM_MAX_DW_TO_WRITE    = 16;
const uint32_t CSS_HEADER_COMMAND     = 0x3FFFFF;

ControllerCommandSenderBase::ControllerCommandSenderBase(void)
{
}

//
// FIXME: These aren't actually used - should the be removed?
//

TBTDRV_STATUS ControllerCommandSenderBase::WriteNVMImage(const controlleriD& cID,
                                                         const bool IsInSafeMode,
                                                         uint32_t BufferSize,
                                                         std::vector<uint8_t> Buffer,
                                                         const ROUTE_STRING& routeString,
                                                         int dmaPort,
                                                         ThunderboltGeneration generation) const
{

   uint32_t TargetOffset   = 0;
   uint8_t* pBufferCurrent = (uint8_t*)&Buffer.front();
   uint8_t* pBufferEnd     = pBufferCurrent + BufferSize;
   TBTDRV_STATUS flashRes  = TBTDRV_STATUS::SUCCESS_RESPONSE_CODE;

   // check image
   // FIXME: add sanity check for the image
   if (Buffer.size() != BufferSize || BufferSize % sizeof(uint32_t) != 0)
   {
      return TBTDRV_STATUS::WRONG_IMAGE_SIZE_CODE;
   }

   // The CSS headers are written only in generations that are <AR
   if (generation != ThunderboltGeneration::THUNDERBOLT_3)
   {
      // CSS Headers
      TbtServiceLogger::LogInfo("FwUpdate: Writing CSS header - part 1");
      flashRes = WriteToFlash(cID,
                              pBufferCurrent + NVM_IMAGE_CSS_OFFSET,
                              CSS_HEADER_COMMAND,
                              FLASH_REGION::CSS_HEADER,
                              NVM_MAX_DW_TO_WRITE,
                              routeString,
                              dmaPort);
      if (flashRes != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
      {
         TbtServiceLogger::LogError("Error: WriteToFlash failed (status: %u)", flashRes);
         return flashRes;
      }

      TbtServiceLogger::LogInfo("FwUpdate: Writing CSS header - part 2");

      flashRes = WriteToFlash(cID,
                              pBufferCurrent + NVM_IMAGE_CSS_OFFSET + NVM_MAX_DW_TO_WRITE * sizeof(uint32_t),
                              CSS_HEADER_COMMAND,
                              FLASH_REGION::CSS_HEADER,
                              NVM_MAX_DW_TO_WRITE,
                              routeString,
                              dmaPort);

      if (flashRes != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
      {
         TbtServiceLogger::LogError("Error: WriteToFlash failed (status: %u)", flashRes);
         return flashRes;
      }
   }
   // The offset needs to be changed if the controller is less than AR, or is AR and is not in safe mode
   if (generation != ThunderboltGeneration::THUNDERBOLT_3 || !IsInSafeMode)
   {
      pBufferCurrent += GetImageStartOffset(Buffer); // NVM_IMAGE_START_OFFSET;
   }

   // Image
   TbtServiceLogger::LogInfo("FwUpdate: Writing FW image");

   while (pBufferCurrent < pBufferEnd)
   {
      int NumDW = ((pBufferEnd - pBufferCurrent) / sizeof(uint32_t)) >= NVM_MAX_DW_TO_WRITE
                     ? NVM_MAX_DW_TO_WRITE
                     : (pBufferEnd - pBufferCurrent) / (sizeof(uint32_t));

      flashRes =
         WriteToFlash(cID, pBufferCurrent, TargetOffset, FLASH_REGION::NON_ACTIVE_REGION, NumDW, routeString, dmaPort);

      if (flashRes != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
      {
         TbtServiceLogger::LogError(
            "Error: WriteToFlash failed (status: %u, place in image: %u)", flashRes, pBufferCurrent - Buffer.data());
         return flashRes;
      }

      TargetOffset += NumDW;
      pBufferCurrent += NumDW * sizeof(uint32_t);
   }
   return flashRes;
}

uint32_t ControllerCommandSenderBase::GetImageStartOffset(std::vector<uint8_t> Buffer) const
{
   uint32_t* pBufferCurrent = (uint32_t*)&Buffer.front();
   return *pBufferCurrent & 0xffffff;
}
