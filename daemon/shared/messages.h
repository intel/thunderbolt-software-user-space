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


#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "general.h"

/************************ definitions ************************/
#define MAX_FW_FRAME_SIZE  256

#define PORT_VSEC_BASE     0x3E
#define MAIL_DATA_0_OFFSET 1
#define MAIL_IN_OFFSET     17
#define MAIL_OUT_OFFSET    18
#define MAIL_DATA_BASE     (PORT_VSEC_BASE + MAIL_DATA_0_OFFSET)
#define MAIL_DATA_0        (PORT_VSEC_BASE + MAIL_DATA_0_OFFSET)
#define MAIL_IN            (PORT_VSEC_BASE + MAIL_IN_OFFSET)
#define MAIL_OUT           (PORT_VSEC_BASE + MAIL_OUT_OFFSET)

#define ROUTE_STRING_VALUE 0

typedef UINT32 UNIQUE_ID[4];
typedef UINT32 TBT_HASH[8];
typedef TBT_HASH CHALLENGE;
typedef TBT_HASH CHALLENGE_RESPONSE;
typedef TBT_HASH DEVICE_KEY;
typedef UINT32 END_POINT_NAME[32]; //Connected End Point Identity (ASCII string from the device DROM)
typedef UINT32 TOPOLOGY_ID[2];
typedef struct _ROUTE_STRING
{
   // Route String Hi
   UINT32 Level4PortNum : BITFIELD_RANGE( 0, 5 );
   UINT32               : BITFIELD_RANGE( 6, 7 );
   UINT32 Level5PortNum : BITFIELD_RANGE( 8, 13 );
   UINT32               : BITFIELD_RANGE( 14, 15 );
   UINT32 Level6PortNum : BITFIELD_RANGE( 16, 21 );
   UINT32               : BITFIELD_RANGE( 22, 30 );
   UINT32 CM            : BITFIELD_BIT( 31 );

   // Route String Lo
   UINT32 Level0PortNum : BITFIELD_RANGE( 0, 5 );
   UINT32               : BITFIELD_RANGE( 6, 7 );
   UINT32 Level1PortNum : BITFIELD_RANGE( 8, 13 );
   UINT32               : BITFIELD_RANGE( 14, 15 );
   UINT32 Level2PortNum : BITFIELD_RANGE( 16, 21 );
   UINT32               : BITFIELD_RANGE( 22, 23 );
   UINT32 Level3PortNum : BITFIELD_RANGE( 24, 29 );
   UINT32               : BITFIELD_RANGE( 30, 31 );
} ROUTE_STRING;


// Each physical port contains 2 channels.
// Devices are exposed to user based on physical ports.
#define TBT_CHANNEL_PER_PORT_NUM 2

// Calculate host physcial port number (Zero-based numbering) from host channel/link which starts from 1:
// 1 and 2 -> 0, 3 and 4 -> 1, etc ...
#define TBT_GET_PORT_BY_LINK(_link) ((_link - 1) / TBT_CHANNEL_PER_PORT_NUM)

// EP name consts - For parsing the EP name structure
#define EP_NAME_VENDOR_NAME_STRUCT_LENGTH_OFFSET                              0
#define EP_NAME_VENDOR_NAME_STRUCT_VALUE_OFFSET                               1
#define EP_NAME_VENDOR_NAME_DATA_OFFSET_INTERNAL                              2
#define EP_NAME_VENDOR_NAME_STRUCT_VALUE                                      1
#define EP_NAME_GET_MODEL_NAME_STRUCT_LENGTH_OFFSET(VendorNameStructLength)   (VendorNameStructLength)
#define EP_NAME_GET_MODEL_NAME_STRUCT_VALUE_OFFSET(VendorNameStructLength)    (VendorNameStructLength + 1)
#define EP_NAME_MODEL_NAME_DATA_OFFSET_INTERNAL                               2
#define EP_NAME_GET_MODEL_NAME_DATA_OFFSET(VendorNameStructLength)            (VendorNameStructLength + EP_NAME_MODEL_NAME_DATA_OFFSET_INTERNAL)
#define EP_NAME_MODEL_NAME_STRUCT_VALUE                                       2

// PDF values for SW<->FW communication in raw mode
typedef enum _PDF_VALUE
{
   PDF_DRIVER_QUERY_DRIVER_INFORMATION =-1,
   PDF_READ_CONFIGURATION_REGISTERS = 1,   // CIO read request/response
   PDF_WRITE_CONFIGURATION_REGISTERS,  // CIO write request/response
   PDF_ERROR_NOTIFICATION,
   PDF_ERROR_ACKNOWLEDGEMENT,
   PDF_PLUG_EVENT_NOTIFICATION,
   PDF_INTER_DOMAIN_REQUEST,
   PDF_INTER_DOMAIN_RESPONSE,
   PDF_CM_OVERRIDE,
   PDF_RESET_CIO_SWITCH,
   PDF_FW_TO_SW_NOTIFICATION,          // FW->SW notification
   PDF_SW_TO_FW_COMMAND,               // SW->FW command
   PDF_FW_TO_SW_RESPONSE               // FW->SW response
} PDF_VALUE;

// SW->FW commands
typedef enum _SW_TO_FW_COMMAND_CODE
{
   GET_THUNDERBOLT_TOPOLOGY_COMMAND_CODE = 1,
   GET_VIDEO_RESOURCES_DATA_COMMAND_CODE,
   DRIVER_READY_COMMAND_CODE,
   APPROVE_PCI_CONNECTION_COMMAND_CODE,
   CHALLENGE_PCI_CONNECTION_COMMAND_CODE,
   ADD_DEVICE_AND_KEY_COMMAND_CODE,
   APPROVE_INTER_DOMAIN_CONNECTION_COMMAND_CODE = 0x10
} SW_TO_FW_COMMAND_CODE;

// FW->SW responses
typedef enum _FW_TO_SW_RESPONSE_CODE
{
   GET_THUNDERBOLT_TOPOLOGY_RESPONSE_CODE = 1,
   GET_VIDEO_RESOURCES_DATA_RESPONSE_CODE,
   DRIVER_READY_RESPONSE_CODE,
   APPROVE_PCI_CONNECTION_RESPONSE_CODE,
   CHALLENGE_PCI_CONNECTION_RESPONSE_CODE,
   ADD_DEVICE_AND_KEY_RESPONSE_CODE,
   INTER_DOMAIN_PACKET_SENT_RESPONSE_CODE = 8,
   APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE_CODE = 0x10
} FW_TO_SW_RESPONSE_CODE;


// FW->SW notifications
typedef enum _FW_TO_SW_NOTIFICATION_CODE
{
   DEVICE_CONNECTED_NOTIFICATION_CODE = 3,
   DEVICE_DISCONNECTED_NOTIFICATION_CODE,
   DP_DEVICE_CONNECTED_NOT_TUNNELED_NOTIFICATION_CODE,
   INTER_DOMAIN_CONNECTED_NOTIFICATION_CODE,
   INTER_DOMAIN_DISCONNECTED_NOTIFICATION_CODE
} FW_TO_SW_NOTIFICATION_CODE;

// SW -> FW mailbox commands
typedef enum _SW_TO_FW_INMAILCMD_CODE
{
   STOP_CM_ACTIVITY_COMMAND_CODE,
   ENTER_PASS_THROUGH_MODE_COMMAND_CODE,
   ENTER_CM_OWNERSHIP_MODE_COMMAND_CODE,
   DRIVER_LOADED_COMMAND_CODE,
   DRIVER_UNLOADED_COMMAND_CODE,
   SAVE_CURRENT_CONNECTED_DEVICES_COMMAND_CODE,
   DISCONNECT_PCIE_PATHS_COMMAND_CODE,
   DRIVER_UNLOADS_AND_DISCONNECT_INTER_DOMAIN_PATHS_COMMAND_CODE,
   DISCONNECT_PORT_A_INTER_DOMAIN_PATH = 0x10,
   DISCONNECT_PORT_B_INTER_DOMAIN_PATH,
   DP_TUNNEL_MODE_IN_ORDER_PER_CAPABILITIES = 0x1E,
   DP_TUNNEL_MODE_MAXIMIZE_SNK_SRC_TUNNELS,
   SET_FW_MODE_FD1_D1_CERT_COMMAND_CODE = 0x20,
   SET_FW_MODE_FD1_D1_ALL_COMMAND_CODE,
   SET_FW_MODE_FD1_DA_CERT_COMMAND_CODE,
   SET_FW_MODE_FD1_DA_ALL_COMMAND_CODE,
   SET_FW_MODE_FDA_D1_CERT_COMMAND_CODE,
   SET_FW_MODE_FDA_D1_ALL_COMMAND_CODE,
   SET_FW_MODE_FDA_DA_CERT_COMMAND_CODE,
   SET_FW_MODE_FDA_DA_ALL_COMMAND_CODE
} SW_TO_FW_INMAILCMD_CODE;

// SW -> FW mailbox requests
typedef enum _MAILBOX_REQUEST_CODE
{
   NON_ACTIVE_FLASH_REGION_WRITE_REQUEST_CODE,
   FLASH_UPDATE_AUTHENTICATE_REQUEST_CODE,
   FLASH_REGION_READ_REQUEST_CODE,
   CHALLENGE_DEVICE_REQUEST_CODE,
   POWER_CYCLE_REQUEST_CODE,
   ADD_DEVICE_KEY_REQUEST_CODE
} MAILBOX_REQUEST_CODE;

// FW -> SW mailbox responses
typedef enum _MAILBOX_RESPONSE_CODE
{
   COMMAND_COMPLETED_SUCCESS_RESPONSE_CODE,
   AUTHENTICATION_FAILED_RESPONSE_CODE,
   ACCESS_TO_RESTRICTED_AREA_RESPONSE_CODE,
   GENERAL_ERROR_RESPONSE_CODE,
   AUTHENTICATION_IN_PROGRESS_RESPONSE_CODE,
   NO_KEY_FOR_THE_SPECIFIED_UID_RESPONSE_CODE
} MAILBOX_RESPONSE_CODE;

// CIO error codes
typedef enum _CIO_ERROR_CODE
{
   PORT_DISCONNECTED_ERROR_CODE,
   CONFIG_SPACE_UNSUPPORTED_ERROR_CODE,
   INVALID_CONFIG_REGISTER_ADDRESS_ERROR_CODE,
   HOP_ID_OUT_OF_RANGE_ERROR_CODE,
   PORT_NUMBER_OUT_OF_RANGE_ERROR_CODE,
   CREDITS_ALLOCATED_OUT_OF_RANGE_ERROR_CODE,
   BANDWIDTH_OVERSUBSCRIBED_ERROR_CODE,
   RESOURCE_NOT_AVAILABLE_ERROR_CODE,
   TOPOLOGY_ID_NOT_ZERO_ERROR_CODE,
   INVALID_CONFIG_PORT_ERROR_CODE,
   REQUEST_TOO_LARGE_ERROR_CODE,
   INVALID_WRITE_ERROR_CODE,
   HEC_ERROR_DETECTED_ERROR_CODE,
   FLOW_CONTROL_ERROR_ERROR_CODE
} CIO_ERROR_CODE;

// The configuration space that is accessed by request
typedef enum _CIO_CONFIGURATION_SPACE
{
   PATH_CONFIG_SPACE,
   PORT_CONFIG_SPACE,
   DEVICE_CONFIG_SPACE,
   COUNTERS_CONFIG_SPACE
} CIO_CONFIGURATION_SPACE;

// Security levels
typedef enum _SECURITY_LEVEL
{
   AUTO_CONNECT_SECURITY,           // SL0, only rejected device notifications to SW, auto-approve
   USER_AUTHORIZATION_SECURITY,     // SL1, Redwood Ridge support and up, based on UUID
   SECURE_CONNECT_SECURITY,         // SL2, Falcon Ridge support and up, key save and challenge
   DISPLAY_PORT_ONLY_SECURITY       // SL3, Only Non-PCIe devices
} SECURITY_LEVEL;

// Device power
typedef enum _DEVICE_POWER
{
   SELF_POWERED,
   BUS_POWERED_NORMAL,
   BUS_POWERED_HIGH,
   DEVICE_POWER_UNKNOWN
} DEVICE_POWER;

// Flash region
typedef enum _FLASH_REGION
{
   CSS_HEADER,
   NON_ACTIVE_REGION,
   ACTIVE_REGION
} FLASH_REGION;

// Thunderbolt controllers
typedef enum _CONTROLLER_TYPE
{
   LEGACY_CONTROLLER,      // LR, ER, PR, CR, RR, FR, WR
   SKL_CONTROLLER          // AR
} CONTROLLER_TYPE;

// XDomain Packet Types
typedef enum _XDOMAIN_PACKET_TYPE
{
   XDOMAIN_UUID_RESPONSE_TYPE = 2,
   XDOMAIN_PROPERTIES_READ_REQUEST_TYPE,
   XDOMAIN_PROPERTIES_READ_RESPONSE_TYPE,
   XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION_TYPE,
   XDOMAIN_PROPERTIES_CHANGED_RESPONSE_TYPE,
   XDOMAIN_ERROR_RESPONSE_TYPE,
   XDOMAIN_UUID_REQUEST_TYPE = 12,
   XDOMAIN_LINK_STATE_STATUS_REQUEST_TYPE = 15,
   XDOMAIN_LINK_STATE_STATUS_RESPONSE_TYPE,
   XDOMAIN_LINK_STATE_CHANGE_REQUEST_TYPE,
   XDOMAIN_LINK_STATE_CHANGE_RESPONSE_TYPE,
   XDOMAIN_AUTHENTICAION_TYPE
} XDOMAIN_PACKET_TYPE;

// XDomain Error Response codes
typedef enum _XDOMAIN_ERROR_RESPONSE_CODE
{
   XDOMAIN_SUCCESS,
   XDOMAIN_UNKNOWN_PACKET_TYPE,
   XDOMAIN_UNKNOWN_DOMAIN,
   XDOMAIN_NOT_SUPPORTED,
   XDOMAIN_NOT_READY
} XDOMAIN_ERROR_RESPONSE_CODE;

// ThunderboltIP Packet Types
typedef enum _THUNDERBOLT_IP_PACKET_TYPE
{
   THUNDERBOLT_IP_LOGIN_TYPE,
   THUNDERBOLT_IP_LOGIN_RESPONSE_TYPE,
   THUNDERBOLT_IP_LOGOUT_TYPE,
   THUNDERBOLT_IP_STATUS_TYPE
} THUNDERBOLT_IP_PACKET_TYPE;

#define XDOMAIN_PROTOCOL_OFFSET                 FIELD_OFFSET( XDOMAIN_PROPERTIES_READ_REQUEST, XDomainDiscoveryProtocolUUID )
#define XDOMAIN_PACKET_TYPE_OFFSET              FIELD_OFFSET( XDOMAIN_PROPERTIES_READ_REQUEST, PacketType )
#define XDOMAIN_DISCOVERY_PROTOCOL_UUID         {0x0ED738B6, 0xBB40FF42, 0xE290C297, 0x07FFB2C0}
#define APPLE_THUNDERBOLT_IP_PROTOCOL_UUID      {0x9E588F79, 0x478A1636, 0x6456C697, 0xDDC820A9}
#define APPLE_THUNDERBOLT_IP_PROTOCOL_REVISION  1

#ifdef INTEL_P2P_PROTOCOL_SUPPORTED
   #define INTEL_PROTOCOL_OFFSET                FIELD_OFFSET( P2P_MSG_HDR, Version )
   #define INTEL_PACKET_TYPE_OFFSET             FIELD_OFFSET( P2P_MSG_HDR, MsgType )
#endif

#define BYTE_OFFSET_IN_DWORD_SWAPPED_BUFFER(byte_offset)          (((byte_offset) & ~3) + (3 - ((byte_offset) % 4)))    //byte_offset is the offest in the non-swapped buffer
#define MESSAGE_CODE_OFFSET                                       0
#define MESSAGE_CODE_OFFSET_IN_DWORD_SWAPPED_BUFFER               BYTE_OFFSET_IN_DWORD_SWAPPED_BUFFER( MESSAGE_CODE_OFFSET )

/************************ structs ************************/

typedef struct _GET_THUNDERBOLT_TOPOLOGY_COMMAND
{
   // DWORD 0
   UINT32   RequestCode                               : BITFIELD_RANGE( 0, 7 );     // SW_TO_FW_COMMAND_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 31 );

   // DWORD 1
   UINT32   Crc;
} GET_THUNDERBOLT_TOPOLOGY_COMMAND;

C_ASSERT( sizeof( GET_THUNDERBOLT_TOPOLOGY_COMMAND ) == (2 * sizeof( UINT32 )) );

typedef struct _TOPOLOGY_DATA
{
   // DWORD 0 - 1
   TOPOLOGY_ID TopologyID;

   // DWORD 2
   UINT32   Valid                                     : BITFIELD_BIT( 0 );
   UINT32   UpstreamPort                              : BITFIELD_RANGE( 1, 7 );
   UINT32   MaxPort                                   : BITFIELD_RANGE( 8, 11 );
   UINT32   DeviceID                                  : BITFIELD_RANGE( 12, 14 );
   UINT32                                             : BITFIELD_BIT( 15 );
   UINT32   Pointer                                   : BITFIELD_RANGE( 16, 23 );
   UINT32   SwitchIndex                               : BITFIELD_RANGE( 24, 27 );
   UINT32                                             : BITFIELD_RANGE( 28, 31 );

   // DWORD 3
   UINT32   Reserved;

   // DWORD 4 - 19
   struct
   {
      UINT32   PortAdaptorType                           : BITFIELD_RANGE( 0, 23 );
      UINT32   SwitchPacketindex                         : BITFIELD_RANGE( 24, 31 );
   } SwitchList[16];

   // DWORD 20 - 35
   UINT32   PathInformation[16];
}TOPOLOGY_DATA;

C_ASSERT( sizeof( TOPOLOGY_DATA ) == (36 * sizeof( UINT32 )) );

typedef struct _GET_THUNDERBOLT_TOPOLOGY_RESPONSE
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32                                             : BITFIELD_RANGE( 9, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1 - 36
   TOPOLOGY_DATA  TopologyData;

   // DWORD 37
   UINT32   Crc;
} GET_THUNDERBOLT_TOPOLOGY_RESPONSE;

C_ASSERT( sizeof( GET_THUNDERBOLT_TOPOLOGY_RESPONSE ) == (38 * sizeof( UINT32 )) );

typedef struct _GET_VIDEO_RESOURCES_DATA_COMMAND
{
   // DWORD 0
   UINT32   RequestCode                               : BITFIELD_RANGE( 0, 7 );     // SW_TO_FW_COMMAND_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 31 );

   // DWORD 1
   UINT32   Crc;
} GET_VIDEO_RESOURCES_DATA_COMMAND;

C_ASSERT( sizeof( GET_VIDEO_RESOURCES_DATA_COMMAND ) == (2 * sizeof( UINT32 )) );

typedef struct _GET_VIDEO_RESOURCES_DATA_RESPONSE1
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32                                             : BITFIELD_RANGE( 9, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1 - 4
   UINT32   InPortsParticipateInGivenTunnelList[4];

   // DWORD 5 - 8
   UINT32   OutPortsParticipateInGivenTunnelList[4];

   // DWORD 9 - 12
   UINT32   SwitchesIndicesParticipateInGivenTunnelList[4];

   // DWORD 13
   UINT32   ActiveLengthOfTunnel                      : BITFIELD_RANGE( 0, 7 );
   UINT32   UpperHopsAreBeingUsed                     : BITFIELD_RANGE( 8, 15 );
   UINT32   BWOccupiedForGivenTunnel                  : BITFIELD_RANGE( 16, 23 );
   UINT32   LengthToBeRetrieved                       : BITFIELD_RANGE( 24, 31 );

   // DWORD 14
   UINT32   Crc;
} GET_VIDEO_RESOURCES_DATA_RESPONSE1;

C_ASSERT( sizeof( GET_VIDEO_RESOURCES_DATA_RESPONSE1 ) == (15 * sizeof( UINT32 )) );

typedef struct _GET_VIDEO_RESOURCES_DATA_RESPONSE2
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32                                             : BITFIELD_RANGE( 9, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32   AdapterType;

   // DWORD 2
   UINT32   SwitchIndex                               : BITFIELD_RANGE( 0, 7 );
   UINT32   ResourceIsBeingUsed                       : BITFIELD_RANGE( 8, 15 );
   UINT32   ResourceIsBeingValid                      : BITFIELD_RANGE( 16, 23 );
   UINT32   PortNumber                                : BITFIELD_RANGE( 24, 31 );

   // DWORD 3
   UINT32   Crc;
} GET_VIDEO_RESOURCES_DATA_RESPONSE2;

C_ASSERT( sizeof( GET_VIDEO_RESOURCES_DATA_RESPONSE2 ) == (4 * sizeof( UINT32 )) );

typedef struct _DRIVER_READY_COMMAND
{
   // DWORD 0
   UINT32   RequestCode                               : BITFIELD_RANGE( 0, 7 );     // SW_TO_FW_COMMAND_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 13 );
   UINT32   DriverUnloads                             : BITFIELD_BIT( 14 );
   UINT32   DisconnectDmaPaths                        : BITFIELD_BIT( 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 1
   UINT32   Crc;
} DRIVER_READY_COMMAND;

C_ASSERT( sizeof( DRIVER_READY_COMMAND ) == (2 * sizeof( UINT32 )) );

typedef struct _DRIVER_READY_RESPONSE
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32   AllowAnyThunderboltDevice                 : BITFIELD_BIT( 9 );
   UINT32   AllowDockDevicesToConnectAtAnyDepth       : BITFIELD_BIT( 10 );
   UINT32   Allow1stDepthDevicesToConnectAtAnyDepth   : BITFIELD_BIT( 11 );
   UINT32   MaximizeSnkSrcTunnels                     : BITFIELD_BIT( 12 );
   UINT32                                             : BITFIELD_RANGE( 13, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32   FwRomVersion                              : BITFIELD_RANGE( 0, 7 );
   UINT32   FwRamVersion                              : BITFIELD_RANGE( 8, 15 );
   UINT32   SecurityLevel                             : BITFIELD_RANGE( 16, 19 );      // SECURITY_LEVEL enum
   UINT32                                             : BITFIELD_RANGE( 20, 31 );

   // DWORD 2
   UINT32   Crc;
} DRIVER_READY_RESPONSE;

C_ASSERT( sizeof( DRIVER_READY_RESPONSE ) == (3 * sizeof( UINT32 )) );

typedef struct _APPROVE_PCI_CONNECTION_COMMAND
{
   // DWORD 0
   UINT32   RequestCode                               : BITFIELD_RANGE( 0, 7 );       // SW_TO_FW_COMMAND_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 31 );

   // DWORD 1 - 4
   UNIQUE_ID   ConnectedEndPointUniqueID;

   // DWORD 5
   UINT32   ConnectionKey                             : BITFIELD_RANGE( 0, 7 );
   UINT32   ConnectionID                              : BITFIELD_RANGE( 8, 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 6
   UINT32   Crc;
} APPROVE_PCI_CONNECTION_COMMAND;

C_ASSERT( sizeof( APPROVE_PCI_CONNECTION_COMMAND ) == (7 * sizeof( UINT32 )) );

typedef struct _APPROVE_PCI_CONNECTION_RESPONSE
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32                                             : BITFIELD_RANGE( 9, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1 - 4
   UNIQUE_ID   ConnectedEndPointUniqueID;

   // DWORD 5
   UINT32   ConnectionKey                             : BITFIELD_RANGE( 0, 7 );
   UINT32   ConnectionID                              : BITFIELD_RANGE( 8, 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 6
   UINT32   Crc;
} APPROVE_PCI_CONNECTION_RESPONSE;

C_ASSERT( sizeof( APPROVE_PCI_CONNECTION_RESPONSE ) == (7 * sizeof( UINT32 )) );

typedef struct _CHALLENGE_PCI_CONNECTION_COMMAND
{
   // DWORD 0
   UINT32   RequestCode                               : BITFIELD_RANGE( 0, 7 );     // SW_TO_FW_COMMAND_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 31 );

   // DWORD 1 - 4
   UNIQUE_ID   ConnectedEndPointUniqueID;

   // DWORD 5
   UINT32   ConnectionKey                             : BITFIELD_RANGE( 0, 7 );
   UINT32   ConnectionID                              : BITFIELD_RANGE( 8, 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 6 - 13
   CHALLENGE   Challenge;

   // DWORD 14
   UINT32   Crc;
} CHALLENGE_PCI_CONNECTION_COMMAND;

C_ASSERT( sizeof( CHALLENGE_PCI_CONNECTION_COMMAND ) == (15 * sizeof( UINT32 )) );

typedef struct _CHALLENGE_PCI_CONNECTION_RESPONSE
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32   NoKeyInDevice                             : BITFIELD_BIT( 9 );
   UINT32                                             : BITFIELD_RANGE( 10, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1 - 4
   UNIQUE_ID   ConnectedEndPointUniqueID;

   // DWORD 5
   UINT32   ConnectionKey                             : BITFIELD_RANGE( 0, 7 );
   UINT32   ConnectionID                              : BITFIELD_RANGE( 8, 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 6 - 13
   CHALLENGE            Challenge;

   // DWORD 14 - 21
   CHALLENGE_RESPONSE   ChallengeResponse;

   // DWORD 22
   UINT32   Crc;
} CHALLENGE_PCI_CONNECTION_RESPONSE;

C_ASSERT( sizeof( CHALLENGE_PCI_CONNECTION_RESPONSE ) == (23 * sizeof( UINT32 )) );

typedef struct _ADD_DEVICE_AND_KEY_COMMAND
{
   // DWORD 0
   UINT32   RequestCode                               : BITFIELD_RANGE( 0, 7 );     // SW_TO_FW_COMMAND_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 31 );

   // DWORD 1 - 4
   UNIQUE_ID   ConnectedEndPointUniqueID;

   // DWORD 5
   UINT32   ConnectionKey                             : BITFIELD_RANGE( 0, 7 );
   UINT32   ConnectionID                              : BITFIELD_RANGE( 8, 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 6 - 13
   DEVICE_KEY   Key;

   // DWORD 14
   UINT32   Crc;
} ADD_DEVICE_AND_KEY_COMMAND;

C_ASSERT( sizeof( ADD_DEVICE_AND_KEY_COMMAND ) == (15 * sizeof( UINT32 )) );

typedef struct _ADD_DEVICE_AND_KEY_RESPONSE
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32                                             : BITFIELD_RANGE( 9, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1 - 4
   UNIQUE_ID   ConnectedEndPointUniqueID;

   // DWORD 5
   UINT32   ConnectionKey                             : BITFIELD_RANGE( 0, 7 );
   UINT32   ConnectionID                              : BITFIELD_RANGE( 8, 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 6
   UINT32   Crc;
} ADD_DEVICE_AND_KEY_RESPONSE;

C_ASSERT( sizeof( ADD_DEVICE_AND_KEY_RESPONSE ) == (7 * sizeof( UINT32 )) );

typedef struct _APPROVE_INTER_DOMAIN_CONNECTION_COMMAND
{
   // DWORD 0
   UINT32   RequestCode                               : BITFIELD_RANGE( 0, 7 );     // SW_TO_FW_COMMAND_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 31 );

   // DWORD 1
   UINT32                                             : BITFIELD_RANGE( 0, 15 );
   UINT32   LocalLink                                 : BITFIELD_RANGE( 16, 18 );
   UINT32                                             : BITFIELD_BIT( 19 );
   UINT32   LocalDepth                                : BITFIELD_RANGE( 20, 23 );
   UINT32                                             : BITFIELD_RANGE( 24, 31 );

   // DWORD 2 - 5
   UNIQUE_ID      ConnectedInterDomainRemoteUniqueID;

   // DWORD 6
   UINT32   TransmitPath                              : BITFIELD_RANGE( 0, 15 );
   UINT32   TransmitRingNumber                        : BITFIELD_RANGE( 16, 31 );

   // DWORD 7
   UINT32   ReceivePath                               : BITFIELD_RANGE( 0, 15 );
   UINT32   ReceiveRingNumber                         : BITFIELD_RANGE( 16, 31 );

   // DWORD 8
   UINT32   Crc;
} APPROVE_INTER_DOMAIN_CONNECTION_COMMAND;

C_ASSERT( sizeof( APPROVE_INTER_DOMAIN_CONNECTION_COMMAND ) == (9 * sizeof( UINT32 )) );

typedef struct _APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32   ErrorFlag                                 : BITFIELD_BIT( 8 );
   UINT32                                             : BITFIELD_RANGE( 9, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32                                             : BITFIELD_RANGE( 0, 15 );
   UINT32   LocalLink                                 : BITFIELD_RANGE( 16, 18 );
   UINT32                                             : BITFIELD_BIT( 19 );
   UINT32   LocalDepth                                : BITFIELD_RANGE( 20, 23 );
   UINT32                                             : BITFIELD_RANGE( 24, 31 );

   // DWORD 2 - 5
   UNIQUE_ID   ConnectedInterDomainRemoteUniqueID;

   // DWORD 6
   UINT32   TransmitPath                              : BITFIELD_RANGE( 0, 15 );
   UINT32   TransmitRingNumber                        : BITFIELD_RANGE( 16, 31 );

   // DWORD 7
   UINT32   ReceivePath                               : BITFIELD_RANGE( 0, 15 );
   UINT32   ReceiveRingNumber                         : BITFIELD_RANGE( 16, 31 );

   // DWORD 8
   UINT32   Crc;
} APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE;

C_ASSERT( sizeof( APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE ) == (9 * sizeof( UINT32 )) );

typedef struct _INTER_DOMAIN_PACKET_SENT_RESPONSE
{
   // DWORD 0
   UINT32   ResponseCode                              : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_RESPONSE_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32   Crc;
} INTER_DOMAIN_PACKET_SENT_RESPONSE;

C_ASSERT( sizeof( INTER_DOMAIN_PACKET_SENT_RESPONSE ) == (2 * sizeof( UINT32 )) );

typedef struct _DEVICE_CONNECTED_NOTIFICATION
{
   // DWORD 0
   UINT32   NotificationCode                          : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_NOTIFICATION_CODE enum
   UINT32   DeviceCertifiedForWindows                 : BITFIELD_BIT( 8 );
   UINT32   DevicePower                               : BITFIELD_RANGE( 9, 10 );    // DEVICE_POWER enum
   UINT32   DeviceSecurityLevel                       : BITFIELD_RANGE( 11, 12 );   // SECURITY_LEVEL enum
   UINT32   DockDevice                                : BITFIELD_BIT( 13 );
   UINT32   FirstDepthDevice                          : BITFIELD_BIT( 14 );
   UINT32                                             : BITFIELD_BIT( 15 );         // used to be Dual Controller Device
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1 - 4
   UNIQUE_ID   ConnectedEndPointUniqueID;

   // DWORD 5
   UINT32   ConnectionKey                             : BITFIELD_RANGE( 0, 7 );
   UINT32   ConnectionID                              : BITFIELD_RANGE( 8, 15 );
   UINT32   Link                                      : BITFIELD_RANGE( 16, 18 );
   UINT32   Device                                    : BITFIELD_BIT( 19 );
   UINT32   Depth                                     : BITFIELD_RANGE( 20, 23 );
   UINT32   Approved                                  : BITFIELD_BIT( 24 );
   UINT32   Rejected                                  : BITFIELD_BIT( 25 );
   UINT32   Boot                                      : BITFIELD_BIT( 26 );
   UINT32   Controller                                : BITFIELD_RANGE( 27, 29 );   // CONTROLLER_TYPE
   UINT32                                             : BITFIELD_RANGE( 30, 31 );

   // DWORD 6 - 37
   END_POINT_NAME   ConnectedEndPointName;

   // DWORD 38
   UINT32   Crc;
} DEVICE_CONNECTED_NOTIFICATION;

C_ASSERT( sizeof( DEVICE_CONNECTED_NOTIFICATION ) == (39 * sizeof( UINT32 )) );

typedef struct _DEVICE_DISCONNECTED_NOTIFICATION
{
   // DWORD 0
   UINT32   NotificationCode                          : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_NOTIFICATION_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32                                             : BITFIELD_RANGE( 0, 15 );
   UINT32   Link                                      : BITFIELD_RANGE( 16, 18 );
   UINT32                                             : BITFIELD_BIT( 19 );
   UINT32   Depth                                     : BITFIELD_RANGE( 20, 23 );
   UINT32                                             : BITFIELD_RANGE( 24, 31 );

   // DWORD 2
   UINT32   Crc;
} DEVICE_DISCONNECTED_NOTIFICATION;

C_ASSERT( sizeof( DEVICE_DISCONNECTED_NOTIFICATION ) == (3 * sizeof( UINT32 )) );

typedef struct _DP_DEVICE_CONNECTED_NOT_TUNNELED_NOTIFICATION
{
   // DWORD 0
   UINT32   NotificationCode                          : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_NOTIFICATION_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32   Crc;
} DP_DEVICE_CONNECTED_NOT_TUNNELED_NOTIFICATION;

C_ASSERT( sizeof( DP_DEVICE_CONNECTED_NOT_TUNNELED_NOTIFICATION ) == (2 * sizeof( UINT32 )) );

typedef struct _INTER_DOMAIN_CONNECTED_NOTIFICATION
{
   // DWORD 0
   UINT32   NotificationCode                          : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_NOTIFICATION_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32                                             : BITFIELD_RANGE( 0, 15 );
   UINT32   Link                                      : BITFIELD_RANGE( 16, 18 );
   UINT32   Approve                                   : BITFIELD_BIT( 19 );
   UINT32   Depth                                     : BITFIELD_RANGE( 20, 23 );
   UINT32                                             : BITFIELD_RANGE( 24, 31 );

   // DWORD 2 - 5
   UNIQUE_ID      RemoteHostRouterUniqueID;

   // DWORD 6 - 9
   UNIQUE_ID      LocalHostRouterUniqueID;

   // DWORD 10 - 11
   ROUTE_STRING   LocalRouteString;

   // DWORD 12 - 13
   ROUTE_STRING   RemoteRouteString;

   // DWORD 14
   UINT32   Crc;
} INTER_DOMAIN_CONNECTED_NOTIFICATION;

C_ASSERT( sizeof( INTER_DOMAIN_CONNECTED_NOTIFICATION ) == (15 * sizeof( UINT32 )) );

typedef struct _INTER_DOMAIN_DISCONNECTED_NOTIFICATION
{
   // DWORD 0
   UINT32   NotificationCode                          : BITFIELD_RANGE( 0, 7 );     // FW_TO_SW_NOTIFICATION_CODE enum
   UINT32                                             : BITFIELD_RANGE( 8, 15 );
   UINT32   PacketID                                  : BITFIELD_RANGE( 16, 23 );
   UINT32   TotalPackets                              : BITFIELD_RANGE( 24, 31 );

   // DWORD 1
   UINT32                                             : BITFIELD_RANGE( 0, 15 );
   UINT32   Link                                      : BITFIELD_RANGE( 16, 18 );
   UINT32                                             : BITFIELD_BIT( 19 );
   UINT32   Depth                                     : BITFIELD_RANGE( 20, 23 );
   UINT32                                             : BITFIELD_RANGE( 24, 31 );

   // DWORD 2 - 5
   UNIQUE_ID   RemoteHostRouterUniqueID;

   // DWORD 6
   UINT32   Crc;
} INTER_DOMAIN_DISCONNECTED_NOTIFICATION;

C_ASSERT( sizeof( INTER_DOMAIN_DISCONNECTED_NOTIFICATION ) == (7 * sizeof( UINT32 )) );

typedef struct _READ_CONFIGURATION_REGISTERS_REQUEST
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;                                                      // ROUTE_STRING_VALUE

   // DWORD 2
   UINT32         DWIndex                             : BITFIELD_RANGE( 0, 12 );
   UINT32         Length                              : BITFIELD_RANGE( 13, 18 );
   UINT32         Port                                : BITFIELD_RANGE( 19, 24 );   // DMA_PORT
   UINT32         ConfigurationSpace                  : BITFIELD_RANGE( 25, 26 );   // CIO_CONFIGURATION_SPACE enum
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3
   UINT32         Crc;
} READ_CONFIGURATION_REGISTERS_REQUEST;

C_ASSERT( sizeof( READ_CONFIGURATION_REGISTERS_REQUEST ) == (4 * sizeof( UINT32 )) );

typedef struct _READ_CONFIGURATION_REGISTERS_RESPONSE
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;                                                      // ROUTE_STRING_VALUE

   // DWORD 2
   UINT32         DWIndex                             : BITFIELD_RANGE( 0, 12 );
   UINT32         Length                              : BITFIELD_RANGE( 13, 18 );
   UINT32         Port                                : BITFIELD_RANGE( 19, 24 );   // DMA_PORT
   UINT32         ConfigurationSpace                  : BITFIELD_RANGE( 25, 26 );   // CIO_CONFIGURATION_SPACE enum
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 -19
   union
   {
      struct
      {
         UINT32   Status                              : BITFIELD_RANGE( 0, 3 );     // MAILBOX_RESPONSE_CODE enum
         UINT32   StatusForCommand                    : BITFIELD_RANGE( 4, 7 );     // MAILBOX_REQUEST_CODE enum
         UINT32                                       : BITFIELD_RANGE( 8, 28 );
         UINT32   StatusResponse                      : BITFIELD_BIT( 29 );
         UINT32   NotificationIndication              : BITFIELD_BIT( 30 );
         UINT32   OperationRequest                    : BITFIELD_BIT( 31 );
         UINT32   Crc;
      } StatusResponse;
      struct
      {
         UINT32   Data[16];
         UINT32   Crc;
      } Mail;
      struct
      {
         UINT32   NotCompleted                        : BITFIELD_BIT( 0 );
         UINT32                                       : BITFIELD_RANGE( 1, 31 );
         UINT32   Crc;
      } Operation;
   } ReadData;
} READ_CONFIGURATION_REGISTERS_RESPONSE;

C_ASSERT( sizeof( READ_CONFIGURATION_REGISTERS_RESPONSE ) == (20 * sizeof( UINT32 )) );

typedef struct _WRITE_CONFIGURATION_REGISTERS_REQUEST
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;                                                      // ROUTE_STRING_VALUE

   // DWORD 2
   UINT32         DWIndex                             : BITFIELD_RANGE( 0, 12 );
   UINT32         Length                              : BITFIELD_RANGE( 13, 18 );
   UINT32         Port                                : BITFIELD_RANGE( 19, 24 );   // DMA_PORT
   UINT32         ConfigurationSpace                  : BITFIELD_RANGE( 25, 26 );   // CIO_CONFIGURATION_SPACE enum
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 -19
   union
   {
      struct
      {
         UINT32   Data[16];
         UINT32   Crc;
      } Mail;
      struct
      {
         UINT32   OperationRequest                    : BITFIELD_BIT( 0 );
         UINT32   CssHeader                           : BITFIELD_BIT( 1 );
         UINT32   FlashAddress                        : BITFIELD_RANGE( 2, 23 );
         UINT32   HighDW                              : BITFIELD_RANGE( 24, 27 );
         UINT32   Command                             : BITFIELD_RANGE( 28, 31 );   // MAILBOX_REQUEST_CODE enum
         UINT32   Crc;
      } NonActiveFlashRegionWrite;
      struct
      {
         UINT32   OperationRequest                    : BITFIELD_BIT( 0 );
         UINT32                                       : BITFIELD_RANGE( 1, 27 );
         UINT32   Command                             : BITFIELD_RANGE( 28, 31 );   // MAILBOX_REQUEST_CODE enum
         UINT32   Crc;
      } FlashUpdateAuthenticate;
      struct
      {
         UINT32   OperationRequest                    : BITFIELD_BIT( 0 );
         UINT32                                       : BITFIELD_BIT( 1 );
         UINT32   FlashAddress                        : BITFIELD_RANGE( 2, 23 );
         UINT32   DWCount                             : BITFIELD_RANGE( 24, 27 );
         UINT32   Command                             : BITFIELD_RANGE( 28, 31 );   // MAILBOX_REQUEST_CODE enum
         UINT32   Crc;
      } FlashRegionRead;
      struct
      {
         UINT32   OperationRequest                    : BITFIELD_BIT( 0 );
         UINT32                                       : BITFIELD_RANGE( 1, 23 );
         UINT32   DWCount                             : BITFIELD_RANGE( 24, 27 );
         UINT32   Command                             : BITFIELD_RANGE( 28, 31 );   // MAILBOX_REQUEST_CODE enum
         UINT32   Crc;
      } ChallengeDevice;
      struct
      {
         UINT32   OperationRequest                    : BITFIELD_BIT( 0 );
         UINT32                                       : BITFIELD_RANGE( 1, 27 );
         UINT32   Command                             : BITFIELD_RANGE( 28, 31 );   // MAILBOX_REQUEST_CODE enum
         UINT32   Crc;
      } PowerCycleRequest;
      struct
      {
         UINT32   OperationRequest                    : BITFIELD_BIT( 0 );
         UINT32                                       : BITFIELD_RANGE( 1, 23 );
         UINT32   DWCount                             : BITFIELD_RANGE( 24, 27 );
         UINT32   Command                             : BITFIELD_RANGE( 28, 31 );   // MAILBOX_REQUEST_CODE enum
         UINT32   Crc;
      } AddDeviceKey;
   } WriteData;
} WRITE_CONFIGURATION_REGISTERS_REQUEST;

C_ASSERT( sizeof( WRITE_CONFIGURATION_REGISTERS_REQUEST ) == (20 * sizeof( UINT32 )) );

typedef struct _WRITE_CONFIGURATION_REGISTERS_RESPONSE
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;                                                      // ROUTE_STRING_VALUE

   // DWORD 2
   UINT32         DWIndex                             : BITFIELD_RANGE( 0, 12 );
   UINT32         Length                              : BITFIELD_RANGE( 13, 18 );
   UINT32         Port                                : BITFIELD_RANGE( 19, 24 );   // DMA_PORT
   UINT32         ConfigurationSpace                  : BITFIELD_RANGE( 25, 26 );   // CIO_CONFIGURATION_SPACE enum
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3
   UINT32         Crc;
} WRITE_CONFIGURATION_REGISTERS_RESPONSE;

C_ASSERT( sizeof( WRITE_CONFIGURATION_REGISTERS_RESPONSE ) == (4 * sizeof( UINT32 )) );

typedef struct _ERROR_NOTIFICATION
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;                                                      // ROUTE_STRING_VALUE

   // DWORD 2
   UINT32         ErrorCode                           : BITFIELD_RANGE( 0, 7 );     // CIO_ERROR_CODE enum
   UINT32         Port                                : BITFIELD_RANGE( 8, 13 );    // DMA_PORT
   UINT32                                             : BITFIELD_RANGE( 14, 31 );

   // DWORD 3
   UINT32         Crc;
} ERROR_NOTIFICATION;

C_ASSERT( sizeof( ERROR_NOTIFICATION ) == (4 * sizeof( UINT32 )) );

typedef struct _XDOMAIN_PROPERTIES_READ_REQUEST
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;

   // DWORD 2
   UINT32         Length                              : BITFIELD_RANGE( 0, 5 );
   UINT32                                             : BITFIELD_RANGE( 6, 26 );
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 - 6
   UNIQUE_ID      XDomainDiscoveryProtocolUUID;                                  // XDOMAIN_DISCOVERY_PROTOCOL_UUID

   // DWORD 7
   XDOMAIN_PACKET_TYPE  PacketType;

   // DWORD 8 - 11
   UNIQUE_ID      SourceUUID;

   // DWORD 12 - 15
   UNIQUE_ID      DestinationUUID;

   // DWORD 16
   UINT32         Offset                              : BITFIELD_RANGE( 0, 15 );
   UINT32                                             : BITFIELD_RANGE( 16, 31 );

   // DWORD 17
   UINT32         Crc;
} XDOMAIN_PROPERTIES_READ_REQUEST;

C_ASSERT( sizeof( XDOMAIN_PROPERTIES_READ_REQUEST ) == (18 * sizeof( UINT32 )) );

typedef struct _XDOMAIN_PROPERTIES_READ_RESPONSE
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;

   // DWORD 2
   UINT32         Length                              : BITFIELD_RANGE( 0, 5 );
   UINT32                                             : BITFIELD_RANGE( 6, 26 );
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 - 6
   UNIQUE_ID      XDomainDiscoveryProtocolUUID;                                  // XDOMAIN_DISCOVERY_PROTOCOL_UUID

   // DWORD 7
   XDOMAIN_PACKET_TYPE  PacketType;

   // DWORD 8 - 11
   UNIQUE_ID      SourceUUID;

   // DWORD 12 - 15
   UNIQUE_ID      DestinationUUID;

   // DWORD 16
   UINT32         Offset                              : BITFIELD_RANGE( 0, 15 );
   UINT32         DataLength                          : BITFIELD_RANGE( 16, 31 );

   // DWORD 17
   UINT32         PropertiesBlockGeneration;

   // DWORD 18 - N
   UINT32         PropertiesBlockData[1];

   // DWORD N+1
   //UINT32         Crc;
} XDOMAIN_PROPERTIES_READ_RESPONSE;

C_ASSERT( sizeof( XDOMAIN_PROPERTIES_READ_RESPONSE ) == (19 * sizeof( UINT32 )) );

typedef struct _XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;

   // DWORD 2
   UINT32         Length                              : BITFIELD_RANGE( 0, 5 );
   UINT32                                             : BITFIELD_RANGE( 6, 26 );
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 - 6
   UNIQUE_ID      XDomainDiscoveryProtocolUUID;                                  // XDOMAIN_DISCOVERY_PROTOCOL_UUID

   // DWORD 7
   XDOMAIN_PACKET_TYPE  PacketType;

   // DWORD 8 - 11
   UNIQUE_ID      SourceUUID;

   // DWORD 12
   UINT32         Crc;
} XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION;

C_ASSERT( sizeof( XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION ) == (13 * sizeof( UINT32 )) );

typedef struct _XDOMAIN_PROPERTIES_CHANGED_RESPONSE
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;

   // DWORD 2
   UINT32         Length                              : BITFIELD_RANGE( 0, 5 );
   UINT32                                             : BITFIELD_RANGE( 6, 26 );
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 - 6
   UNIQUE_ID      XDomainDiscoveryProtocolUUID;                                  // XDOMAIN_DISCOVERY_PROTOCOL_UUID

   // DWORD 7
   XDOMAIN_PACKET_TYPE  PacketType;

   // DWORD 8
   UINT32         Crc;
} XDOMAIN_PROPERTIES_CHANGED_RESPONSE;

C_ASSERT( sizeof( XDOMAIN_PROPERTIES_CHANGED_RESPONSE ) == (9 * sizeof( UINT32 )) );

typedef struct _XDOMAIN_ERROR_RESPONSE
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;

   // DWORD 2
   UINT32         Length                              : BITFIELD_RANGE( 0, 5 );
   UINT32                                             : BITFIELD_RANGE( 6, 26 );
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 - 6
   UNIQUE_ID      XDomainDiscoveryProtocolUUID;                                  // XDOMAIN_DISCOVERY_PROTOCOL_UUID

   // DWORD 7
   XDOMAIN_PACKET_TYPE           PacketType;

   // DWORD 8
   XDOMAIN_ERROR_RESPONSE_CODE   ErrorResponse;

   // DWORD 9
   UINT32         Crc;
} XDOMAIN_ERROR_RESPONSE;

C_ASSERT( sizeof( XDOMAIN_ERROR_RESPONSE ) == (10 * sizeof( UINT32 )) );

typedef struct _THUNDERBOLT_IP_PACKET
{
   // DWORD 0 - 1
   ROUTE_STRING   RouteString;

   // DWORD 2
   UINT32         Length                              : BITFIELD_RANGE( 0, 5 );
   UINT32                                             : BITFIELD_RANGE( 6, 26 );
   UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
   UINT32                                             : BITFIELD_RANGE( 29, 31 );

   // DWORD 3 - 6
   UNIQUE_ID      AppleThunderboltIPProtocolUUID;                             // APPLE_THUNDERBOLT_IP_PROTOCOL_UUID

   // DWORD 7 - 10
   UNIQUE_ID      InitiatorUUID;

   // DWORD 11 - 14
   UNIQUE_ID      TargetUUID;

   // DWORD 15 - N
   UINT32         MessageData[1];

   // DWORD N+1
   //UINT32         Crc;
} THUNDERBOLT_IP_PACKET;

typedef struct _THUNDERBOLT_IP_LOGIN
{
   // DWORD 15
   THUNDERBOLT_IP_PACKET_TYPE PacketType;

   // DWORD 16
   UINT32                     CommandID;

   // DWORD 17
   UINT32                     ProtocolRevision;                            // APPLE_THUNDERBOLT_IP_PROTOCOL_REVISION

   // DWORD 18
   UINT32                     TransmitPath;

   // DWORD 19 - 22
   UINT32                     Reserved[4];
} THUNDERBOLT_IP_LOGIN;

typedef struct _THUNDERBOLT_IP_LOGIN_RESPONSE
{
   // DWORD 15
   THUNDERBOLT_IP_PACKET_TYPE PacketType;

   // DWORD 16
   UINT32                     CommandID;

   // DWORD 17
   UINT32                     Status;

   // DWORD 18 - 19
   UINT32                     ReceiverMACAddress[2];

   // DWORD 20
   UINT32                     ReceiverMACAddressLength;

   // DWORD 21 - 24
   UINT32                     Reserved[4];
} THUNDERBOLT_IP_LOGIN_RESPONSE;

typedef struct _THUNDERBOLT_IP_LOGOUT
{
   // DWORD 15
   THUNDERBOLT_IP_PACKET_TYPE PacketType;

   // DWORD 16
   UINT32                     CommandID;
} THUNDERBOLT_IP_LOGOUT;

typedef struct _THUNDERBOLT_IP_STATUS
{
   // DWORD 15
   THUNDERBOLT_IP_PACKET_TYPE PacketType;

   // DWORD 16
   UINT32                     CommandID;

   // DWORD 17
   UINT32                     Status;
} THUNDERBOLT_IP_STATUS;

#ifdef INTEL_P2P_PROTOCOL_SUPPORTED
   // enum for the bus driver to route the messages to the right destination
   typedef enum _P2P_MSG_DEST
   {
      DEST_P2P_DRIVER,
      DEST_SERVICE
   } P2P_MSG_DEST;

   // enum of the different messages in the P2P Intel protocol
   typedef enum _P2P_MSG_TYPE
   {
      MSG_TYPE_PEER_ATTRIBUTES,
      MSG_TYPE_KEY,
      MSG_TYPE_CHALLENGE,
      MSG_TYPE_CHALLENGE_RESPONSE,
      MSG_TYPE_CONNECT,
      MSG_TYPE_DISCONNECT,
      MSG_TYPE_ACK
   } P2P_MSG_TYPE;

   // common header of all messages in the P2P Intel protocol
   typedef struct _P2P_MSG_HDR
   {
      // DWORD 0 - 1
      ROUTE_STRING   RouteString;

      // DWORD 2
      UINT32         Length                              : BITFIELD_RANGE( 0, 5 );
      UINT32                                             : BITFIELD_RANGE( 6, 26 );
      UINT32         SequenceNumber                      : BITFIELD_RANGE( 27, 28 );
      UINT32                                             : BITFIELD_RANGE( 29, 31 );

      // DWORD 3
      UINT32         Version;

      // DWORD 4
      P2P_MSG_DEST   Dest;

      // DWORD 5
      P2P_MSG_TYPE   MsgType;
   } P2P_MSG_HDR;

   // Acknowledge message that can be used for any P2P_MSG_TYPE
   typedef struct _P2P_MSG_ACK
   {
      // DWORD 0 - 5
      P2P_MSG_HDR    Header;        // MsgType = MSG_TYPE_ACK

      // DWORD 6
      P2P_MSG_TYPE   AckedMsg;

      // DWORD 7
      UINT32         Crc;
   } P2P_MSG_ACK;

   // Attributes of a peer system
   typedef struct _P2P_MSG_PEER_ATTRIBUTES
   {
      // DWORD 0 - 5
      P2P_MSG_HDR Header;           // MsgType = MSG_TYPE_PEER_ATTRIBUTES

      // DWORD 6
      UINT32      ComputerNameLength;

      // DWORD 7
      UINT32      RemoteAttributeRequired                : BITFIELD_BIT( 0 );
      UINT32                                             : BITFIELD_RANGE( 1, 31 );

      // DWORD 8 - N
      UINT8       ComputerName[1];  // Should be DWORD padded

      // DWORD N+1
      //UINT32         Crc;
   } P2P_MSG_PEER_ATTRIBUTES;

   // Security level 2 information
   typedef struct _P2P_MSG_SL2_INFO
   {
      // DWORD 0 - 5
      P2P_MSG_HDR Header;           // MsgType = MSG_TYPE_KEY or MSG_TYPE_CHALLENGE or MSG_TYPE_CHALLENGE_RESPONSE

      // DWORD 6
      TBT_HASH    Hash;

      // DWORD 7
      UINT32      Crc;
   } P2P_MSG_SL2_INFO;

   // Information that needed for establishing path between the 2 peers
   typedef struct _P2P_MSG_CONNECT
   {
      // DWORD 0 - 5
      P2P_MSG_HDR Header;           // MsgType = MSG_TYPE_CONNECT

      // DWORD 6
      UINT32      TransmitPath;

      // DWORD 7 - 8
      UINT8       MacAddress[6];
      UINT8       Reserved[2];

      // DWORD 9
      UINT32      Crc;
   } P2P_MSG_CONNECT;

   // Notification that a peer is going to be disconnected (driver unload, Sx, �)
   typedef struct _P2P_MSG_DISCONNECT
   {
      // DWORD 0 - 5
      P2P_MSG_HDR Header;           // MsgType = MSG_TYPE_DISCONNECT

      // DWORD 6
      UINT32      Crc;
   } P2P_MSG_DISCONNECT;
#endif   // INTEL_P2P_PROTOCOL_SUPPORTED


#endif   //_MESSAGES_H_
