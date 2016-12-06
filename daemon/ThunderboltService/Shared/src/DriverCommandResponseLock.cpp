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

#include "DriverCommandResponseLock.h"

namespace // anon. namespace
{
const std::string errorStr("Error: WaitForResponse failed");
} // anon. namespace

bool DriverCommandResponseLock::WaitForResponse(uint32_t* ResultCode /*= nullptr*/)
{
   try
   {
      if (!m_semaphore.wait_for(m_timeout).first)
      {
         return false;
      }
      if (ResultCode != nullptr)
      {
         *ResultCode = m_result;
      }
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("%s: %s", errorStr.c_str(), e.what());
   }
   catch (...)
   {
      TbtServiceLogger::LogError("%s", errorStr.c_str());
   }

   return true;
}

void DriverCommandResponseLock::UnlockWithResult(uint32_t res)
{
   // Using the semaphore's internal lock for ensuring atomic operation regarding result value, too
   auto lock = m_semaphore.lock();
   m_result  = res;
   m_semaphore.notify(std::move(lock));
}

void DriverCommandResponseLock::Unlock()
{
   m_semaphore.notify();
}

void DriverCommandResponseLock::ResetSemaphore()
{
   m_semaphore.reset();
}
