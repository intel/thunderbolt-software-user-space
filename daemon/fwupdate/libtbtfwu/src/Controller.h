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

#include "DBusControllerProxy.h"
#include "IFWUpdateCallback.h"
#include <vector>
#include <cstdint>
#include <mutex>
#include <condition_variable>

namespace tbt
{

class DBusCtx;

/**
 * An instance of this class wraps a single D-Bus controller client object,
 * which in turn are proxies to controller objects exposed by the thunderbolt
 * daemon.  Those D-Bus daemon objects in turn each represent a physical
 * controller board for a time period in which the controller is powered up.
 *
 * That is, an instance of this class is a proxy to an online physical
 * controller board.
 *
 *              libtbt                            thunderboltd
 * .--------------------------------------.       .----------.    .----------.
 * |                                      | D-Bus |daemon    |    |physical  |
 * |Controller   -->   DBusControllerProxy|------>|controller| -->|controller|
 * |                                      |       |object    |    |          |
 * .--------------------------------------.       .----------.    .----------.
 */
class Controller : public IFWUpdateCallback
{
public:
   /**
    * Create the controller object.
    *
    * @param[in] pCtx
    *   The DBus context to use to proxy commands to the controller.  The
    *   context essentially represents the address of the daemon for D-Bus
    *   communications.
    *
    * @param[in] sDBusID
    *   The D-Bus identifier of the relevant controller.  This is a D-Bus
    *   path such as "/com/Intel/Thunderbolt1/controller/1".  Such values
    *   come from the D-Bus API for enumerating controllers.
    */
   Controller(DBusCtx* pCtx, std::string sDBusID);
   virtual ~Controller() {}

   /**
    * @return true if and only if this controller is in safe mode.  This call
    * goes to the network.  Throws if there is any failure determining
    * whether the controller is in safe mode.
    */
   bool IsInSafeMode();

   /**
    * @return the model ID of this controller.
    *
    * @throws std::exception on any failure to read the value.
    */
   uint16_t GetModelID();

   /**
    * @return the vendor ID of this controller.
    *
    * @throws std::exception on any failure to read the value.
    */
   uint16_t GetVendorID();

   /**
    * Return the D-Bus object path of this controller.  This is simply the
    * same string passed in to the constructor.
    */
   const std::string& GetDBusPath() const;

   /// <summary>
   /// Validates FW image file compatibility with the current controller
   /// </summary>
   /// <param name="image">FW image file to check</param>
   /// <remarks>
   /// When controller is in safe-mode, it's impossible to read its FW to gather needed info
   /// for comparison, so the function is no-op! User has to check if the controller is in
   /// safe-mode to know if validation had any effect.
   /// </remarks>
   /// <exception>Throws exception in case of incompatibility error or other errors</exception>
   void ValidateImage(const std::vector<uint8_t>& image);

   /**
    * Read a block of bytes from the firmware for this controller.
    *
    * @param[in] offset
    *   Firmware offset at which to start reading.
    *
    * @param[in] length
    *   Number of bytes to read.
    *
    * @return The requested array of bytes.
    *
    * @throws std::exception on any failure to read the given blocks.
    */
   std::vector<uint8_t> ReadFirmware(uint32_t offset, uint32_t length);

   /**
    * Update this controller's firmware to the image contained in 'image'.
    *
    * @param[in] image
    *   The FW image to which ot update the controller.
    *
    * @throws std::exception on any failure to update the firmware.
    */
   uint32_t UpdateFirmware(const std::vector<uint8_t>& image);

   /**
    * Fetch the major and minor NVM versions currently running on this
    * controller.
    *
    * @param[out] major
    *   The major NVM version.
    *
    * @param[out] minor
    *   The minor NVM version.
    *
    * @throws std::exception on any failure to determine the NVM version.
    */
   void GetCurrentNVMVersion(uint32_t& major, uint32_t& minor);

   /**
    * Fetch the controller's ID.  The exact semantics of the ID are TBD.
    *
    * @return the controller's ID.
    *
    * @throws std::exception on any failure teo determine the ID.
    */
   std::string GetID();

   /**
    * Called as a callback by our owned DBusControllerProxy when a firmware
    * upgrade has completed (success or fail).
    */
   virtual void FWUpdateDone(uint32_t rc, uint32_t txnid, const std::string& errstr) noexcept override;

private:
   /**
    * Helper for GetModelID and GetVendorID.  Read a uint16 from the DROM.
    *
    * @param[in] offset
    *   The offset within the DROM to read a uint16.
    */
   uint16_t ReadDROM(uint32_t offset);

   /**
    * Helper for ReadDROM.
    *
    * @param[in] offset
    *   The NVM offset at which to read a uint16.
    */
   uint16_t ReadUShort(uint32_t offset);

   DBusControllerProxy m_DBusProxy;
   DBus::BusDispatcher& m_rDispatcher;
   std::mutex m_fwu_mutex;
   std::condition_variable m_fwu_signal;
   uint32_t m_fwu_rc;
   uint32_t m_fwu_txnid;
   std::string m_fwu_errstr;
   bool m_fwu_done;

   /** Our ID.  This is cached the first time it is fetched from the daemon.  */
   std::string m_id;
};

} // namespace tbt
