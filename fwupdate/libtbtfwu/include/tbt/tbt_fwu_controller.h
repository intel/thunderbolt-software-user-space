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

#pragma once

/**
 * \file
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct tbt_fwu_Controller
 *
 * A structure representing a single TBT controller on this host.  Clients
 * of this library never allocate an instance of this structure on their own;
 * they obtain instances via calls to this library.
 *
 * The layout details are left to the implementation.
 */
struct tbt_fwu_Controller;

/**
 * Find the underlying ID of the given controller.
 *
 * \param[in] pController
 *   A controller structure previously returned by a call to
 *   tbt_fwu_getControllerList().
 *
 * \param[out] str
 *   Pointer to space at which we should store the ID of the controller.
 *
 * \param[in,out] str_size
 *   The number of bytes of space available at `str`.  This space will
 *   not be exceeded.  On successful return, contains the number of bytes
 *   required to fully fill in the ID, including the null terminator.  (This
 *   can be used as an initial query to determine the amount of space required.)
 *
 * \return
 *   TBT_OK on success, or one of the standard error codes defined in
 * tbt_fwu_err.h.
 */
extern int tbt_fwu_Controller_getID(struct tbt_fwu_Controller* pController,
                                    char* str,
                                    size_t* str_size);

/**
 * Fetch the model ID on the given controller.  The model ID
 * value itself is filled in to *id.
 *
 * \param[in] pController
 *   The controller we should query.  This is a pointer previously returned
 *   by tbt_fwu_getControllerList().
 *
 * \param[out] id
 *   Pointer to space at which we should store the value.
 *
 * \return
 *   TBT_OK on success, or one of the standard error codes defined in
 *   tbt_fwu_err.h.
 */
extern int tbt_fwu_Controller_getModelID(struct tbt_fwu_Controller* pController,
                                         uint16_t* id);

/**
 * Fetch the vendor ID on the given controller.  The vendor ID
 * value itself is filled in to *id.
 *
 * \param[in] pController
 *   The controller we should query.  This is a pointer previously returned
 *   by tbt_fwu_getControllerList().
 *
 * \param[out] id
 *   Pointer to space at which we should store the value.
 *
 * \return
 *   TBT_OK on success, or one of the standard error codes defined in
 *   tbt_fwu_err.h.
 */
extern int
tbt_fwu_Controller_getVendorID(struct tbt_fwu_Controller* pController,
                               uint16_t* id);

/**
 * Determine whether the controller `pController` is in safe mode.
 *
 * \param[in] pController
 *   The controller whose safe-mode status we should fetch.  This is
 *   a structure previously returned by tbt_fwu_getControllerList().
 *
 * \param[out] safe_mode
 *   Pointer to a buffer to fill in with a boolean indicator for whether
 *   the controller is in safe mode.  On successful return, this will be
 *   filled with nonzero if the controller is in safe mode and zero
 *   otherwise.
 *
 * \return
 *   TBT_OK if the safe mode status was successfully determined, or one of
 *   the standard error codes defined in tbt_fwu_err.h.
 */
extern int
tbt_fwu_Controller_isInSafeMode(struct tbt_fwu_Controller* pController,
                                int* safe_mode);

/**
 * Fetch the current NVM version on the given controller.  The version
 * value itself is filled in to *major and *minor.
 *
 * \param[in] pController
 *   The controller whose version we should fetch.  This is
 *   a pointer previously returned by tbt_fwu_getControllerList().
 *
 * \param[out] major
 *   Pointer to space at which we should store the current major NVM version of
 *   pController.
 *
 * \param[out] minor
 *   Pointer to space at which we should store the current minor NVM version of
 *   pController.
 *
 * \return
 *   TBT_OK on success, or one of the standard error codes defined in
 *   tbt_fwu_err.h.
 */
extern int
tbt_fwu_Controller_getNVMVersion(struct tbt_fwu_Controller* pController,
                                 uint32_t* major,
                                 uint32_t* minor);

/**
 * Confirm that the given firmware image in `image` is valid with respect to
 * the controller `pController`.
 *
 * \param[in] pController
 *   The controller against which we should validate the image.  This is
 *   a structure previously returned by tbt_fwu_getControllerList().
 *
 * \param[in] image
 *   Byte array containing the firmware image to validate.
 *
 * \param[in] image_size
 *   Number of bytes pointed to by image.
 *
 * \return
 *   TBT_OK if the image is valid, or one of the standard error codes
 *   defined in tbt_fwu_err.h if the image is invalid or its validity status
 *   is indeterminate.
 */
extern int
tbt_fwu_Controller_validateFWImage(struct tbt_fwu_Controller* pController,
                                   const uint8_t* image,
                                   size_t image_size);

/**
 * Update the controller `pController` to the firmware
 * image in `image`.  Unless the controller is in safe mode, make an
 * initial attempt to validate that `image` is a well-formed and
 * compatible image.  If it is not, then do not attempt the update and report
 * the error.  If the controller is in safe mode, then make no attempt to
 * validate the image; simply press on with the update.
 *
 * The calling thread will block during the update process.
 *
 * \param[in] pController
 *   The controller whose firmware we should update.  This is
 *   a structure previously returned by tbt_fwu_getControllerList().
 *
 * \param[in] image
 *   Byte array containing the firmware image to validate.
 *
 * \param[in] image_size
 *   Number of bytes pointed to by image.
 *
 * \return
 *   TBT_OK if the update was successful, or one of the standard error codes
 *   defined in tbt_fwu_err.h.
 */
extern int tbt_fwu_Controller_updateFW(struct tbt_fwu_Controller* pController,
                                       const uint8_t* image,
                                       size_t image_size);

#ifdef __cplusplus
} /* extern "C" */
#endif
