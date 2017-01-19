/********************************************************************************
 * Thunderbolt(TM) FW update library
 * This library is distributed under the following BSD-style license:
 *
 * Copyright(c) 2016 - 2017 Intel Corporation.
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

#include "Controller.h"
#include "DBusCtx.h"
#include "ControllerFwInfoSource.h"
#include "FileFwInfoSource.h"
#include "HostFwInfo.h"
#include "tbt/tbt_fwu_err.h"
#include "Exceptions.h"
#include "HostImageValidator.h"
#include "log.h"
#include "util.h"
#include <unistd.h>
#include <thread>
#include <string>
#include <atomic>

namespace tbt
{

Controller::Controller(DBusCtx* pCtx, std::string sDBusID)
   : m_DBusProxy(pCtx->GetConnection(), sDBusID.c_str(), pCtx->GetEndpoint().c_str(), *this),
     m_rDispatcher(pCtx->GetDispatcher()),
     m_fwu_mutex(),
     m_fwu_signal(),
     m_fwu_rc(-1),
     m_fwu_txnid(-1),
     m_fwu_errstr(),
     m_fwu_done(false)
{
}

bool Controller::IsInSafeMode()
{
   // This call, like most of the calls in this class, works by firing
   // a D-Bus network request, blocking for the response, and reporting
   // the response data back to the caller.
   uint32_t rc;
   std::string errstr;
   bool b;
   m_DBusProxy.IsInSafeMode(rc, errstr, b);
   if (rc != TBT_OK)
   {
      TBT_THROW(rc, errstr);
   }
   return b;
}

std::string Controller::GetID()
{
   if (!m_id.empty())
   {
      return m_id;
   }
   uint32_t rc;
   std::string errstr;
   std::string id;
   m_DBusProxy.GetControllerID(rc, errstr, id);
   if (rc != TBT_OK)
   {
      TBT_THROW(rc, errstr);
   }
   m_id = id;
   return id;
}

void Controller::GetCurrentNVMVersion(uint32_t& major, uint32_t& minor)
{
   uint32_t rc;
   std::string errstr;
   m_DBusProxy.GetCurrentNVMVersion(rc, errstr, major, minor);
   if (rc != TBT_OK)
   {
      TBT_THROW(rc, errstr);
   }
}

uint16_t Controller::GetModelID()
{
   return ReadDROM(0x12);
}

uint16_t Controller::GetVendorID()
{
   return ReadDROM(0x10);
}

uint16_t Controller::ReadDROM(uint32_t offset)
{
   fwu::ControllerFwInfoSource src(*this);
   fwu::HostFwInfo fwInfo(src);
   auto sectionInfo = fwInfo.GetSectionInfo();
   if (sectionInfo.find(fwu::DROM) == sectionInfo.end())
   {
      TBT_THROW(TBT_SDK_NO_DROM_IN_FILE_ERROR, "controller has no DROM section");
   }
   return ReadUShort(sectionInfo[fwu::DROM].Offset + offset);
}

uint16_t Controller::ReadUShort(uint32_t offset)
{
   uint32_t rc;
   std::string errstr;
   std::vector<uint8_t> data;
   m_DBusProxy.ReadFirmware(offset, 2, rc, errstr, data);
   if (rc != TBT_OK)
   {
      TBT_THROW(rc, errstr);
   }
   if (data.size() != 2)
   {
      errstr = "Expected daemon to return array of size 2, got size " + std::to_string(data.size());
      TBT_THROW(TBT_SDK_GENERAL_ERROR_CODE, errstr);
   }
   return ToUInt16(data, 0);
}

const std::string& Controller::GetDBusPath() const
{
   return m_DBusProxy.path();
}

void Controller::ValidateImage(const std::vector<uint8_t>& image)
{
   if (IsInSafeMode())
   {
      return;
   }
   fwu::ControllerFwInfoSource controllerSource(*this);
   fwu::HostFwInfo controllerFwInfo(controllerSource);
   const fwu::HwInfo& ctrlHwInfo = controllerFwInfo.GetHwInfo();
   if (ctrlHwInfo.GetGeneration() < fwu::DSL6540_6340)
   {
      TBT_THROW(TBT_SDK_CONTROLLER_NOT_SUPPORTED);
   }
   fwu::FileFwInfoSource fileSource(image);
   fwu::HostFwInfo fileFwInfo(fileSource);
   const fwu::HwInfo& fileHwInfo = fileFwInfo.GetHwInfo();
   if (ctrlHwInfo.GetGeneration() != fileHwInfo.GetGeneration())
   {
      TBT_LOG(LOG_WARNING,
              "Generation mismatch: controller %d vs file %d",
              ctrlHwInfo.GetGeneration(),
              fileHwInfo.GetGeneration());
      TBT_THROW(TBT_SDK_HW_GENERATION_MISMATCH);
   }
   const fwu::HwType* pCtrlType = ctrlHwInfo.GetHwType();
   const fwu::HwType* pFileType = fileHwInfo.GetHwType();
   if (!pCtrlType || !pFileType || *pCtrlType != *pFileType)
   {
      TBT_THROW(TBT_SDK_PORT_COUNT_MISMATCH);
   }

   auto controllerSectionInfo = controllerFwInfo.GetSectionInfo();
   auto fileSectionInfo       = fileFwInfo.GetSectionInfo();
   fwu::HostImageValidator validator(*this, image, controllerSectionInfo, fileSectionInfo, ctrlHwInfo);
   validator.Validate();
}

std::vector<uint8_t> Controller::ReadFirmware(uint32_t offset, uint32_t length)
{
   if (IsInSafeMode())
   {
      TBT_THROW(TBT_SDK_INVALID_OPERATION_IN_SAFE_MODE);
   }
   uint32_t rc;
   std::string errstr;
   std::vector<uint8_t> data;
   m_DBusProxy.ReadFirmware(offset, length, rc, errstr, data);
   if (rc != TBT_OK)
   {
      TBT_THROW(rc, errstr);
   }
   if (data.size() != length)
   {
      TBT_THROW(TBT_SDK_GENERAL_ERROR_CODE,
                "ReadFirmware: daemon reported success, but returned data of "
                "incorrect size %zu (expected %zu)",
                data.size(),
                length);
   }
   return data;
}

uint32_t Controller::UpdateFirmware(const std::vector<uint8_t>& fwimg,
                                    void (*progress_cb)(uint32_t percentage, void* user_data),
                                    void* user_data)
{
   uint32_t rc;
   uint32_t txnid;
   std::string errstr;

   // First, validate the image (if we are not in safe mode).  If the
   // validation fails, allow the exception to propagate upward so that
   // the firmware update fails overall.
   ValidateImage(fwimg);

   // Now run the firmware update.  This call has a different structure
   // than the other D-Bus calls in this layer.  The UpdateFirmware call
   // does not block to wait for the operation to complete.  Instead, the
   // daemon sends an ACK right away, and fills in a transaction ID.  Later,
   // when the update is done (either success or fail), the daemon emits a
   // signal with the same transaction ID indicating that the transaction is
   // complete.
   //
   // We hide these details by ourself exposing a blocking interface.  We
   // send the update request, wait for the response, and report the response
   // to the caller of this method.  All the other code in this method is
   // for error handling.
   m_DBusProxy.UpdateFirmware(fwimg, rc, txnid, errstr);

   if (rc != TBT_OK)
   {
      // Failed to initiate the update.
      TBT_THROW(rc, errstr);
   }

   // The update has begun.  We now block and wait for the ensuing signal
   // that the update is done.

   {
      // Set up some state variables.
      std::unique_lock<std::mutex> l(m_fwu_mutex);
      m_fwu_txnid = txnid;
      m_fwu_done  = false;
      m_fwu_rc    = -1;
      m_fwu_errstr.clear();
   }

   std::atomic_bool quitDispatch(false);

   // We are about to start blocking to wait for the signal that the update
   // is done.  However, perhaps there will be some error modes: the daemon
   // could deadlock or crash, for example.  We spin up a thread to break us
   // out of the blocking wait after a timeout.
   std::thread th([this, &quitDispatch, progress_cb, user_data]() {

      namespace cr       = std::chrono;
      const auto timeout = cr::minutes(10);

      if (!progress_cb)
      {
         std::unique_lock<std::mutex> l(m_fwu_mutex);
         // Block til either the timeout fires or the FWU is done.
         (void)m_fwu_signal.wait_for(l, timeout, [this]() { return m_fwu_done; });
      }

      else
      {
         const auto expectedUpdateTime = cr::minutes(2);
         const auto onePercent         = cr::duration_cast<cr::milliseconds>(expectedUpdateTime) / 100;

         // Encapsulating the loop in a local function to be able to break early
         // and signal it out. Returns true if it breaks because FW update done,
         // false - if returned by end of the loop.
         auto progressLoop = [&] {
            for (uint32_t percentage = 0; percentage < 99;) // so we don't reach 100 too soon
            {
               std::unique_lock<std::mutex> l(m_fwu_mutex);
               if (m_fwu_signal.wait_for(l, onePercent, [this]() { return m_fwu_done; }))
               {
                  return true;
               }
               ++percentage;
               progress_cb(percentage, user_data);
            }
            return false;
         };

         if (!progressLoop())
         {
            std::unique_lock<std::mutex> l(m_fwu_mutex);
            // Block til either the timeout fires or the FWU is done.
            (void)m_fwu_signal.wait_for(l, timeout - expectedUpdateTime, [this]() { return m_fwu_done; });
         }

         // Report 100 anyway
         progress_cb(100, user_data);
      }

      // Either way, break the main thread out of the D-Bus dispatch loop.
      quitDispatch = true;
   });

   // Block til we get a "FWUpdateDone" response for txnid, or the timeout
   // thread fires.
   while (!quitDispatch)
   {
      m_rDispatcher.do_iteration();
   }

   // Wait for the thread to shut down.  This should be pretty quick, since
   // it just broke us out of the dispatch loop (which is its last operation
   // before entering thread library clean-up code).
   th.join();

   if (!m_fwu_done)
   {
      // Timeout.
      TBT_THROW(TBT_SDK_SERVICE_FWU_TIMEOUT);
   }

   // We got a response for the FWU.  Unpack it and reply to the caller
   // as appropriate.
   {
      std::unique_lock<std::mutex> l(m_fwu_mutex);
      rc          = m_fwu_rc;
      errstr      = m_fwu_errstr;
      m_fwu_txnid = -1;
   }
   if (rc != TBT_OK)
   {
      TBT_THROW(rc, errstr);
   }
   return rc;
}

// Called in the main thread when the daemon emits a signal indicating a FWU
// operation is complete.  Remember the results, and signal the timeout thread
// that is set up in UpdateFirmware, above.  The timeout thread will then break
// us out of the D-Bus dispatch loop.
void Controller::FWUpdateDone(uint32_t rc, uint32_t txnid, const std::string& errstr) noexcept
{
   std::unique_lock<std::mutex> lock(m_fwu_mutex);
   if (txnid != m_fwu_txnid)
   {
      // Spurious response.  Ignore it.
      return;
   }
   m_fwu_rc     = rc;
   m_fwu_errstr = errstr;
   m_fwu_done   = true;
   m_fwu_signal.notify_one();
}

} // namespace tbt
