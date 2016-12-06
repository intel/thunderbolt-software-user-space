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
extern "C" {
#endif

/**
 * \return a human-readable string describing a TBT error code.  The behavior
 * is unspecified if the input error code is invalid.
 *
 * \param[in] tbt_errorCode
 *   A TBT error code previously returned by some other API function.
 */
extern const char* tbt_strerror(int tbt_errorCode);

/**
 * \return a detailed error message describing the last error experienced
 * by the library.  The returned string points to volatile space that may
 * change in the next call to the library.
 */
extern const char* tbt_lastErrorDetail();

/* clang-format off */

/*
 * If you add something here, make sure to add an entry to tbt_strerror.cpp.
 */

#define TBT_SUCCESS_RESPONSE_CODE                               0x0000
#define TBT_OK                                        TBT_SUCCESS_RESPONSE_CODE

// FW return codes
#define TBT_AUTHENTICATION_FAILED_RESPONSE_CODE                 0x0001
#define TBT_ACCESS_TO_RESTRICTED_AREA_RESPONSE_CODE             0x0002
#define TBT_GENERAL_ERROR_RESPONSE_CODE                         0x0003
#define TBT_AUTHENTICATION_IN_PROGRESS_RESPONSE_CODE            0x0004
#define TBT_NO_KEY_FOR_THE_SPECIFIED_UID_RESPONSE_CODE          0x0005
#define TBT_AUTHENTICATION_KEY_FAILED_RESPONSE_CODE             0x0006
#define TBT_AUTHENTICATION_BONDED_UUID_FAILED_RESPONSE_CODE     0x0007
#define TBT_SAFE_MODE_RESPONSE_CODE                             0x0009

// SW return codes
#define TBT_FW_RESPONSE_TIMEOUT_CODE                            0x0100
#define TBT_WRONG_IMAGE_SIZE_CODE                               0x0101
#define TBT_SERVICE_INTERNAL_ERROR_CODE                         0x0102
#define TBT_POWER_CYCLE_FAILED_CODE                             0x0103
#define TBT_INVALID_OPERATION_IN_SAFE_MODE                      0x0104
#define TBT_NOT_SUPPORTED_PLATFORM                              0x0105
#define TBT_INVALID_ARGUMENTS                                   0x0106
#define TBT_DEVICE_NOT_SUPPORTED                                0x0107
#define TBT_CONTROLLER_NOT_SUPPORTED                            0x0108
#define TBT_SDK_IN_USE                                          0x0109
#define TBT_DEPRECATED_METHOD                                   0x010a
#define TBT_I2C_ACCESS_NOT_SUPPORTED                            0x010b

// SDK and samples return codes
#define TBT_SDK_GENERAL_ERROR_CODE                              0x0200
#define TBT_SDK_INTERNAL_ERROR                                  0x0201
#define TBT_SDK_NO_COMMAND_SUPPLIED                             0x0202
#define TBT_SDK_COMMAND_NOT_FOUND                               0x0203
#define TBT_SDK_ARGUMENT_COUNT_MISMATCH                         0x0204
#define TBT_SDK_INVALID_CONTROLLER_ID                           0x0205
#define TBT_SDK_INVALID_DEVICE_UUID                             0x0206
#define TBT_SDK_FILE_NOT_FOUND                                  0x0207
#define TBT_SDK_SERVICE_NOT_FOUND                               0x0208
#define TBT_SDK_LOAD_CONTROLLERS_ERROR                          0x0209
#define TBT_SDK_LOAD_DEVICES_ERROR                              0x020a
#define TBT_SDK_NO_CONTROLLERS                                  0x020b
#define TBT_SDK_NO_DEVICES                                      0x020c
#define TBT_SDK_INVALID_OPERATION_IN_SAFE_MODE                  0x020d
#define TBT_SDK_NO_EP_UPDATE_SUPPORT                            0x020e
#define TBT_SDK_DEVICE_NOT_SUPPORTED                            0x020f
#define TBT_SDK_CONTROLLER_NOT_SUPPORTED                        0x0210
#define TBT_SDK_UNKNOWN_CHIP                                    0x0211
#define TBT_SDK_INVALID_IMAGE_FILE                              0x0212
#define TBT_SDK_IMAGE_VALIDATION_ERROR                          0x0213
#define TBT_SDK_HW_GENERATION_MISMATCH                          0x0214
#define TBT_SDK_PORT_COUNT_MISMATCH                             0x0215
#define TBT_SDK_CHIP_SIZE_ERROR                                 0x0216
#define TBT_SDK_IMAGE_FOR_HOST_ERROR                            0x0217
#define TBT_SDK_IMAGE_FOR_DEVICE_ERROR                          0x0218
#define TBT_SDK_PD_MISMATCH                                     0x0219
#define TBT_SDK_NO_DROM_IN_FILE_ERROR                           0x021a
#define TBT_SDK_DROM_MISMATCH                                   0x021b
#define TBT_SDK_VENDOR_MISMATCH                                 0x021c
#define TBT_SDK_MODEL_MISMATCH                                  0x021d
#define TBT_SDK_NO_MATCHING_DEVICE_FOUND                        0x021e
#define TBT_SDK_MULTIPLE_IMAGES_FOUND                           0x021f
#define TBT_SDK_COMMAND_IS_NOT_SUPPORTED_ON_DEVICE              0x0220
#define TBT_SDK_DEPRECATED_METHOD                               0x0221

#define TBT_SDK_OUT_OF_MEMORY                                   0x1000
#define TBT_SDK_MISUSE                                          0x1001
#define TBT_SDK_SERVICE_COMMUNICATION_FAILURE                   0x1002
#define TBT_SDK_SERVICE_FWU_TIMEOUT                             0x1003

/* clang-format on */

#ifdef __cplusplus
}
#endif
