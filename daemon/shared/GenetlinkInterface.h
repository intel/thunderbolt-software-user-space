/*******************************************************************************
 *
 * Intel Thunderbolt daemon
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
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/


#ifndef NHI_GENL_VERSION_H_
#define NHI_GENL_VERSION_H_

//Interface version, driver may support multiple versions
#define NHI_GENL_VERSION 1

//Netlink user defined header, currently include one DW that represent the controller ID
#define NHI_GENL_USER_HEADER_SIZE 4

typedef enum
{
	NHI_GENL_EVENT_SOURCE_FW,
	NHI_GENL_EVENT_SOURCE_DRIVER
} NHI_GENL_EVENT_SOURCE;

//Define attributes types according to netlink protocol
typedef enum {
	NHI_ATTR_UNSPEC,				//unknown attribute type
	NHI_ATTR_DRIVER_VERSION,
	NHI_ATTR_NVM_VER_OFFSET,
	NHI_ATTR_NUM_PORTS,
	NHI_ATTR_DMA_PORT,
	NHI_ATTR_SUPPORT_FULL_E2E,
	NHI_ATTR_MAILBOX_CMD,
	NHI_ATTR_PDF,
	NHI_ATTR_MSG_TO_ICM,
	NHI_ATTR_MSG_FROM_ICM,
	NHI_ATTR_LOCAL_ROUTE_STRING,
	NHI_ATTR_LOCAL_UNIQUE_ID,
	NHI_ATTR_REMOTE_UNIQUE_ID,
	NHI_ATTR_LOCAL_DEPTH,
	NHI_ATTR_ENABLE_FULL_E2E,
	NHI_ATTR_MATCH_FRAME_ID,
	__NHI_ATTR_MAX,
} NHI_GENL_ATTR;

//Max number of defined attributes
#define NHI_ATTR_MAX (__NHI_ATTR_MAX - 1)

//Defince netlink commands
typedef enum {
	NHI_CMD_UNSPEC,					//unspecified command
	NHI_CMD_SUBSCRIBE,				//send
	NHI_CMD_UNSUBSCRIBE,			//send
	NHI_CMD_QUERY_INFORMATION,  	//response
	NHI_CMD_MSG_TO_ICM,				//send
	NHI_CMD_MSG_FROM_ICM,	    	//events
	NHI_CMD_MAILBOX,				//send
	NHI_CMD_APPROVE_TBT_NETWORKING,	//send
	NHI_CMD_ICM_IN_SAFE_MODE,
	__NHI_CMD_MAX,
} NHI_GENL_CMD;

//Max number of defined driver commands
#define NHI_CMD_MAX (__NHI_CMD_MAX - 1)

//netlink family name, used as referance to remote netlink server
#define NHI_GENL_NAME "thunderbolt"

#endif // NHI_GENL_VERSION_H_
