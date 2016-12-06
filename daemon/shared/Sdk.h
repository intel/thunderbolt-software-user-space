/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2011 - 2016 Intel Corporation.
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


enum class TBTDRV_STATUS
{
	SUCCESS_RESPONSE_CODE,
	
	//FW RETURN CODES
	AUTHENTICATION_FAILED_RESPONSE_CODE,
	ACCESS_TO_RESTRICTED_AREA_RESPONSE_CODE,
	GENERAL_ERROR_RESPONSE_CODE,
	AUTHENTICATION_IN_PROGRESS_RESPONSE_CODE,
	NO_KEY_FOR_THE_SPECIFIED_UID_RESPONSE_CODE,
	AUTHENTICATION_KEY_FAILED_RESPONSE_CODE,
	AUTHENTICATION_BONDED_UUID_FAILED_RESPONSE_CODE,
	SAFE_MODE_RESPONSE_CODE = 0x9,

	//SW RETURN CODES
	FW_RESPONSE_TIMEOUT_CODE = 0x100,
	WRONG_IMAGE_SIZE_CODE,
	SERVICE_INTERNAL_ERROR_CODE,
	POWER_CYCLE_FAILED_CODE,
	INVALID_OPERATION_IN_SAFE_MODE,
	NOT_SUPPORTED_PLATFORM,
	INVALID_ARGUMENTS,
	DEVICE_NOT_SUPPORTED,
	CONTROLLER_NOT_SUPPORTED,
	SDK_IN_USE,
	DEPRECATED_METHOD,
	I2C_ACCESS_NOT_SUPPORTED,
};
