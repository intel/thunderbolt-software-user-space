/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2009 - 2016 Intel Corporation.
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

#include "MessagesWrapper.h"
#include <stdint.h>
#include <string>
#include <vector>

typedef std::wstring controlleriD;
typedef std::wstring NVMVersion;
typedef std::wstring DeviceUUID;

#define IS_SETTINGS_CMD(cmd) (cmd >= SET_FW_MODE_FD1_D1_CERT_COMMAND_CODE && cmd <= SET_FW_MODE_FDA_DA_ALL_COMMAND_CODE)
#define MAX_DRIVER_VERSION 20

typedef struct
{
   uint32_t controller_id;
   uint16_t nvm_offset;
   uint8_t num_of_ports;
   uint8_t dma_port;
   char driver_version[MAX_DRIVER_VERSION];
   bool supportsFullE2E;
} QueryDriverInformation;

typedef struct _HostConfiguration
{
   uint32_t win_cert_allow_any : 1, dock_allow_any : 1, gpu_allow_any : 1,
      host_security : 3, // enum HOST_SECURITY
      host_power : 4,    // enum HOST_POWER_TYPE
      sx_exit : 1,       // host awake from SX
      port_num : 3,      // number of host port
      reserved : 10, error_flags : 7,
      valid_flag : 1; // host has been initialized correctly (driver ready ack)
} HostConfiguration;

enum class ThunderboltGeneration
{
   THUNDERBOLT_1 = 1,
   THUNDERBOLT_2 = 2,
   THUNDERBOLT_3 = 3
};
