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
#pragma once
#include <thread>
#include <vector>
#include "IP2PDevice.h"
#include "defines.h"
#include "Sdk.h"

/* IControllerCommandSender Interface
 * ----------------------------------
 * Interface to the host controller, allow sending requests to the FW
 *
 *
 *
 */
class IControllerCommandSender
{
public:
   virtual ~IControllerCommandSender(){};
   virtual void SendTbtWmiApproveP2P(const IP2PDevice& Device) const = 0;
   virtual void
   SendTbtWmiMessageToFW(const controlleriD& Cid, uint8_t* message, size_t messageSize, PDF_VALUE pdf) const = 0;
   virtual void SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd) const = 0;
   virtual void SendPowerCycleCommand(const controlleriD& Cid, int dmaPort, const ROUTE_STRING& routeString) const = 0;

   // SDK Methods
   virtual TBTDRV_STATUS ReadBlockFromFlash(const controlleriD& cID,
                                            std::vector<uint8_t>& destination_buffer,
                                            uint32_t source_offset,
                                            uint32_t bytes_count,
                                            const ROUTE_STRING& routeString,
                                            int dmaPort) const = 0;
   virtual TBTDRV_STATUS WriteToFlash(const controlleriD& cID,
                                      uint8_t* source,
                                      uint32_t destination_offset,
                                      FLASH_REGION destination_region,
                                      uint32_t dword_count,
                                      const ROUTE_STRING& routeString,
                                      int dmaPort) const = 0;
   virtual TBTDRV_STATUS WriteNVMImage(const controlleriD& cID,
                                       const bool IsInSafeMode,
                                       uint32_t BufferSize,
                                       std::vector<uint8_t> Buffer,
                                       const ROUTE_STRING& routeString,
                                       int dmaPort,
                                       ThunderboltGeneration generation) const = 0;
   virtual uint32_t GetImageStartOffset(std::vector<uint8_t> Buffer) const     = 0;
   virtual TBTDRV_STATUS
   FlashAuthentication(const controlleriD& cID, const ROUTE_STRING& routeString, int dmaPort) const = 0;
};
