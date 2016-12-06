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
 * @file tbt_controller_impl.cpp
 */

#include "tbt_controller_impl.h"
#include "DBusCtx.h"
#include "Controller.h"
#include "likely.h"
#include "util.h"
#include "log.h"
#include <new>

const static uint32_t TBT_MAGIC = 0xE11AE11A;

bool tbtController_valid(const tbt_fwu_Controller* p)
{
   return p && p->m_magic == TBT_MAGIC;
}

extern "C" {

const char* tbt_fwu_Controller_getDBusID(struct tbt_fwu_Controller* pController)
{
   // Safe to return c_str() because the std::string is a ref all the way down
   // to the DBUs::Object std::string, which in turn is never altered.
   return pController->m_pController->GetDBusPath().c_str();
}
}

struct tbt_fwu_Controller* tbtController_new(tbt::DBusCtx* pCtx, std::string sControllerID)
{
   tbt_fwu_Controller* p = (tbt_fwu_Controller*)calloc(1, sizeof(tbt_fwu_Controller));
   if (UNLIKELY(!p))
   {
      throw std::bad_alloc();
   }
   p->m_magic = TBT_MAGIC;
   try
   {
      p->m_pController = new tbt::Controller(pCtx, sControllerID.c_str());
   }
   catch (...)
   {
      p->m_magic = 0;
      free(p);
      tbt::CLEAR_PTR(p);
      throw;
   }
   return p;
}

void tbtController_delete(struct tbt_fwu_Controller* p)
{
   if (UNLIKELY(!p))
   {
      return;
   }
   if (UNLIKELY(!tbtController_valid(p)))
   {
      TBT_LOG(LOG_ERR, "Invalid tbt_fwu_Controller pointer %p: ignoring\n", p);
      return;
   }
   delete p->m_pController;
   tbt::CLEAR_PTR(p->m_pController);
   free(p);
   tbt::CLEAR_PTR(p);
}
