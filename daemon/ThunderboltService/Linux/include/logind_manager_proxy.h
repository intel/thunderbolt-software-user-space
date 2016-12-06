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

#ifndef LOGINDPROXY_H_
#define LOGINDPROXY_H_
#include <functional>
#include "tbtdbus.h"

/**
 * this is dbus proxy for logind, used to catch the reboot and the sleep events
 */
class LogindManagerProxy : public DBus::InterfaceProxy, public DBus::ObjectProxy
{
public:
   LogindManagerProxy(DBus::Connection& connection, std::function<void()> shutdown_cb, std::function<void()> sleep_cb)
      : DBus::InterfaceProxy("org.freedesktop.login1.Manager"),
        DBus::ObjectProxy(connection, "/org/freedesktop/login1", "org.freedesktop.login1"),
        _shutdown_cb(shutdown_cb),
        _sleep_cb(sleep_cb)
   {
      connect_signal(LogindManagerProxy, PrepareForShutdown, PrepareForShutdownCb);
      connect_signal(LogindManagerProxy, PrepareForSleep, PrepareForSleepCb);
   };

private:
   // registering shutdown callback, will be called when is system is about
   // to reboot/shutdown
   void PrepareForShutdownCb(const DBus::SignalMessage&)
   {
      if (_shutdown_cb)
         _shutdown_cb();
   }
   // registering shutdown callback, will be called when is system is about
   // to enter sleep mode
   void PrepareForSleepCb(const DBus::SignalMessage&)
   {
      if (_sleep_cb)
         _sleep_cb();
   }

private:
   // shutdown callback that will be executed when system is going down
   std::function<void()> _shutdown_cb;
   // sleep callback that will be executed when system is entering sx
   std::function<void()> _sleep_cb;
};
#endif
