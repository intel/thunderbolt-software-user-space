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
#pragma once
#include <string>
#include <future>
#include "IP2PDevice.h"
#include "Utils.h"
#include "tbtException.h"
#include "UniqueID.h"
#include "XDomainProperties/Properties.h"
#include "ConnectionManager.h"
#include "logger.h"

//max payload size that can be in the properties response message
const uint32_t MAX_PAYLOAD_SIZE = MAX_FW_FRAME_SIZE - sizeof(XDOMAIN_PROPERTIES_READ_RESPONSE);

std::string P2P_STATE_STRING(P2P_STATE e);

/**
 * this class is for holding a P2P device object created when connecting two
 * Thunderbolt hosts (Windows <--> Linux <--> Apple)
 */
class P2PDevice : public IP2PDevice
{
public:
   /**
    * \brief C-tor for initializing P2PDevice properties
    *
    * \param[in]  id                         Controller ID as retrieved from the driver
    * \param[in]  RemoteRouterUniqueID
    * \param[in]  LocalRouteString
    * \param[in]  LocalHostRouterUniqueID
    * \param[in]  LocalHostName
    * \param[in]  depth
    * \param[in]  PathEstablished
    * \param[in]  supportsFullE2E            True if the (local) controller supports full-E2E mode
    *
    * \todo Fill description
    */
   P2PDevice(const controlleriD& id,
             const UniqueID& RemoteRouterUniqueID,
             const ROUTE_STRING& LocalRouteString,
             const UniqueID& LocalHostRouterUniqueID,
             const std::string& LocalHostName,
             uint8_t depth,
             bool PathEstablished,
             bool supportsFullE2E);
	~P2PDevice();

	//Setters & Getters
	uint8_t Depth() const { return m_Depth;}
	const UniqueID& RemoteRouterUniqueID() const { return m_RemoteRouterUniqueID; }
	const ROUTE_STRING& LocalRouteString() const { return m_LocalRouteString; }
	const UniqueID& LocalHostRouterUniqueID() const { return m_LocalHostRouterUniqueID; }
	const controlleriD& ControllerID() const {return m_ControllerID;}
	std::string RemoteHostName() const { return m_RemoteHostName; }
	bool PathEstablished() const { return m_PathEstablished; }
	void SetRemoteHostName(const std::string& hostname){
		TbtServiceLogger::LogInfo("P2P: Remote hostname set to: %s", hostname.c_str());
		m_RemoteHostName = hostname;
	}
	std::string LocalHostName() const{ return m_LocalHostName; }
	void SetLocalHostName(const std::string& hostname) {
		TbtServiceLogger::LogInfo("P2P: Local hostname set to: %s", hostname.c_str());
		m_LocalHostName = hostname;
	}
	const std::atomic_bool& ResetPropertiesRequest() const { return m_ResetPropertiesRequest; }
	void SetResetPropertiesRequest(bool value) {
		if(value)
			TbtServiceLogger::LogInfo("P2P: Properties send request was reset");
		m_ResetPropertiesRequest = value;
	}
	const std::atomic<P2P_STATE>& State() const { return m_State; }
	void SetState(P2P_STATE value) {
		TbtServiceLogger::LogInfo("P2P: State change: %s ===> %s", P2P_STATE_STRING(m_State).c_str() ,P2P_STATE_STRING(value).c_str());
		m_State = value;
		if(m_State == P2P_STATE::TUNNEL && !PathEstablished()) {
			TbtServiceLogger::LogInfo("P2P: path established for %s",RemoteHostName().c_str());
			SetPathEstablished();
		}
	}

	//Methods
	void HandleInterDomainRequest(const std::vector<uint8_t>& Msg);
	void HandleInterDomainResponse(const std::vector<uint8_t>& Msg);
	void SendPropertiesChangeResponse(const XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION& Msg) const;
	void SendReadPropertiesResponse(uint32_t Offset, uint8_t sn) const;
	void StartHandshake();
	void SendPropertiesChangeRequest();
	void OnSystemPreShutdown();

   /**
    * \brief Returns if full-E2E mode should be enabled for this P2P device
    *
    * \return true if full-E2E mode should be enabled, otherwise - false
    */
   bool GetEnableFullE2E() const override;

   /**
   * \brief Returns if P2P should match fragments ID
   *
   * \return true if match fragments ID, otherwise - false
   */
   bool GetMatchFragmentsID() const override;

private:
	const std::atomic_bool& PropertiesChangeActive() const { return m_PropertiesChangeActive; }
	void SetPropertiesChangeActive(bool value) {
		TbtServiceLogger::LogInfo("P2P: Properties change request thread is %s", value ? "active" : "inactive");
		m_PropertiesChangeActive = value;
	}
	const std::atomic_bool& CancelRequests() { return m_CancelRequests; };
	void SetCancelRequests(bool value) {
		TbtServiceLogger::LogInfo("P2P: Cancel P2P Requests was set");
		m_CancelRequests = value;
	};
	//once the path established it remains until Interdomain disconnect message
	void SetPathEstablished(){ m_PathEstablished = true; }
	const XDomainProperties::Properties& Properties() const { return m_LocalHostProperties; }

	/*	This function is to initialize XDomain messages header
	 *	T should be XDomain message
	 */
	template<typename T, XDOMAIN_PACKET_TYPE PT>
	inline void XDomainHeaderIntialization(T& Msg) const
	{
		UNIQUE_ID XDomainProtocolUUIDBuffer = XDOMAIN_DISCOVERY_PROTOCOL_UUID;

		Msg.Length = DwSizeOf<T>() - XDOMAIN_PROTOCOL_OFFSET / sizeof(uint32_t) - 1;
		Msg.PacketType = PT;
		Msg.RouteString = LocalRouteString();
		Msg.SequenceNumber = 0;
		UniqueID(XDomainProtocolUUIDBuffer).ToBuffer(Msg.XDomainDiscoveryProtocolUUID);
	}

   /**
    * \brief Sets flag property for sending to the remote peer
    * 
    * \param[in]  enable  true to tell that flag is supported
    * \param[in]  flag
    */
   void SetFlag(bool enable, uint32_t flag);

	XDOMAIN_PROPERTIES_READ_REQUEST PrepareReadPropertiesRequest(uint32_t Offset);
	std::vector<uint8_t> PrepareReadPropertiesResponseBuffer(uint32_t Offset, uint8_t Sn) const;
	std::shared_ptr<XDomainProperties::Properties> GetRemoteHostProperties();

	const controlleriD m_ControllerID;
	std::string m_LocalHostName;								//local host name
	const uint8_t m_Depth;											//depth of the P2P in a chain of devices
	const ROUTE_STRING m_LocalRouteString;			//local routing string
	const UniqueID m_LocalHostRouterUniqueID;   //local UUID
	const UniqueID m_RemoteRouterUniqueID;			//remote host UUID
	XDomainProperties::Properties m_LocalHostProperties; //the XDomain properties for this host
	std::string m_RemoteHostName;								//remote host name of the other peer ("deviceid" in the XDomain properties structure)
	std::atomic_bool m_CancelRequests;					//this is a mark for sending threads to stop
	std::atomic_bool m_ResetPropertiesRequest;  //mark for properties request thread to reset
	std::atomic_bool m_PropertiesChangeActive;	//mark the state of the properties change thread
	std::atomic<P2P_STATE> m_State;						  //the state of the p2p device { NOT_READY, 	READING_PROPERTIES,	TUNNEL,	NOT_SUPPORTED,	PENDING_TUNNEL}
	bool m_AutenticationSupported;						  //mark if the remote host support authentication
	bool m_PathEstablished;									    //mark if the thunderbolt p2p path is established
	std::promise<std::vector<uint8_t>> m_properties_response;	//this is to pass the response to the async request thread
	std::promise<void> m_properties_change;			//this is to notify the properties change async thread that the request received

   /**
    * \brief true if full-E2E mode should be enabled for this P2P device
    *
    * It means, if the local controller supports it AND we know from the remote XDomainProperties
    * that it supports it, too. So DON'T set it in the c-tor, before you know about the remote properties
    */
   bool m_enableFullE2E = false;

   /**
   * \brief true if P2P should match fragments ID
   */
   bool m_MatchFragmentsID = false;
};
