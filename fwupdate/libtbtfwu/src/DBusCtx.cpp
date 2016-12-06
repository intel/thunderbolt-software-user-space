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

#include "DBusCtx.h"
#include "log.h"
#include <stdexcept>
#include <memory>
#include <stdlib.h>

namespace tbt
{

namespace internal
{

DispatcherSetter::DispatcherSetter(DBus::BusDispatcher& d)
{
   DBus::default_dispatcher = &d;
}
DispatcherSetter::~DispatcherSetter()
{
   DBus::default_dispatcher = nullptr;
}

#ifndef NDEBUG
// These are overridden by UTs to test allocation failures.
void* DispatcherSetter::operator new(size_t count)
{
   return malloc(count);
}
void DispatcherSetter::operator delete(void* ptr)
{
   free(ptr);
}
#endif

} // namespace internal

DBusCtx::DBusCtx(const char* ept, int bSystemBus) : m_sEndpoint(ept), m_dispatcher(), m_pCx(), m_pDispatcherSetter()
{
   if (s_objCount++ > 0)
   {
      --s_objCount;
      throw std::runtime_error("Cannot have more than one DBusCtx "
                               "in the app at a time.");
   }
   try
   {
      m_pDispatcherSetter.reset(new internal::DispatcherSetter(m_dispatcher));
      m_pCx.reset(new DBus::Connection(bSystemBus ? DBus::Connection::SystemBus() : DBus::Connection::SessionBus()));
   }
   catch (...)
   {
      --s_objCount;
      throw;
   }
}
DBusCtx::~DBusCtx()
{
   --s_objCount;
}

std::atomic<int> DBusCtx::s_objCount(0);

} // namespace tbt
