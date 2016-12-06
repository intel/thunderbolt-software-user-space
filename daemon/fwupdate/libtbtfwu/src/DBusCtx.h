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

/**
 * @file TbtDBusCtx.h
 */

#pragma once

#include "tbtdbus.h"

#include <atomic>
#include <memory>

namespace tbt
{

namespace internal
{

/**
 * RAII object for setting the global D-Bus dispatcher object.  On
 * construction, sets the DBus default dispatcher to a value.  On destruction,
 * clears the DBus default dispatcher.
 */
class DispatcherSetter
{
public:
   DispatcherSetter(DBus::BusDispatcher& d);
   ~DispatcherSetter();
#ifndef NDEBUG
   // For debug builds, we allow test code to override the 'new' and 'delete'
   // behaviors for objets of this type.  This allows easier testing of
   // serious failure modes, such as memory allocation errors.
   void* operator new(size_t count);
   void operator delete(void* ptr);
#endif
};

} // tbt::internal

/**
 * The overall DBus connection context for the application.  Only one
 * of these may exist in the application at a time, because the unique
 * instance owns the DBus default_dispatcher.
 */
class DBusCtx
{
public:
   DBusCtx(const char* ept, int bSystemBus);
   ~DBusCtx();

   DBus::Connection& GetConnection() { return *m_pCx; }
   std::string GetEndpoint() { return m_sEndpoint; }
   DBus::BusDispatcher& GetDispatcher() { return m_dispatcher; }
private:
   static std::atomic<int> s_objCount;

   std::string m_sEndpoint;
   DBus::BusDispatcher m_dispatcher;
   std::unique_ptr<DBus::Connection> m_pCx;
   std::unique_ptr<internal::DispatcherSetter> m_pDispatcherSetter;
};

} // namespace tbt
