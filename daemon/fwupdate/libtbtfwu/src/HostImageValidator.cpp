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

#include "HostImageValidator.h"
#include "tbt/tbt_fwu_err.h"
#include <string>
#include <list>
#include <stdint.h>
#include <memory>

namespace tbt
{
namespace fwu
{

template <typename... Types>
std::shared_ptr<HostImageValidator::HostCheckLocation> ms(Types... args)
{
   return std::make_shared<HostImageValidator::HostCheckLocation>(args...);
}

#ifndef NDEBUG
void HostImageValidator::HostCheckLocation::check()
{
}
#endif

std::list<std::shared_ptr<HostImageValidator::HostCheckLocation>>
HostImageValidator::GetHostCheckLocations(HwGeneration gen)
{
   // Note that we would like to return using a nice initializer list
   // idiom here, e.g. return {ms(...), ms(...), ... }.  But there is a
   // gcc bug causing a memory leak if one of the ms(...) calls throws:
   // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66139
   // So we go with this slightly less attractive idiom using push_back.
   std::list<std::shared_ptr<HostCheckLocation>> out;
   switch (gen)
   {
   case DSL5520_5320: // release notes rev1-28, lane swap configuration rev1
   {
      out.push_back(ms(_1Port, 0x5, 2, "Device ID"));
      out.push_back(ms(_1Port, 0x10, 4, "PCIe Settings"));
      out.push_back(ms(_1Port, 0x143, 1, "CIO-Port0_TX"));
      out.push_back(ms(_1Port, 0x153, 1, "CIO-Port0_RX"));
      out.push_back(ms(_1Port, 0x147, 1, "CIO-Port1_TX"));
      out.push_back(ms(_1Port, 0x157, 1, "CIO-Port1_RX"));
      out.push_back(ms(_2Ports, 0x14b, 1, "CIO-Port2_TX"));
      out.push_back(ms(_2Ports, 0x15b, 1, "CIO-Port2_RX"));
      out.push_back(ms(_2Ports, 0x14f, 1, "CIO-Port3_TX"));
      out.push_back(ms(_2Ports, 0x15f, 1, "CIO-Port3_RX"));
      out.push_back(ms(_1Port, 0x211, 1, "Snk0_0(DP-in)"));
      out.push_back(ms(_1Port, 0x215, 1, "Snk0_1(DP-in)"));
      out.push_back(ms(_1Port, 0x219, 1, "Snk0_2(DP-in)"));
      out.push_back(ms(_1Port, 0x21D, 1, "Snk0_3(DP-in)"));
      out.push_back(ms(_2Ports, 0X11C3, 1, "Snk1_0(DP-in)"));
      out.push_back(ms(_2Ports, 0X11C7, 1, "Snk1_1(DP-in)"));
      out.push_back(ms(_2Ports, 0X11CB, 1, "Snk1_2(DP-in)"));
      out.push_back(ms(_2Ports, 0X11CF, 1, "Snk1_3(DP-in)"));
      out.push_back(ms(_1Port, 0X2175, 1, "PA(DP-out)"));
      out.push_back(ms(_1Port, 0X2179, 1, "PB(DP-out)"));
      out.push_back(ms(_1Port, 0X217D, 1, "Src0(DP-out)", 0b10101010));
   };
      return out;
   case DSL5110: // release notes rev3-9, lane swap configuration rev2
   {
      out.push_back(ms(_1Port, 0x5, 2, "Device ID"));
      out.push_back(ms(_1Port, 0x10, 4, "PCIe Settings"));
      out.push_back(ms(_1Port, 0x14f, 1, "CIO-Port0_TX"));
      out.push_back(ms(_1Port, 0x157, 1, "CIO-Port0_RX"));
      out.push_back(ms(_1Port, 0x153, 1, "CIO-Port1_TX"));
      out.push_back(ms(_1Port, 0x15b, 1, "CIO-Port1_RX"));
      out.push_back(ms(_1Port, 0x1f1, 1, "Snk0_0(DP-in)"));
      out.push_back(ms(_1Port, 0x1f5, 1, "Snk0_1(DP-in)"));
      out.push_back(ms(_1Port, 0x1f9, 1, "Snk0_2(DP-in)"));
      out.push_back(ms(_1Port, 0x1fD, 1, "Snk0_3(DP-in)"));
      out.push_back(ms(_1Port, 0X11A5, 1, "PA(DP-out)"));
   };
      return out;
   case DSL6540_6340: // B0, release notes rev1, lane swap configuration rev2
   case JHL6540_6340: // C0, release notes rev0, lane swap configuration rev3
   {
      out.push_back(ms(_1Port, 0x5, 2, "Device ID"));
      out.push_back(ms(_1Port, 0x10, 4, "PCIe Settings"));
      out.push_back(ms(_1Port, 0x12, 1, "PA", 0b11001100, DRAMUCode));
      out.push_back(ms(_2Ports, 0x13, 1, "PB", 0b11001100, DRAMUCode));
      out.push_back(ms(_1Port, 0x121, 1, "Snk0"));
      out.push_back(ms(_1Port, 0x129, 1, "Snk1"));
      out.push_back(ms(_1Port, 0x136, 1, "Src0", 0xF0));
      out.push_back(ms(_1Port, 0xB6, 1, "PA/PB (USB2)", 0b11000000));
   };
      return out;
   case JHL6240: // A0, release notes rev1, lane swap configuration rev1
   {
      out.push_back(ms(_1Port, 0x5, 2, "Device ID"));
      out.push_back(ms(_1Port, 0x10, 4, "PCIe Settings"));
      out.push_back(ms(_1Port, 0x12, 1, "PA", 0b11001100, DRAMUCode));
      out.push_back(ms(_1Port, 0x13, 1, "PB", 0b01000100, DRAMUCode));
      out.push_back(ms(_1Port, 0x121, 1, "Snk0"));
      out.push_back(ms(_1Port, 0xB6, 1, "PA/PB (USB2)", 0b11000000));
   };
      return out;
   default:
      TBT_THROW(TBT_SDK_UNKNOWN_CHIP);
   }
}

bool HostImageValidator::MustHaveDROM(HwGeneration generation)
{
   return generation >= DSL6540_6340;
}

ImageValidator::location_list_t HostImageValidator::GetCheckLocations(HwGeneration generation)
{
   auto isHost = GetIsHostCheckLocation();
   isHost->SetErrorCode(TBT_SDK_IMAGE_FOR_DEVICE_ERROR);
   ImageValidator::location_list_t checkLocations;
   checkLocations.push_back(isHost);

   auto hostLocs = GetHostCheckLocations(generation);
   for (auto val : hostLocs)
   {
      auto hwType = val->GetHwType();
      if (hwType == _1Port || GetHwInfo().HwTypeEquals(hwType))
      {
         checkLocations.push_back(std::make_shared<CheckLocation>(*val));
      }
   }

   return checkLocations;
}
}
}
