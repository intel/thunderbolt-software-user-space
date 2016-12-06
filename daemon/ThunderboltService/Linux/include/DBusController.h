/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2014-2016 Intel Corporation.
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "dbus/dbus_controller_adapter.h"
#pragma GCC diagnostic pop

#include "IController.h"

#include <string>
#include <atomic>

/**
 * This class contains the main program logic for servicing commands
 * received via D-Bus from some external client, bound for a particular
 * TBT controller.  Our primary job is to marshall the commands down to
 * that controller and respond to the D-Bus client with the results.
 */
class DBusController : public com::Intel::Thunderbolt1::controller_adaptor,
                       public DBus::IntrospectableAdaptor,
                       public DBus::ObjectAdaptor
{
public:
   DBusController(DBus::Connection& cx, IController& rCtrl);

   /**
    * Retrieve this controller's ID and place it into 'id'.
    *
    * As with the rest of this API, errors are reported by placing a status
    * code into 'rc' (which is one of the values from TBTDRV_STATUS) and a
    * detail string into 'errstr'.
    *
    * @param[out] rc
    *   The return code; one of the TBTDRV_STATUS values from Sdk.h.
    *
    * @param[out] errstr
    *   The error detail string (if an error occurred).
    *
    * @param[out] id
    *   On success, the ID of this controller.
    */
   virtual void GetControllerID(uint32_t& rc, std::string& errstr, std::string& id);

   /**
    * Determine whether this controller is in safe mode, and put the value
    * into 'b'.
    *
    * As with the rest of this API, errors are reported by placing a status
    * code into 'rc' (which is one of the values from TBTDRV_STATUS) and a
    * detail string into 'errstr'.
    *
    * @param[out] rc
    *   The return code; one of the TBTDRV_STATUS values from Sdk.h.
    *
    * @param[out] errstr
    *   The error detail string (if an error occurred).
    *
    * @param[out] b
    *   On success, true if and only if this controller is in safe mode.
    */
   virtual void IsInSafeMode(uint32_t& rc, std::string& errstr, bool& b);

   /**
    * Begin updating the firmware image on this controller to 'buffer'.  On
    * success, this function fills in 'txnid' with a unique transaction ID,
    * and returns immediately after launching the update process.  At some
    * future point when the update is done (either after a success or after
    * a failure), the UpdateFirmwareResponse signal will be emitted with the
    * same txnid as was returned by this method.
    *
    * As with the rest of this API, errors are reported by placing a status
    * code into 'rc' (which is one of the values from TBTDRV_STATUS) and a
    * detail string into 'errstr'.
    *
    * @param[in] buffer
    *   The new firmware image to which to upgrade this controller.
    *
    * @param[out] rc
    *   The return code; one of the TBTDRV_STATUS values from Sdk.h.
    *
    * @param[out] errstr
    *   The error detail string (if an error occurred).
    *
    * @param[out] txnid
    *   On success, the transaction ID for this firmware update.  This
    *   same transaction ID will be included in a future UpdateFirmwareResponse
    *   signal.
    */
   virtual void UpdateFirmware(const std::vector<uint8_t>& buffer, uint32_t& rc, uint32_t& txnid, std::string& errstr);

   /**
    * Retrieve the current NVM version running on this controller and place
    * it into 'major' and 'minor'.
    *
    * As with the rest of this API, errors are reported by placing a status
    * code into 'rc' (which is one of the values from TBTDRV_STATUS) and a
    * detail string into 'errstr'.
    *
    * @param[out] rc
    *   The return code; one of the TBTDRV_STATUS values from Sdk.h.
    *
    * @param[out] errstr
    *   The error detail string (if an error occurred).
    *
    * @param[out] major
    *   On success, the major NVM version currently running on the controller.
    *
    * @param[out] minor
    *   On success, the minor NVM version currently running on the controller.
    */
   virtual void GetCurrentNVMVersion(uint32_t& rc, std::string& errstr, uint32_t& major, uint32_t& minor);

   /**
    * Read a contiguous range of bytes from the controller NVM.
    *
    * As with the rest of this API, errors are reported by placing a status
    * code into 'rc' (which is one of the values from TBTDRV_STATUS) and a
    * detail string into 'errstr'.
    *
    * @param[in] offset
    *   The offset at which to start the read.
    *
    * @param[in] size
    *   The number of bytes to read.
    *
    * @param[out] rc
    *   The return code; one of the TBTDRV_STATUS values from Sdk.h.
    *
    * @param[out] errstr
    *   The error detail string (if an error occurred).
    *
    * @param[out] data
    *   On success, the byte range the caller requested.
    */
   virtual void ReadFirmware(const uint32_t& offset,
                             const uint32_t& size,
                             uint32_t& rc,
                             std::string& errstr,
                             std::vector<uint8_t>& data);

private:
   /**
    * The next value to be used as the final component of the DBus path
    * to a DBusController object.  This is used to
    * allocate the DBus object paths for instances of this class.
    * The full path name will be of the form
    *
    * THUNDERBOLT_SERVER_PATH/THUNDERBOLT_CONTROLLER_DBUS_NAME/<<s_nextDBusID>>
    *
    * i.e.
    *
    * /com/Intel/Thunderbolt1/controller/<<s_nextDBusID>>
    */
   static std::atomic<uint32_t> s_nextDBusID;

   /**
    * Return the next available object path.
    */
   static std::string AllocateObjectPath();

   /**
    * Reference to the wrapped controller.
    */
   IController& m_rController;

   /**
    * Running transaction ID counter.
    */
   uint32_t m_txnidCounter;
};
