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

#pragma once

#include "tbtdbus.h"

#include "dbus_controller_proxy.h"
#include "IFWUpdateCallback.h"

namespace tbt
{

/**
 * This class is a very thin wrapper around a few native D-Bus classes,
 * including one class generated from the API description.  This proxy
 * object exposes two types of function:
 *
 * 1. Functions representing D-Bus methods.  Calls to these functions are
 *    blocking calls that invoke remote calls in the thunderbolt daemon to
 *    perform some action on a controller.
 *
 * 2. Functions representing D-Bus "signals".  These are asynchronous events
 *    that originate from the daemon.  The only one of these today is an event
 *    that indicates that a firmware update is complete.  (We do not use the
 *    normal blocking mechanism for this because the duration of an update
 *    is larger than the default timeout for the libdbus-c++ library.  This
 *    timeout cannot be altered in the 0.5.0 version of the library, which
 *    we are forced to support for RHEL 7.2.)
 */
class DBusControllerProxy : public com::Intel::Thunderbolt1::controller_proxy,
                            public DBus::IntrospectableProxy,
                            public DBus::ObjectProxy
{
public:
   /**
    * Instantiate a proxy to the daemon controller object exposed at 'path'
    * in the daemon at the other end of connection 'connection'.
    *
    * @param[in] connection
    *   The location of the daemon with which to exchange D-Bus messages
    *   regarding this controller.
    *
    * @param[in] path
    *   The D-Bus object path of the relevant controller.  E.g.
    *   "/com/Intel/Thunderbolt1/controller/1".
    *
    * @param[in] cb
    *   A callback to call on receipt of an "update firmware response" event.
    *   The caller must guarantee that 'cb' lives at least as long as this
    *   DBusControllerProxy object.
    */
   DBusControllerProxy(DBus::Connection& connection, const char* path, const char* name, IFWUpdateCallback& cb);

   /**
    * This function is called by libdbus-c++ on arrival of an "update firmware
    * response" event.  This event is fired by the daemon when a firmware
    * update is done (either with a success or failure status).
    *
    * The implementation is to simply pass through the arguments to the callback
    * object that was registered at construction time ('cb' from the
    * constructor above).
    *
    * @param[in] rc
    *   The status code of the firmware update overall.  This is one of the
    *   codes listed in tbt_fwu_err.h.
    *
    * @param[in] txnid
    *   The transaction ID of the firmware update operation.  This is the
    *   same transaction ID as previously returned by the UpdateFirmware
    *   method call that initiated the firmware update operation.
    *
    * @param[in] errstr
    *   The error detail string, if relevant.  This is a human-readable
    *   elaboration on the return-code, and might include more detail than
    *   is available in the return code.
    */
   virtual void UpdateFirmwareResponse(const uint32_t& rc, const uint32_t& txnid, const std::string& errstr) override;

private:
   IFWUpdateCallback& m_rFWUCB;
};
}
