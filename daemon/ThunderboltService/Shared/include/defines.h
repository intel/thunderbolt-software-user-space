/*******************************************************************************
 *
 * Intel Thunderbolt(TM) daemon
 * Copyright(c) 2014 - 2015 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * Intel Thunderbolt Mailing List <thunderbolt-software@lists.01.org>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/

#ifndef _DEFINES_
#define _DEFINES_

#include "MessagesWrapper.h"
#include <stdint.h>
#include <string>
#include <vector>


typedef std::wstring controlleriD;
typedef std::wstring NVMVersion;
typedef std::wstring DeviceUUID;

#define IS_SETTINGS_CMD(cmd) (cmd >= SET_FW_MODE_FD1_D1_CERT_COMMAND_CODE && cmd <= SET_FW_MODE_FDA_DA_ALL_COMMAND_CODE)
#define MAX_DRIVER_VERSION 20

typedef struct {
	uint32_t controller_id;
	uint16_t nvm_offset;
	uint8_t  num_of_ports;
	uint8_t  dma_port;
	char driver_version[MAX_DRIVER_VERSION];
   bool supportsFullE2E;
} QueryDriverInformation;

typedef struct _HostConfiguration
{
   uint32_t win_cert_allow_any : 1,
   dock_allow_any : 1,
   gpu_allow_any : 1,
   host_security : 3,	//enum HOST_SECURITY
   host_power : 4,		//enum HOST_POWER_TYPE
   sx_exit : 1,			//host awake from SX
   port_num : 3,		   //number of host port
   reserved : 10,
   error_flags : 7,
   valid_flag : 1;		//host has been initialized correctly (driver ready ack)
}HostConfiguration;

enum class ThunderboltGeneration
{
	THUNDERBOLT_1 = 1,
	THUNDERBOLT_2 = 2 ,
	THUNDERBOLT_3 = 3
};


#endif // !_DEFINES_

