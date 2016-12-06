/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
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

#include "logger.h"

#pragma GCC diagnostic push

#if DBUS_CXX_INTROSPECT_HAS_SPURIOUS_CONST
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

#include <dbus-c++/introspection.h>
#include <dbus-c++/interface.h>

#pragma GCC diagnostic pop

#include "DBusController.h"
#include "Utils.h"
#include "ControllerDetails.h"

#include "../config.h"

#include <atomic>
#include <string>
#include <locale>

namespace
{

std::mutex g_fwu_mutex;
bool g_is_fwu = false;
bool InFWU()
{
   std::unique_lock<std::mutex> l(g_fwu_mutex);
   return g_is_fwu;
}

void in_use(uint32_t& rc, std::string& errstr)
{
   rc     = uint32_t(TBTDRV_STATUS::SDK_IN_USE);
   errstr = "The daemon is in use.  Please try again later.";
}

uint32_t wrap(std::string what, std::string& errstr, std::function<int(void)> f)
{
   try
   {
      uint32_t rc = f();
      return rc;
   }
   catch (TbtException& e)
   {
      std::string msg = std::string("Error while ") + what + ": " + e.what();
      errstr          = msg;
      TbtServiceLogger::LogError("%s", msg.c_str());
      return uint32_t(TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE);
   }
   catch (std::exception& e)
   {
      std::string msg = std::string("general error while ") + what + ": " + e.what();
      errstr          = msg;
      TbtServiceLogger::LogError("%s", msg.c_str());
      return uint32_t(TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE);
   }
   catch (...)
   {
      std::string msg = std::string("unknown error while ") + what;
      errstr          = msg;
      TbtServiceLogger::LogError("%s", msg.c_str());
      return uint32_t(TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE);
   }
}

} // end anonymous namespace

DBusController::DBusController(DBus::Connection& cx, IController& rCtrl)
   : DBus::ObjectAdaptor(cx, DBusController::AllocateObjectPath()), m_rController(rCtrl), m_txnidCounter(1)
{
}

void DBusController::GetControllerID(uint32_t& rc, std::string& errstr, std::string& id)
{
   if (InFWU())
   {
      in_use(rc, errstr);
      return;
   }
   rc = wrap("fetching controller ID", errstr, [this, &id]() {
      id = WStringToString(m_rController.GetControllerID());
      return uint32_t(TBTDRV_STATUS::SUCCESS_RESPONSE_CODE);
   });
}
void DBusController::IsInSafeMode(uint32_t& rc, std::string& errstr, bool& b)
{
   if (InFWU())
   {
      in_use(rc, errstr);
      b = false;
      return;
   }
   rc = wrap("fetching safe mode status", errstr, [this, &b]() {
      b = m_rController.GetControllerData()->GetIsInSafeMode();
      return uint32_t(TBTDRV_STATUS::SUCCESS_RESPONSE_CODE);
   });
}
void DBusController::UpdateFirmware(const std::vector<uint8_t>& buffer,
                                    uint32_t& rc,
                                    uint32_t& txnid,
                                    std::string& errstr)
{
   txnid = 0; // In case of error.
   {
      std::unique_lock<std::mutex> l(g_fwu_mutex);
      if (g_is_fwu)
      {
         in_use(rc, errstr);
         return;
      }
      g_is_fwu = true;
   }
   txnid = m_txnidCounter++;
   std::thread([this, buffer /* copy! */, txnid]() {
      std::string errstr;
      uint32_t rc =
         wrap("updating firmware", errstr, [this, buffer]() { return m_rController.ControllerFwUpdate(buffer); });
      wrap("sending firmware update response", errstr, [this, rc, txnid, errstr]() {
         UpdateFirmwareResponse(rc, txnid, errstr);
         return 0;
      });
      {
         std::unique_lock<std::mutex> l(g_fwu_mutex);
         g_is_fwu = false;
      }
   }).detach();
   rc = uint32_t(TBTDRV_STATUS::SUCCESS_RESPONSE_CODE);
}
void DBusController::GetCurrentNVMVersion(uint32_t& rc, std::string& errstr, uint32_t& major, uint32_t& minor)
{
   major = minor = 0; // In case of error.
   if (InFWU())
   {
      in_use(rc, errstr);
      return;
   }
   rc = wrap("fetching NVM version", errstr, [this, &major, &minor]() {
      if (m_rController.GetControllerData()->GetGeneration() < ThunderboltGeneration::THUNDERBOLT_3)
      {
         return uint32_t(TBTDRV_STATUS::CONTROLLER_NOT_SUPPORTED);
      }
      Version nvmVersion;
      uint32_t rc = m_rController.GetNVMVersion(nvmVersion);
      if (rc != 0)
      {
         return rc;
      }
      if (!nvmVersion.major() || !nvmVersion.minor())
      {
         return uint32_t(TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE);
      }
      major = *nvmVersion.major();
      minor = *nvmVersion.minor();
      return uint32_t(TBTDRV_STATUS::SUCCESS_RESPONSE_CODE);
   });
}

void DBusController::ReadFirmware(const uint32_t& offset,
                                  const uint32_t& size,
                                  uint32_t& rc,
                                  std::string& errstr,
                                  std::vector<uint8_t>& data)
{
   if (InFWU())
   {
      in_use(rc, errstr);
      return;
   }
   rc = wrap("reading firmware", errstr, [this, offset, size, &errstr, &data]() {
      uint32_t rc;
      // For now we disallow reads greater than 64 bytes.
      if (size > 64)
      {
         errstr = "Reads greater than 64 bytes are disallowed.";
         return uint32_t(TBTDRV_STATUS::INVALID_ARGUMENTS);
      }
      std::vector<uint8_t> out;
      // The firmware wrapping code at the moment only can deal with one
      // 4-byte word at a time.  In addition, the word it returns is 4-byte
      // aligned.  We translate this behavior into a nicer behavior according
      // to what the caller expects (which is that the offset need not be
      // aligned, nor the size restricted).
      size_t nWord = (offset % 4 + size + 3) / 4;
      out.reserve(size + 8);
      size_t alignedOffset = offset - offset % 4;
      for (size_t i = 0; i < nWord; ++i)
      {
         std::vector<uint8_t> word;
         rc = uint32_t(m_rController.ReadFirmware(alignedOffset + 4 * i, 4 /* length in bytes (ignored) */, word));
         if (rc != 0)
         {
            return rc;
         }
         if (word.size() != 4)
         {
            // Very odd internal bug.
            return uint32_t(TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE);
         }
         out.insert(out.end(), word.begin(), word.end());
      }
      data = std::vector<uint8_t>(out.begin() + offset % 4, out.begin() + offset % 4 + size);
      return uint32_t(TBTDRV_STATUS::SUCCESS_RESPONSE_CODE);
   });
}

std::atomic<uint32_t> DBusController::s_nextDBusID(1);

std::string DBusController::AllocateObjectPath()
{
   uint32_t nextDBusPath = s_nextDBusID++;
   return std::string(THUNDERBOLT_SERVER_PATH "/" THUNDERBOLT_CONTROLLER_DBUS_NAME "/") + std::to_string(nextDBusPath);
}
