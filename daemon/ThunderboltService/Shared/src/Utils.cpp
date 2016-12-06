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

#include <algorithm>
#include "Utils.h"
#include "boost/crc.hpp"
#include "tbtException.h"
#include <string>
#include <locale>
#include <iomanip>
#include <locale>
#include <limits.h>

std::wstring StringToWString(const std::string& Str)
{
   return std::wstring(Str.begin(), Str.end());
}

std::string WStringToString(const std::wstring& Str)
{
#if 0
   // This uses a C++11 feature that is not available in the stock RHEL7.2
   // gcc (4.8.5).  Disabled for now til we can come up with a reasonable
   // alternative.
   static std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
   return std::string(cvt.to_bytes(Str));
#else
   return std::string(Str.begin(), Str.end());
#endif
}

void ArrayDwordSwap(uint8_t* Array, size_t Size)
{
   if ((Size % 4) != 0)
   {
      TbtServiceLogger::LogError("Error: Trying to swap array of size that is not a multiple of 4!");
      throw TbtException("Trying to swap array of size that is not a multiple of 4!");
   }

   for (uint32_t i = 0; i < Size; i += sizeof(uint32_t))
   {
      std::reverse(Array + i, Array + i + sizeof(uint32_t));
   }
}

uint32_t CalculateCrc(uint8_t* Array, size_t Size)
{
   boost::crc_optimal<32, 0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, true, true> CrcVal;
   CrcVal.process_bytes(Array, Size);
   return CrcVal.checksum();
}

uint32_t ControllerSettingsToEnum(bool certifiedOnly, bool overrideFirstDepth)
{
   uint32_t flags = SET_FW_MODE_FD1_D1_CERT_COMMAND_CODE;
   flags |= (!certifiedOnly) << 0;
   flags |= overrideFirstDepth << 2;
   return flags;
}

std::string BoolToString(bool val)
{
   return std::string((val) ? "True" : "False");
}

std::wstring Backshlashify(const std::wstring& input)
{
   static const std::wstring from = L"\\";
   static const std::wstring to   = L"\\\\";

   size_t start_pos = 0;
   std::wstring Res = input;

   while ((start_pos = Res.find(from, start_pos)) != std::wstring::npos)
   {
      Res.replace(start_pos, from.length(), to);
      start_pos += to.length();
   }

   return Res;
}

std::wstring RemoveoDoubleSlash(const std::wstring& input)
{
   static const std::wstring from = L"\\\\";
   static const std::wstring to   = L"\\";

   size_t start_pos = 0;
   std::wstring Res = input;

   while ((start_pos = Res.find(from, start_pos)) != std::wstring::npos)
   {
      Res.replace(start_pos, from.length(), to);
      start_pos += to.length();
   }

   return Res;
}

uint32_t GetNomOfPortsFromControllerID(const controlleriD& ID)
{
   auto devID = GetDeviceIDFromControllerID(ID);
   switch (devID)
   {
   case 0x15dc:
   case 0x15dd:
   case 0x15de:
      return 0;
   case 0x1566:
   case 0x156a:
   case 0x157d:
   case 0x1575:
   case 0x15bf:
   case 0x15d9:
      return 1;
   case 0x1568:
   case 0x156c:
   case 0x1577:
   case 0x15d2:
      return 2;
   default:
      throw TbtException("Unknown device type");
   }
}

ThunderboltGeneration GetGenerationFromControllerID(const controlleriD& ID)
{
   auto devID = GetDeviceIDFromControllerID(ID);
   switch (devID)
   {
   case 0x1566:
   case 0x1568:
      return ThunderboltGeneration::THUNDERBOLT_1;
   case 0x156a:
   case 0x156c:
   case 0x157d:
      return ThunderboltGeneration::THUNDERBOLT_2;
   case 0x1575:
   case 0x1577:
   case 0x15bf:
   case 0x15d9:
   case 0x15dc:
   case 0x15dd:
   case 0x15de:
   case 0x15d2:
      return ThunderboltGeneration::THUNDERBOLT_3;
   default:
      throw TbtException("Unknown device ID");
   }
}

const std::wstring Version::NA = L"N/A";

Version::Version(uint32_t major, uint32_t minor) : m_major(major), m_minor(minor)
{
   std::wostringstream str;
   str << std::hex << major << L"." << std::setw(2) << std::setfill(L'0') << minor;
   m_str = str.str();
}

Version::Version(const std::wstring& version) : m_str(version)
{
   try
   {
      if (version != NA)
      {
         std::wistringstream str(version);
         str.exceptions(std::ios::badbit | std::ios::failbit);
         uint32_t major;
         wchar_t separator;
         uint32_t minor;
         str >> std::hex >> major >> separator >> minor;
         m_major = major;
         m_minor = minor;
      }
   }
   catch (...)
   {
      TbtServiceLogger::LogDebug("Version string parsing failed");
   }
}
