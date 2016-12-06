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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

/**
 * \file
 */

#pragma once

#ifdef __cplusplus
#include <cinttypes>
#include <cstddef>
#else
#include <inttypes.h>
#include <stddef.h>
#endif

#include <tbt/tbt_fwu_controller.h>
#include <tbt/tbt_fwu_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize this library.  This function should be called once
 * at application start time.
 *
 * \return
 *   TBT_OK on success, or an error status from tbt_fwu_err.h
 *   on failure.  On failure, the caller may reattempt the tbt_fwu_init, but
 *   may not call any other API functions.
 */
extern int tbt_fwu_init(void);

/**
 * Shut down the library instance, bringing down all external connections
 * and cleaning up all state.
 *
 * After making this call, tbt_fwu_init must not be called again from the
 * same process.
 */
extern void tbt_fwu_shutdown(void);

/**
 * Get the list of available controllers and return the result as a
 * dynamically allocated array of pointers.  Each pointer points to
 * an opaque structure representing a controller.  The returned array
 * must be freed by the caller, by calling tbt_fwu_freeControllerList.
 *
 * \param[out] papControllers
 *   A pointer to an array of pointers to controller structures.
 *   On success, this is filled in with a pointer to the output array.  The
 *   space is allocated dynamically and must be freed by a subsequent call to
 *   tbt_fwu_freeControllerList.  If the return value is non-zero, then the
 *   passed-in pointer is left untouched.  In this case, the caller MUST NOT
 *   call tbt_fwu_freeControllerList on the pointer.
 *
 * \param[out] controllers_size
 *   A pointer to space that is filled in with the number of returned elements
 *   in `papControllers`.
 *
 * \return
 *   TBT_OK on success, or one of the error codes listed in tbt_fwu_err.h.
 *
 * \section Example
 * \code{.c}
 *   struct tbt_fwu_Controller** apControllers;
 *   size_t nControllers;
 *   int rc = tbt_fwu_getControllerList(&apControllers, &nControllers);
 *   if (rc != TBT_OK)
 *   {
 *      // Failure -- report the error, try again, or so on.
 *      return;
 *   }
 *   int i;
 *   for (i = 0; i < nControllers; i++)
 *   {
 *      struct tbt_fwu_Controller* pController = apControllers[i];
 *      // Do something interesting with the controller 'pController' using
 *      // other API calls.  E.g.:
 *      uint32_t major, minor;
 *      rc = tbt_fwu_Controller_getNVMVersion(pController, &major, &minor);
 *      if (rc == TBT_OK)
 *      {
 *         printf("Controller NVM version: %d %d\n", major, minor);
 *      }
 *   }
 *
 *   // Don't forget to free up the memory when done.
 *   tbt_fwu_freeControllerList(apControllers);
 * \endcode
 */
extern int
tbt_fwu_getControllerList(struct tbt_fwu_Controller*** papControllers,
                          size_t* controllers_size);

/**
 * Free a dynamically-allocated controller list previously returned
 * in tbt_fwu_getControllerList().
 */
extern void
tbt_fwu_freeControllerList(struct tbt_fwu_Controller** apControllers,
                           int controllers_size);

/**
 * Fetch the NVM version from the given firmware image.  The version
 * value itself is filled in to *pMajor and *pMinor.
 *
 * \param[in] image
 *   The image whose NVM version we should fetch.
 *
 * \param[in] image_size
 *   Size of image in bytes.
 *
 * \param[out] pMajor
 *   Pointer to space at which we should store the current major NVM version of
 *   pController.
 *
 * \param[out] pMinor
 *   Pointer to space at which we should store the current minor NVM version of
 *   pController.
 *
 * \return
 *   TBT_OK on success, or one of the standard error codes defined in
 *   tbt_fwu_err.h.
 */
extern int tbt_fwu_getImageNVMVersion(const uint8_t* image,
                                      size_t image_size,
                                      uint32_t* major,
                                      uint32_t* minor);

/**
 * Set the log level to 'level'.  Once set, this library will not emit
 * syslog messages at levels lower than 'level'.
 *
 * \param[in] level
 *   The syslog level (defined in syslog.h) to set this library's logging to.
 *   E.g. LOG_ERR, LOG_INFO, LOG_WARN, etc.  LOG_DEBUG is the lowest level and
 *   LOG_EMERG is the highest level.  Set to -1 to disable logging.
 */
extern void tbt_fwu_setLogLevel(int level);

#ifdef __cplusplus
}
#endif
