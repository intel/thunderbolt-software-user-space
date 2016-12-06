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

#pragma once

#include <boost/noncopyable.hpp>
#include "MessagesWrapper.h"
#include "logger.h"
#include "tbtException.h"
#include "../CommonUtils/CountingSemaphore.h"

class DriverCommandResponseLock : private boost::noncopyable
{
public:
   DriverCommandResponseLock(std::chrono::seconds timeout) : m_semaphore(), m_result(0), m_timeout(timeout) {}
   virtual ~DriverCommandResponseLock() = default;

   /// return false if time out, otherwise true
   bool WaitForResponse(uint32_t* ResultCode = nullptr);

   void UnlockWithResult(uint32_t res);
   void Unlock();

   /**
    * \brief Reset the internal semaphore when we want to ignore response to previous driver commands
    *
    * Useful after FW update, when we do power cycle and don't want to count on FW response to the
    * power cycle command, but on the other hand have to handle the cases where we do get such a
    * response, so we reset the semaphore when driver ready message arrives.
    */
   void ResetSemaphore();

private:
   // Using counting semaphore instead of std::condition_variable to solve race condition between
   // the waiting thread and the notifier thread
   Utils::CountingSemaphore m_semaphore;
   uint32_t m_result = 0;
   std::chrono::seconds m_timeout;
};
