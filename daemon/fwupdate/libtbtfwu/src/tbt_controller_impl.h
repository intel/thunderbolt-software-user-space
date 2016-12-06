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
 * @file tbt_controller_impl.h
 *
 * This file defines the tbt_fwu_Controller struct used by clients of this library.
 */

#pragma once

#include <string>
#include <stdint.h>

namespace tbt
{
class Controller;
class DBusCtx;
}

extern "C" {

/**
 * This is the implementation of the user-facing C API structure representing
 * a controller.  The API client never allocates an object of this type; it
 * can only obtain instances via other API calls.  This allows us to retain
 * ABI compatibility more easily.
 */
struct tbt_fwu_Controller
{
   /**
    * Magic value to sanity check that we are getting sane pointers from
    * callers.
    */
   uint32_t m_magic;

   /** Handle on the C++ implementation for this controller */
   tbt::Controller* m_pController;
};

} // extern "C"

/**
 * Allocate a new tbt_fwu_Controller struct for 'sControllerID', using pCtx to
 * communicate with the daemon.
 *
 * @param[in] pCtx
 *   The D-Bus context needed to communicate with the daemon.
 *
 * @param[in] sControllerID
 *   The D-Bus ID of the controller that should be wrapped.
 *
 * @return a pointer to the newly-allocated controller struct.  The caller
 *   MUST free this with a subsequent call to tbtController_delete.
 *
 * @throw std::exception if some error occurs allocating or initializing
 *   the structure.
 */
extern struct tbt_fwu_Controller* tbtController_new(tbt::DBusCtx* pCtx, std::string sControllerID);

/**
 * Free all resources for a tbt_fwu_Controller object previously allocated by
 * a call to tbtController_new.
 *
 * @param[in] p
 *   Points to a tbt_fwu_Controller object previously returned by
 *   tbtController_new.  If this is NULL, then the call is a harmless no-op.
 */
extern void tbtController_delete(struct tbt_fwu_Controller* p);

/**
 * Return true if and only if 'p' is a valid TBT controller pointer.  Used
 * for sanity checking client program inputs.
 */
extern bool tbtController_valid(const tbt_fwu_Controller* p);
