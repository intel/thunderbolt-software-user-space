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

#include "tbtException.h"
#include "Utils.h"
#include "logger.h"

std::wstring utf8stringToWstring(const std::string& ut8String)
{
   return StringToWString(ut8String);
}

/**
 * this function return if this P2P host support authentication
 * 0 - not supported, in this case automatic connection will be established
 *     no need for user approval
 * 1 - supported, currently not implemented
 */
int32_t IsAuthenticationSupported()
{
   return 0;
}

uint32_t ControllerIDToToInt(const controlleriD& ID)
{
   uint32_t controller_id;
   std::wstringstream wss;
   wss << std::hex << ID;
   wss >> controller_id;
   return controller_id;
}

uint32_t GetDeviceIDFromControllerID(const controlleriD& ID)
{
   uint32_t DevIDInt;
   std::wstringstream wss;

   try
   {
      auto devIDstr = ID.substr(4, 4);
      ;
      wss << std::hex << devIDstr;
      wss >> DevIDInt;
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: GetDeviceIDFromControllerID failed (Exception: %s)", e.what());
      throw;
   }
   catch (...)
   {
      TbtServiceLogger::LogError("Error: GetDeviceIDFromControllerID unknown exception");
      throw;
   }
   return DevIDInt;
}
