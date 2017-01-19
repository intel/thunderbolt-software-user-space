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

/**
 * @file tbt.cpp
 */

#include <stddef.h>
#include <tbt/tbt_fwu_err.h>
#include <stdlib.h>
#include <memory>
#include <string.h>

#include "DBusControllersProxy.h"
#include "Controller.h"
#include "tbtdbus.h"

#include "util.h"
#include "likely.h"
#include "log.h"

#include "tbt_controller_impl.h"
#include "tbt/tbt_fwu_controller.h"
#include "tbt_strerror.h"

#include "DBusCtx.h"
#include "Exceptions.h"

#include <functional>
#include "ImageUtilities.h"

extern "C" const char* tbt_fwu_Controller_getDBusID(struct tbt_fwu_Controller*);

namespace
{

tbt::DBusCtx* g_pCtx = nullptr;

std::string g_errDetail;

const static uint32_t TBT_CONTROLLER_LIST_MAGIC = 0xEA41E11A;

int wrap(std::string what, std::function<int(void)> f)
{
   try
   {
      int rc = f();
      if (rc != TBT_OK)
      {
         g_errDetail = std::string("Error while ") + what + ": " + tbt_strerror(rc);
      }
      return rc;
   }
   catch (tbt::TbtException& e)
   {
      g_errDetail = std::string("Error while ") + what + ": " + e.what();
      TBT_LOG(LOG_ERR, "%s", g_errDetail.c_str());
      return e.ErrorCode();
   }
   catch (DBus::Error& e)
   {
      g_errDetail = std::string("Daemon communication failure while ") + what + ": " + e.what();
      TBT_LOG(LOG_ERR, "%s", g_errDetail.c_str());
      return TBT_SDK_SERVICE_COMMUNICATION_FAILURE;
   }
   catch (std::bad_alloc&)
   {
      g_errDetail = std::string("out of memory while ") + what;
      TBT_LOG(LOG_ERR, "%s", g_errDetail.c_str());
      return TBT_SDK_OUT_OF_MEMORY;
   }
   catch (std::exception& e)
   {
      g_errDetail = std::string("general error while ") + what + ": " + e.what();
      TBT_LOG(LOG_ERR, "%s", g_errDetail.c_str());
      return TBT_SDK_GENERAL_ERROR_CODE;
   }
   catch (...)
   {
      g_errDetail = std::string("unknown error while ") + what;
      TBT_LOG(LOG_ERR, "%s", g_errDetail.c_str());
      return TBT_SDK_GENERAL_ERROR_CODE;
   }
}
}

extern "C" {

const char* tbt_lastErrorDetail()
{
   return g_errDetail.c_str();
}

void tbt_fwu_shutdown()
{
   delete g_pCtx;
   g_pCtx = nullptr;
}

/**
 * Initialize this library.  This function should be called once
 * at application start time.
 *
 * This is an alternative function to tbt_fwu_init() above.  It is used in
 * the case where the daemon may running on the session bus rather than
 * the system bus, or may be available via a D-Bus name other than the
 * well-known "com.Intel.Thunderbolt1".
 *
 * A call to tbt_fwu_init() is equivalent to a call to
 * tbt_fwu_init2(1, "com.Intel.Thunderbolt1").
 *
 * @param[in] bSystemBus
 *   True if and only if we should attempt to connect to the daemon via
 *   the D-Bus system bus.  If false, then we attempt via the D-Bus session
 *   bus.
 *
 * @param[in] zEndpoint
 *   The name of the daemon at the given bus.  This can be a well-known
 *   name such as "com.Intel.Thunderbolt1", or a D-Bus unique name such as
 *   ":1.2341".
 *
 * @return
 *   TBT_OK (0) on success, or an error status from tbt_fwu_err.h
 *   on failure.  On failure, the caller may reattempt the tbt_fwu_init, but
 *   may not call any other API functions.
 */
int tbt_fwu_init2(int bSystemBus, const char* zEndpoint)
{
   int rc;
   rc = tbt::tbt_init_strerror();
   if (UNLIKELY(rc != TBT_OK))
   {
      return rc;
   }
   if (!zEndpoint)
   {
      zEndpoint = "com.Intel.Thunderbolt1";
   }
   if (g_pCtx)
   {
      tbt_fwu_shutdown();
   }
   return wrap("initializing TBT library", [zEndpoint, bSystemBus]() {
      g_pCtx = new tbt::DBusCtx(zEndpoint, bSystemBus);
      return TBT_OK;
   });
}

int tbt_fwu_init(void)
{
   return tbt_fwu_init2(1, NULL);
}

int tbt_fwu_Controller_getNVMVersion(struct tbt_fwu_Controller* pController, uint32_t* pMajor, uint32_t* pMinor)
{
   if (UNLIKELY(!tbtController_valid(pController)))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!pMajor))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!pMinor))
   {
      return TBT_SDK_MISUSE;
   }
   return wrap("getting NVM version", [pMajor, pMinor, pController]() {
      if (pController->m_pController->IsInSafeMode())
      {
         TBT_LOG(LOG_ERR,
                 "Cannot get NVM version: controller %s "
                 "is in safe mode.",
                 tbt_fwu_Controller_getDBusID(pController));
         return TBT_SDK_INVALID_OPERATION_IN_SAFE_MODE;
      }
      pController->m_pController->GetCurrentNVMVersion(*pMajor, *pMinor);
      return TBT_OK;
   });
}

extern int tbt_fwu_Controller_getPCIAddress(struct tbt_fwu_Controller* pController, uint16_t* address)
{
   const int addressSize = 4;
   return wrap("getting model ID", [address, pController]() {
      auto id  = pController->m_pController->GetID();
      *address = stoul(id.substr(0, addressSize), 0, 16);
      return TBT_OK;
   });
}

int tbt_fwu_Controller_getModelID(struct tbt_fwu_Controller* pController, uint16_t* pID)
{
   if (UNLIKELY(!tbtController_valid(pController)))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!pID))
   {
      return TBT_SDK_MISUSE;
   }
   return wrap("getting model ID", [pID, pController]() {
      if (pController->m_pController->IsInSafeMode())
      {
         TBT_LOG(
            LOG_ERR, "Cannot get model ID: controller %s is in safe mode.", tbt_fwu_Controller_getDBusID(pController));
         return TBT_SDK_INVALID_OPERATION_IN_SAFE_MODE;
      }
      *pID = pController->m_pController->GetModelID();
      return TBT_OK;
   });
}

int tbt_fwu_Controller_getVendorID(struct tbt_fwu_Controller* pController, uint16_t* pID)
{
   if (UNLIKELY(!tbtController_valid(pController)))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!pID))
   {
      return TBT_SDK_MISUSE;
   }
   return wrap("getting vendor ID", [pID, pController]() {
      if (pController->m_pController->IsInSafeMode())
      {
         TBT_LOG(
            LOG_ERR, "Cannot get vendor ID: controller %s is in safe mode.", tbt_fwu_Controller_getDBusID(pController));
         return TBT_SDK_INVALID_OPERATION_IN_SAFE_MODE;
      }
      *pID = pController->m_pController->GetVendorID();
      return TBT_OK;
   });
}

int tbt_fwu_getImageNVMVersion(const uint8_t* pBuffer, size_t nBuffer, uint32_t* pMajor, uint32_t* pMinor)
{
   if (UNLIKELY(!pMajor))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!pMinor))
   {
      return TBT_SDK_MISUSE;
   }
   return wrap("getting NVM version from image", [pBuffer, nBuffer, pMajor, pMinor]() {
      std::vector<uint8_t> image;
      image.assign(pBuffer, pBuffer + nBuffer);
      (void)tbt::GetImageNVMVersion(image, *pMajor, *pMinor);
      return TBT_OK;
   });
}

int tbt_fwu_Controller_getID(struct tbt_fwu_Controller* pController, char* zBuf, size_t* pnBuf)
{
   if (UNLIKELY(!tbtController_valid(pController)))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!zBuf))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!pnBuf))
   {
      return TBT_SDK_MISUSE;
   }
   return wrap("getting controller ID", [pController, zBuf, pnBuf]() {
      std::string id = pController->m_pController->GetID();
      *pnBuf         = snprintf(zBuf, *pnBuf, "%s", id.c_str()) + 1;
      return TBT_OK;
   });
}

void tbt_fwu_freeControllerList(struct tbt_fwu_Controller** apControllers, int);

int tbt_fwu_getControllerList(struct tbt_fwu_Controller*** papControllers, size_t* pnControllers)
{
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   return wrap("getting controller list", [papControllers, pnControllers]() {
      struct tbt_fwu_Controller** apControllers = NULL;
      tbt::DBusControllersProxy proxy(
         g_pCtx->GetConnection(), "/com/Intel/Thunderbolt1/controllers", g_pCtx->GetEndpoint().c_str());
      std::vector<std::string> lCtrls = proxy.GetControllerList();
      /* We allocate two extra pointers.  The first extra pointer is at the
       * head of the array and contains a magic value used for error checking.
       * (This is not visible to the library user.)  The second extra pointer
       * is at the end of the array and is a null terminator.  This is an
       * unpublished API feature that we may choose to publish at a later time.
       */
      apControllers = (struct tbt_fwu_Controller**)calloc(lCtrls.size() + 2, sizeof(struct tbt_fwu_Controller*));
      if (!apControllers)
      {
         return TBT_SDK_OUT_OF_MEMORY;
      }
      apControllers[0] = (struct tbt_fwu_Controller*)TBT_CONTROLLER_LIST_MAGIC;
      ++apControllers;
      size_t i = 0;
      for (auto ctrlID : lCtrls)
      {
         try
         {
            apControllers[i] = tbtController_new(g_pCtx, ctrlID);
            ++i;
         }
         catch (...)
         {
            for (size_t j = 0; j < i; j++)
            {
               tbtController_delete(apControllers[j]);
            }
            --apControllers;
            free(apControllers);
            throw;
         }
      }
      *papControllers = apControllers;
      *pnControllers  = lCtrls.size();
      return TBT_OK;
   });
}

static bool tbt_fwu_ControllerListValid(struct tbt_fwu_Controller** apControllers)
{
   return apControllers[0] == (struct tbt_fwu_Controller*)TBT_CONTROLLER_LIST_MAGIC;
}

void tbt_fwu_freeControllerList(struct tbt_fwu_Controller** apControllers, int nController __attribute__((unused)))
{
   if (UNLIKELY(!apControllers))
   {
      return;
   }
   --apControllers;
   if (!tbt_fwu_ControllerListValid(apControllers))
   {
      TBT_LOG(LOG_ERR,
              "Application attempted to free invalid list of tbt_fwu_Controller "
              "%p: ignoring\n",
              apControllers);
      return;
   }
   for (struct tbt_fwu_Controller** p = apControllers + 1; *p; ++p)
   {
      if (UNLIKELY(!tbtController_valid(*p)))
      {
         TBT_LOG(LOG_ERR,
                 "Application attempted to free invalid tbt_fwu_Controller "
                 "pointer %p: ignoring\n",
                 *p);
         continue;
      }
      tbtController_delete(*p);
      tbt::CLEAR_PTR(*p);
   }
   free(apControllers);
}

int tbt_fwu_Controller_isInSafeMode(struct tbt_fwu_Controller* pController, int* bSafeMode)
{
   if (UNLIKELY(!tbtController_valid(pController)))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!bSafeMode))
   {
      return TBT_SDK_MISUSE;
   }
   return wrap("getting safe mode status", [pController, bSafeMode] {
      *bSafeMode = pController->m_pController->IsInSafeMode();
      return TBT_OK;
   });
}

int tbt_fwu_Controller_validateFWImage(struct tbt_fwu_Controller* pController, const uint8_t* pBuffer, size_t nBuffer)
{
   int rc;
   int bSafe;
   if (UNLIKELY(!tbtController_valid(pController)))
   {
      return TBT_SDK_MISUSE;
   }
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   rc = tbt_fwu_Controller_isInSafeMode(pController, &bSafe);
   if (rc != TBT_OK)
   {
      return rc;
   }
   if (bSafe)
   {
      TBT_LOG(LOG_ERR,
              "Cannot validate image file: controller %s "
              "is in safe mode.",
              tbt_fwu_Controller_getDBusID(pController));
      return TBT_SDK_INVALID_OPERATION_IN_SAFE_MODE;
   }
   return wrap("validating image", [pController, pBuffer, nBuffer]() {
      std::vector<uint8_t> image;
      image.assign(pBuffer, pBuffer + nBuffer);
      pController->m_pController->ValidateImage(image);
      return TBT_OK;
   });
}

int tbt_fwu_Controller_updateFW(struct tbt_fwu_Controller* pController,
                                const uint8_t* pBuffer,
                                size_t nBuffer,
                                void (*progress_cb)(uint32_t percentage, void* user_data),
                                void* user_data)

{
   if (UNLIKELY(!tbtController_valid(pController)))
   {
      return TBT_SDK_MISUSE;
   }
   int bSafe = 0; // Default false -- i.e. proceed if we cannot determine mode.
   if (UNLIKELY(!g_pCtx))
   {
      return TBT_SDK_MISUSE;
   }
   int rc = tbt_fwu_Controller_isInSafeMode(pController, &bSafe);
   if (rc != TBT_OK)
   {
      TBT_LOG(LOG_WARNING,
              "Cannot determine whether controller %s "
              "is in safe mode.  Attempting to proceed.",
              tbt_fwu_Controller_getDBusID(pController));
   }
   if (bSafe)
   {
      TBT_LOG(LOG_WARNING,
              "Cannot validate image file: controller %s "
              "is in safe mode.",
              tbt_fwu_Controller_getDBusID(pController));
   }
   return wrap("updating firmware", [pController, pBuffer, nBuffer, progress_cb, user_data]() {
      std::vector<uint8_t> vec;
      vec.assign(pBuffer, pBuffer + nBuffer);
      return pController->m_pController->UpdateFirmware(vec, progress_cb, user_data);
   });
}

void tbt_fwu_setLogLevel(int level)
{
   tbt::SetLogLevel(level);
}

} // extern "C"
