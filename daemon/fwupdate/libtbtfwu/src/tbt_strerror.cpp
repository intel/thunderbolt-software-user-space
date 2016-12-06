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

#include <string.h>
#include <errno.h>
#include <unordered_map>
#include <limits.h>
#include "tbt/tbt_fwu_err.h"

#include "likely.h"

namespace
{

std::unordered_map<uint16_t, std::string> tbt_strmap;

} // end anonymous namespace

namespace tbt
{
int tbt_init_strerror();
}

extern "C" {

const char* tbt_strerror(int tbt_errorCode)
{
   if (UNLIKELY(tbt_strmap.empty()))
   {
      tbt::tbt_init_strerror();
   }
   if (UNLIKELY(tbt_errorCode < 0 || tbt_errorCode > USHRT_MAX))
   {
      return "invalid error code";
   }
   uint16_t ec16 = (uint16_t)tbt_errorCode;
   auto it       = tbt_strmap.find(ec16);
   if (UNLIKELY(it == tbt_strmap.end()))
   {
      return "invalid error code";
   }
   return it->second.c_str();
}

} // extern "C"

namespace tbt
{

// FIXME this should be auto-generated from Resources.resx.
int tbt_init_strerror()
{
   try
   {
      tbt_strmap = {
         {TBT_SUCCESS_RESPONSE_CODE, "success"},

         {TBT_AUTHENTICATION_FAILED_RESPONSE_CODE, "FW: authentication failed"},
         {TBT_ACCESS_TO_RESTRICTED_AREA_RESPONSE_CODE, "FW: access to restricted area"},
         {TBT_GENERAL_ERROR_RESPONSE_CODE, "FW: general error"},
         {TBT_AUTHENTICATION_IN_PROGRESS_RESPONSE_CODE, "FW: authentication in progress"},
         {TBT_NO_KEY_FOR_THE_SPECIFIED_UID_RESPONSE_CODE, "FW: no key for the specified UID"},
         {TBT_AUTHENTICATION_KEY_FAILED_RESPONSE_CODE, "FW: authentication key failed"},
         {TBT_AUTHENTICATION_BONDED_UUID_FAILED_RESPONSE_CODE, "FW: authentication of bonded UUID failed"},
         {TBT_SAFE_MODE_RESPONSE_CODE, "FW: controller is in safe mode"},

         {TBT_FW_RESPONSE_TIMEOUT_CODE, "firmware response timeout"},
         {TBT_WRONG_IMAGE_SIZE_CODE, "wrong image size"},
         {TBT_SERVICE_INTERNAL_ERROR_CODE, "daemon reported an internal error"},
         {TBT_POWER_CYCLE_FAILED_CODE, "power cycle failed"},
         {TBT_INVALID_OPERATION_IN_SAFE_MODE, "daemon reported invalid operation: controller is in safe mode"},
         {TBT_NOT_SUPPORTED_PLATFORM, "unsupported host platform"},
         {TBT_INVALID_ARGUMENTS, "invalid arguments"},
         {TBT_DEVICE_NOT_SUPPORTED, "device not supported"},
         {TBT_CONTROLLER_NOT_SUPPORTED, "controller not supported"},
         {TBT_SDK_IN_USE, "daemon is busy"},
         {TBT_DEPRECATED_METHOD, "method is deprecated"},
         {TBT_I2C_ACCESS_NOT_SUPPORTED, "I2C access isn't supported"},

         {TBT_SDK_GENERAL_ERROR_CODE, "general SDK error"},
         {TBT_SDK_INTERNAL_ERROR, "internal SDK error"},
         {TBT_SDK_NO_COMMAND_SUPPLIED, "no command supplied"},
         {TBT_SDK_COMMAND_NOT_FOUND, "command not found"},
         {TBT_SDK_ARGUMENT_COUNT_MISMATCH, "argument count mismatch"},
         {TBT_SDK_INVALID_CONTROLLER_ID, "invalid controller ID"},
         {TBT_SDK_INVALID_DEVICE_UUID, "invalid device UUID"},
         {TBT_SDK_FILE_NOT_FOUND, "file not found"},
         {TBT_SDK_SERVICE_NOT_FOUND, "service not found"},
         {TBT_SDK_LOAD_CONTROLLERS_ERROR, "error loading controllers"},
         {TBT_SDK_LOAD_DEVICES_ERROR, "error loading devices"},
         {TBT_SDK_NO_CONTROLLERS, "no controllers found"},
         {TBT_SDK_NO_DEVICES, "no devices found"},
         {TBT_SDK_INVALID_OPERATION_IN_SAFE_MODE, "invalid operation: controller is in safe mode"},
         {TBT_SDK_NO_EP_UPDATE_SUPPORT, "controller does not support device firmware update"},
         {TBT_SDK_DEVICE_NOT_SUPPORTED, "device does not support device firmware update"},
         {TBT_SDK_CONTROLLER_NOT_SUPPORTED, "controller not supported"},
         {TBT_SDK_UNKNOWN_CHIP, "unknown chip"},
         {TBT_SDK_INVALID_IMAGE_FILE, "invalid image file"},
         {TBT_SDK_IMAGE_VALIDATION_ERROR, "image validation error"},
         {TBT_SDK_HW_GENERATION_MISMATCH, "HW generation mismatch"},
         {TBT_SDK_PORT_COUNT_MISMATCH, "port count mismatch"},
         {TBT_SDK_CHIP_SIZE_ERROR, "chip size error"},
         {TBT_SDK_IMAGE_FOR_HOST_ERROR, "image for host error"},
         {TBT_SDK_IMAGE_FOR_DEVICE_ERROR, "image for device error"},
         {TBT_SDK_PD_MISMATCH, "PD mismatch"},
         {TBT_SDK_NO_DROM_IN_FILE_ERROR, "no DROM in file"},
         {TBT_SDK_DROM_MISMATCH, "DROM mismatch"},
         {TBT_SDK_VENDOR_MISMATCH, "vendor mismatch"},
         {TBT_SDK_MODEL_MISMATCH, "model mismatch"},
         {TBT_SDK_NO_MATCHING_DEVICE_FOUND, "no matching device found"},
         {TBT_SDK_MULTIPLE_IMAGES_FOUND, "multiple images found"},
         {TBT_SDK_COMMAND_IS_NOT_SUPPORTED_ON_DEVICE, "command isn't supported on device"},
         {TBT_SDK_DEPRECATED_METHOD, "method is deprecated"},

         {TBT_SDK_OUT_OF_MEMORY, "out of memory"},
         {TBT_SDK_MISUSE, "misuse of API"},
         {TBT_SDK_SERVICE_COMMUNICATION_FAILURE, "failed to communicate with service"},
         {TBT_SDK_SERVICE_FWU_TIMEOUT, "timeout while waiting for firmware upgrade response"},
      };
   }
   catch (std::bad_alloc&)
   {
      return TBT_SDK_OUT_OF_MEMORY;
   }
   catch (...)
   {
      return TBT_SDK_GENERAL_ERROR_CODE;
   }
   return TBT_OK;
}

} // namespace tbt
