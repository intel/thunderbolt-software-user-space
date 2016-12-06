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

#include "FwInfoSource.h"
#include "Exceptions.h"
#include "log.h"
#include "FwInfo.h"
#include "util.h"
#include "tbt/tbt_fwu_err.h"
#include <assert.h>
#include <memory>

namespace
{

typedef std::unique_ptr<tbt::fwu::HwInfo> up;

up _HwConfiguration(uint16_t controllerId)
{
   using namespace tbt::fwu;
   switch (controllerId)
   {
   // DSL4510_4410
   case 0x1566:
   case 0x1567:
      return up(new HwInfo(DSL4510_4410, _1Port));
   case 0x1568:
   case 0x1569:
      return up(new HwInfo(DSL4510_4410, _2Ports));

   // DSL5520_DSL5320
   case 0x156A:
   case 0x156B:
      return up(new HwInfo(DSL5520_5320, _1Port));
   case 0x156C:
   case 0x156D:
      return up(new HwInfo(DSL5520_5320, _2Ports));

   // DSL5110
   case 0x157D:
   case 0x157E:
      return up(new HwInfo(DSL5110, _1Port));

   // DSL6540_6340
   case 0x1577:
   case 0x1578:
      return up(new HwInfo(DSL6540_6340, _2Ports));
   case 0x1575:
   case 0x1576:
      return up(new HwInfo(DSL6540_6340, _1Port));
   case 0x15DD:
      return up(new HwInfo(DSL6540_6340));

   // JHL6240
   case 0x15BF:
   case 0x15C0:
      return up(new HwInfo(JHL6240, _1Port));
   case 0x15DC:
      return up(new HwInfo(JHL6240));

   // JHL6540_6340
   case 0x15D2:
   case 0x15D3:
      return up(new HwInfo(JHL6540_6340, _2Ports));
   case 0x15D9:
   case 0x15DA:
      return up(new HwInfo(JHL6540_6340, _1Port));
   case 0x15DE:
      return up(new HwInfo(JHL6540_6340));

   default:
      TBT_LOG(LOG_WARNING, "Unknown chip 0x%hx", controllerId);
      TBT_THROW(TBT_SDK_UNKNOWN_CHIP);
   }
}

} // end anonymous namespace

namespace tbt
{
namespace fwu
{

std::unique_ptr<HwInfo> FwInfoSource::HwConfiguration(uint16_t controllerId)
{
   return _HwConfiguration(controllerId);
}

#ifndef NDEBUG
void HwInfo::check()
{
}
#endif

std::unique_ptr<HwInfo> FwInfoSource::GetHwConfiguration()
{
   std::vector<uint8_t> buf = Read(DigitalSectionOffset() + 0x5, 2);
   uint16_t controllerId    = tbt::ToUInt16(buf, 0);
   return HwConfiguration(controllerId);
}

FwInfoSource::FwInfoSource() noexcept : m_HwInfo(HW_GEN_UNKNOWN)
{
}
}
}
