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
#include <future>
#include <cstring>
#include "P2PDevice.h"
#include "messages.h"

#define XDOMAIN_MAX_RETRIES 60
#define XDOMAIN_TIMEOUT 1

namespace // anon. namespace
{
const std::string network("network");
const std::string protocolSettings("prtcstns");
const uint32_t FULL_E2E_SUPPORT_BIT = 1 << 0;
const uint32_t MATCH_FRAGS_ID_BIT   = 1 << 1;

/**
 * \brief Implementation function for flags functions below
 *
 * \param[in]  prop  XDomainProperties as received from remote peer or as prepared locally
 * \param[in]  flag
 *
 * \returns true if flag key exists and is set in the prop argument, otherwise - false
 */
bool FlagImpl(const XDomainProperties::Properties& prop, uint32_t flag)
{
   if (!prop.Exists(network))
   {
      return false;
   }
   const auto& networkProp = prop[network];
   if (!networkProp.Exists(protocolSettings))
   {
      return false;
   }
   auto val = static_cast<uint32_t>(networkProp[protocolSettings].GetInt32());
   return val & flag ? true : false;
}

/**
 * \brief Helper function to get from XDomainProperties if full-E2E mode is supported or not
 *
 * \param[in]  prop  XDomainProperties as received from remote peer or as prepared locally
 * \param[in]  name  Name of the controller that the properties are belong to, to print in the log message
 *
 * \returns true if full-E2E property exists and is set in the prop argument, otherwise - false
 */
bool SupportsFullE2E(const XDomainProperties::Properties& prop, const std::string& name)
{
   auto ret = FlagImpl(prop, FULL_E2E_SUPPORT_BIT);

   std::string infoStr = "Full E2E support is reported as ";
   if (!ret)
   {
      infoStr += "un";
   }
   infoStr += "supported by XDomainProperties of the " + name + " controller";
   TbtServiceLogger::LogDebug(infoStr.c_str());

   return ret;
}

/**
* \brief Helper function to get from XDomainProperties if match fragements ID or not
*
* \param[in]  prop  XDomainProperties as received from remote peer or as prepared locally
* \param[in]  name  Name of the controller that the properties are belong to, to print in the log message
*
* \returns true if match fragements ID property exists and is set in the prop argument, otherwise - false
*/
bool MatchFragmentsID(const XDomainProperties::Properties& prop, const std::string& name)
{
   auto ret = FlagImpl(prop, MATCH_FRAGS_ID_BIT);

   std::string infoStr = "Match fragments ID is reported as ";
   if (!ret)
   {
      infoStr += "un";
   }
   infoStr += "supported by XDomainProperties of the " + name + " controller";
   TbtServiceLogger::LogDebug(infoStr.c_str());

   return ret;
}

} // anon. namespace

/**	convert the P2P state for string. Helper function for logs
 */
std::string P2P_STATE_STRING(P2P_STATE e)
{
	switch (e)
	{
	case P2P_STATE::NOT_READY: return "NOT_READY";
	case P2P_STATE::READING_PROPERTIES: return "READING_PROPERTIES";
	case P2P_STATE::TUNNEL: return "TUNNEL";
	case P2P_STATE::NOT_SUPPORTED: return "NOT_SUPPORTED";
	case P2P_STATE::PENDING_TUNNEL: return "PENDING_TUNNEL";
	default: throw TbtException("Bad P2P_STATE");
	}
}

/**	the function check if the remote host support Thunderbolt IP
 */
bool CheckSupported(const XDomainProperties::Properties& remoteProp)
{
	return (remoteProp.Exists("network") &&
		remoteProp["network"].Exists("prtcid") && remoteProp["network"]["prtcid"].GetInt32() == 1 &&
		remoteProp["network"].Exists("prtcvers") && remoteProp["network"]["prtcvers"].GetInt32() == 1
		&& remoteProp["network"].Exists("prtcrevs") && remoteProp["network"]["prtcrevs"].GetInt32() == 1);
}

P2PDevice::P2PDevice(const controlleriD& id,
                     const UniqueID& remoteRouterUniqueID,
                     const ROUTE_STRING& localRouteString,
                     const UniqueID& localHostRouterUniqueID,
                     const std::string& localHostName,
                     uint8_t depth,
                     bool PathEstablished,
                     bool supportsFullE2E)
   : m_ControllerID(id),
     m_Depth(depth),
     m_LocalRouteString(localRouteString),
     m_LocalHostRouterUniqueID(localHostRouterUniqueID),
     m_RemoteRouterUniqueID(remoteRouterUniqueID),
     m_State(P2P_STATE::NOT_READY),
     m_PathEstablished(PathEstablished)
{
   m_PropertiesChangeActive = false;
   m_CancelRequests = false;
   m_ResetPropertiesRequest = true;
   SetLocalHostName(localHostName);
   SetFlag(supportsFullE2E, FULL_E2E_SUPPORT_BIT);
   SetFlag(true, MATCH_FRAGS_ID_BIT);
}

/**	the async properties request thread can get various errors from the other peer
 *	in this case an exception will be thrown to properties request async thread and
 *	and will be handle according to the error code. (taking care in the GetRemoteHostProperties function)
 */
class XDomainException : std::logic_error
{
public:
	XDomainException(XDOMAIN_ERROR_RESPONSE_CODE error_code, const char* description = nullptr) :
		std::logic_error(description),
		m_error_code(error_code){}
	XDOMAIN_ERROR_RESPONSE_CODE error_code() const { return m_error_code; }
private:
	const XDOMAIN_ERROR_RESPONSE_CODE m_error_code;	//protocol error code
};

/**	when destroing the P2P device, it will request the sending threads to stop
 *	and wait till they will stop.
 */
P2PDevice::~P2PDevice()
{
	try {
		//in case that read requests or properties change thread are running we need to stop them
		SetCancelRequests(true);
		TbtServiceLogger::LogInfo("P2P: ~P2PDevice: Wait for P2P threads to finish");
		//wait for properties threads to finish before destroying the object
		while (State() == P2P_STATE::READING_PROPERTIES || PropertiesChangeActive()) {
			std::this_thread::yield();
		}
		TbtServiceLogger::LogInfo("P2P: ~P2PDevice: All P2P threads are canceled");
	}
	catch (...) {
		TbtServiceLogger::LogInfo("P2P: ~P2PDevice: exception");
	}
}

/**	handle p2p requests {read request, properties change}
 *	Msg - raw data of the request message
 */
void P2PDevice::HandleInterDomainRequest(const std::vector<uint8_t>& Msg)
{
	//protocol ID as defined in Apple spec
	UNIQUE_ID XDomainDiscoveryProtocolUuidBuffer = XDOMAIN_DISCOVERY_PROTOCOL_UUID;

	if (UniqueID(XDomainDiscoveryProtocolUuidBuffer) == UniqueID(*(UNIQUE_ID*) (&Msg.front() + XDOMAIN_PROTOCOL_OFFSET)))
	{
		auto PacketType = *(XDOMAIN_PACKET_TYPE*) (&Msg.front() + XDOMAIN_PACKET_TYPE_OFFSET);
		switch (PacketType)
		{
		case XDOMAIN_PROPERTIES_READ_REQUEST_TYPE:
		{
			TbtServiceLogger::LogInfo("P2P: Received properties read request");
			//first sending the response for the request
			auto pXDomainReadRequest = GetBufStruct<XDOMAIN_PROPERTIES_READ_REQUEST>(Msg);
			SendReadPropertiesResponse(pXDomainReadRequest->Offset, pXDomainReadRequest->SequenceNumber);
			//this is for case of when the remote peer was not responsive (NOT_READY)
			//or Thunderbolt IP not supported. sending us request tell us it is now responsive
			//and maybe Thunderbolt IP supported
			if (State() == P2P_STATE::NOT_READY || State() == P2P_STATE::NOT_SUPPORTED) {
				StartHandshake();
			}
			break;
		}
		case XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION_TYPE:
		{
			//first sending response for the request
			TbtServiceLogger::LogInfo("P2P: Received properties change request");
			auto pXDomainPropChangedNotification = GetBufStruct<XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION>(Msg);
			SendPropertiesChangeResponse(*pXDomainPropChangedNotification);
			//if the device is in reading mode, then we need to reset the request,
			//because the remote properties might have been changed
			if (State() == P2P_STATE::READING_PROPERTIES) {
				SetResetPropertiesRequest(true);
			}
			else {	//device properties change, we need to read again
				StartHandshake();
			}
			break;
		}
		default:
			TbtServiceLogger::LogWarning("Warning: Received an unknown packet type from inter domain (protocol: XDOMAIN) : %u", (uint32_t) PacketType);
		}
	}
	else
	{
		TbtServiceLogger::LogInfo("P2P: Request received with an unknown protocol");
	}
}

/**	handle p2p response { properties response , properties change response , error }
 *	Msg - raw data of the request message
 */
void P2PDevice::HandleInterDomainResponse(const std::vector<uint8_t>& Msg)
{
	//protocol ID as defined in Apple spec
	UNIQUE_ID XDomainDiscoveryProtocolUuidBuffer = XDOMAIN_DISCOVERY_PROTOCOL_UUID;

	if (UniqueID(XDomainDiscoveryProtocolUuidBuffer) == UniqueID(*(UNIQUE_ID*) (&Msg.front() + XDOMAIN_PROTOCOL_OFFSET)))
	{
		auto PacketType = *(XDOMAIN_PACKET_TYPE*) (&Msg.front() + XDOMAIN_PACKET_TYPE_OFFSET);

		switch (PacketType)
		{
		case XDOMAIN_PROPERTIES_READ_RESPONSE_TYPE:
		{
			try {
				TbtServiceLogger::LogInfo("P2P: Received properties read response");
				//this is for a corner case that after the last retry of properties request the response arrived,
				//p2p remote host is probably now responsive and thus requesting again
				if (State() == P2P_STATE::NOT_READY || State() == P2P_STATE::NOT_SUPPORTED ) {
					StartHandshake();
				}
				else if (State() == P2P_STATE::READING_PROPERTIES) {
					//in case that the service is in reading mode, we pass the request to the reading thread
					//using the promise/future mechanism
					m_properties_response.set_value(Msg);
				}
			}
			catch (const std::future_error& e) {
				//if the thread finish to read and still one response will arrive an exception will be thrown,
				//this is fine, just ignore it
				TbtServiceLogger::LogWarning("Warning: HandleInterDomainResponse: Caught a future_error with code %u : %s", e.code(), e.what());
			}
			break;
		}
		case XDOMAIN_PROPERTIES_CHANGED_RESPONSE_TYPE:
		{
			try{
				TbtServiceLogger::LogInfo("P2P: Received properties change response");
				//this will unblock the properties change sending thread, and make it
				//stop sending requests.
				m_properties_change.set_value();
			}
			catch (const std::future_error& e) {
				TbtServiceLogger::LogWarning("Warning: P2P: XDOMAIN_PROPERTIES_CHANGED_RESPONSE_TYPE , code %u : %s", e.code(), e.what());
			}
			break;
		}
		case XDOMAIN_ERROR_RESPONSE_TYPE:
		{
			TbtServiceLogger::LogInfo("P2P: Received XDomain error response");
			auto pXDomainReadResponse = GetBufStruct<XDOMAIN_ERROR_RESPONSE>(Msg);
			//in the case of an error with properties read request we will get an error message
			//this will put an exception with error code in the request that will
			//thrown when trying to get the result
			m_properties_response.set_exception(std::make_exception_ptr(XDomainException(pXDomainReadResponse->ErrorResponse)));
			break;
		}
		default:
			TbtServiceLogger::LogWarning("Warning: Received an unknown packet type from inter domain (protocol: XDOMAIN)");
		}
	}
	else
	{
		TbtServiceLogger::LogInfo("P2P: Request received with an unknown protocol");
	}
}

/**	this function is to prepare the properties read request for this p2p device
 *	Offset - XDomain properties can be bigger then the maximum payload size,
 *					 and will be sent in chunks, this is the requested offset in DW's
 *					 of the properties.
 */
XDOMAIN_PROPERTIES_READ_REQUEST P2PDevice::PrepareReadPropertiesRequest(uint32_t Offset)
{
	UNIQUE_ID XDomainProtocolUUIDBuffer = XDOMAIN_DISCOVERY_PROTOCOL_UUID;
	XDOMAIN_PROPERTIES_READ_REQUEST Request;
	std::memset(&Request, 0, sizeof(Request));
	Request.RouteString = LocalRouteString();
	Request.SequenceNumber = 0;
	Request.Length = DwSizeOf<XDOMAIN_PROPERTIES_READ_REQUEST>() - XDOMAIN_PROTOCOL_OFFSET / sizeof(uint32_t) - 1;
	UniqueID(XDomainProtocolUUIDBuffer).ToBuffer(Request.XDomainDiscoveryProtocolUUID);
	Request.PacketType = XDOMAIN_PACKET_TYPE::XDOMAIN_PROPERTIES_READ_REQUEST_TYPE;
	LocalHostRouterUniqueID().ToBuffer(Request.SourceUUID);
	RemoteRouterUniqueID().ToBuffer(Request.DestinationUUID);
	Request.Offset = Offset;
	return Request;
}

/**	sending properties change response to the corresponding request
 *	Msg - the original request
 *	Note: response should be replied with same sequence number
 */
void P2PDevice::SendPropertiesChangeResponse(const XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION& Msg) const
{
	TbtServiceLogger::LogInfo("P2P: Sending properties change response. Controller: %s", WStringToString(m_ControllerID).c_str());
	XDOMAIN_PROPERTIES_CHANGED_RESPONSE response;
	std::memset(&response, 0, sizeof(response));
	XDomainHeaderIntialization<XDOMAIN_PROPERTIES_CHANGED_RESPONSE, XDOMAIN_PROPERTIES_CHANGED_RESPONSE_TYPE>(response);
	response.SequenceNumber = Msg.SequenceNumber;

	ConnectionManager::GetInstance()->GetControllerCommandSender().SendTbtWmiMessageToFW(m_ControllerID, (uint8_t*) &response, sizeof(response), PDF_INTER_DOMAIN_RESPONSE);
}

/**	sending properties response with the requested offset
 *	Offset - requested offset in DW's of the properties
 *	sn - the request sequence number, need to be replied with same sn
 *	Note: response should be replied with same sequence number
 */
void P2PDevice::SendReadPropertiesResponse(uint32_t Offset, uint8_t sn) const
{
	try
	{
		auto buffer = PrepareReadPropertiesResponseBuffer(Offset, sn);
		XDOMAIN_PROPERTIES_READ_RESPONSE *XDomainResponse = (XDOMAIN_PROPERTIES_READ_RESPONSE*) &buffer.front();
		TbtServiceLogger::LogInfo("P2P: Sending Response with offset %u through controller %s", Offset, WStringToString(m_ControllerID).c_str());

		ConnectionManager::GetInstance()->GetControllerCommandSender().SendTbtWmiMessageToFW(m_ControllerID, (PUCHAR) XDomainResponse, buffer.size(), PDF_INTER_DOMAIN_RESPONSE);
	}
	catch (const std::exception& ex)
	{
		TbtServiceLogger::LogError("Error: SendReadPropertiesResponse failed (Exception: %s)", ex.what());
		throw;
	}
	catch (...)
	{
		TbtServiceLogger::LogError("Error: SendReadPropertiesResponse failed");
		throw;
	}
}

/**	preparing the response message with the requested properties payload
 *	Offset - requested offset in DW's of the poperites
 *	sn - the request sequence number, need to be replied with same sn
 */
std::vector<uint8_t> P2PDevice::PrepareReadPropertiesResponseBuffer(uint32_t Offset, uint8_t Sn) const
{
	auto properties = Properties();
	properties["deviceid"] = m_LocalHostName;
	auto requstOffsetInBytes = Offset * sizeof(uint32_t);
	std::vector<uint8_t> FullPropertiesBuffer = properties.ToBuffer();
	uint32_t currentBuffSizeInBytes = FullPropertiesBuffer.size() - requstOffsetInBytes;
	currentBuffSizeInBytes = (currentBuffSizeInBytes > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : currentBuffSizeInBytes;
	std::vector<uint8_t> Buffer(currentBuffSizeInBytes + sizeof(XDOMAIN_PROPERTIES_READ_RESPONSE));
	std::copy(FullPropertiesBuffer.begin() + requstOffsetInBytes, FullPropertiesBuffer.begin() + requstOffsetInBytes + currentBuffSizeInBytes, Buffer.begin() + offsetof(XDOMAIN_PROPERTIES_READ_RESPONSE, PropertiesBlockData));
	XDOMAIN_PROPERTIES_READ_RESPONSE *XDomainResponse = (XDOMAIN_PROPERTIES_READ_RESPONSE*) &Buffer.front();
	XDomainResponse->RouteString = LocalRouteString();
	XDomainResponse->SequenceNumber = Sn;
	XDomainResponse->Length = (currentBuffSizeInBytes + sizeof(XDOMAIN_PROPERTIES_READ_RESPONSE) - XDOMAIN_PROTOCOL_OFFSET) / sizeof(uint32_t) - 1; // -1 ==> because of CRC
	UNIQUE_ID protocolUUID = XDOMAIN_DISCOVERY_PROTOCOL_UUID;
	UniqueID(protocolUUID).ToBuffer(XDomainResponse->XDomainDiscoveryProtocolUUID);
	XDomainResponse->PacketType = XDOMAIN_PACKET_TYPE::XDOMAIN_PROPERTIES_READ_RESPONSE_TYPE;
	LocalHostRouterUniqueID().ToBuffer(XDomainResponse->SourceUUID);
	RemoteRouterUniqueID().ToBuffer(XDomainResponse->DestinationUUID);
	XDomainResponse->Offset = Offset;
	XDomainResponse->DataLength = properties.GetSize(); //in DW
	XDomainResponse->PropertiesBlockGeneration = properties.GetBlockGeneration();
	return Buffer;
}

/**	this is function is starting a thread that will request the properties, parse
 *	the result and will perform the right action.
 */
void P2PDevice::StartHandshake()
{
	SetState(P2P_STATE::READING_PROPERTIES);
	std::thread([this](){
		try {
			//this will open another async thread that will get the properties using
			//the GetRemoteHostProperties method
			auto result = std::async(std::launch::async, &P2PDevice::GetRemoteHostProperties,this);
			//waiting for the properties object to return from GetRemoteHostProperties
			//in case of an protocol error an exception will be thrown
			auto pRemoteProp = result.get();

			//the GetRemoteHostProperties didn't managed to get any properties
			//change the state to not ready
			if (pRemoteProp == nullptr || CancelRequests()) {
				SetState(P2P_STATE::NOT_READY);
				return;
			}

			const auto remote_properties = *pRemoteProp;

			//check if the remote host support Thunderbolt IP
			if (CheckSupported(remote_properties)) {
				//TODO: verify that the computer name support UTF* characters
				//			Chinese, Hebrew, etc.
				std::string computerNameUTF8 = remote_properties["deviceid"];

				SetRemoteHostName(computerNameUTF8);

				//network path is already established
				if(PathEstablished()) {
					SetState(P2P_STATE::TUNNEL);
				}
				else
				{
               auto remote = SupportsFullE2E(remote_properties, "remote");
               auto local = SupportsFullE2E(m_LocalHostProperties, "local");
               m_enableFullE2E = remote && local;
               remote = MatchFragmentsID(remote_properties, "remote");
               local = MatchFragmentsID(m_LocalHostProperties, "local");
               m_MatchFragmentsID = remote && local;
					//set the state to pending, until we will get approval that the p2p adapter is up
					SetState(P2P_STATE::PENDING_TUNNEL);
					ConnectionManager::GetInstance()->GetControllerCommandSender().SendTbtWmiApproveP2P(*this);
				}
				//TODO: REMOVE!!!! - state change to tunnel for debug purpose need to be updated after driver response

			}
			else {
				SetState(P2P_STATE::NOT_SUPPORTED); // properties arrived but Thunderbolt IP is not supported
			}
		}
		catch (const std::exception& e) {
			//in any case of exception, protocol exception or std::exception,
			//state is changed to not ready. and will be recoverable in any case
			TbtServiceLogger::LogError("Error: StartHandshake failed ( Exception: %s )", e.what());
			SetState(P2P_STATE::NOT_READY);
		}
	}).detach();
}

/**	this is function is starting a thread send properties request with retries
 *  will stop when first response arrived or if has canceled request (cable disconnected)
 */
void P2PDevice::SendPropertiesChangeRequest()
{
	SetPropertiesChangeActive(true);
	std::thread([this](){
		try
		{
			//preparing the notification request
            auto msg = emptyInit<XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION>();
			XDomainHeaderIntialization<XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION, XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION_TYPE>(msg);
			LocalHostRouterUniqueID().ToBuffer(msg.SourceUUID);

			for (int i = 0; i < XDOMAIN_MAX_RETRIES; i++)
			{
				if (CancelRequests())
				{
					TbtServiceLogger::LogInfo("P2P: Cancel properties change has requested, stopping request thread");
					break;
				}
				TbtServiceLogger::LogInfo("P2P: Sending properties change notification to remote host: Host name: %s, Attempt: %u", RemoteHostName().c_str(),i);
				m_properties_change = std::promise<void>();
				auto result = m_properties_change.get_future();

				ConnectionManager::GetInstance()->GetControllerCommandSender().SendTbtWmiMessageToFW(m_ControllerID, (PUCHAR) &msg, sizeof(msg), PDF_INTER_DOMAIN_REQUEST);
				//waiting for the request to arrive with timeout, if arrived before time out then quit
				if (result.wait_for(std::chrono::seconds(XDOMAIN_TIMEOUT)) == std::future_status::ready) {
					TbtServiceLogger::LogInfo("P2P: Host reply for properties change notification, Host name: %s, Attempt: %u", RemoteHostName().c_str(), i);
					break;
				}
			}
		}
		catch (const std::exception& e) {
			TbtServiceLogger::LogError("Error: Exception: %s", e.what());
		}
		catch (...) {
			TbtServiceLogger::LogError("Error: Unknown error");
		}
		//when exit mark this thread as not active
		SetPropertiesChangeActive(false);
	}).detach();
}


/**
 * this function is sending properties change just before the daemon is shutting down
 * FW is responsible to answer XDomain properties requests after that
 */
void P2PDevice::OnSystemPreShutdown()
{
	TbtServiceLogger::LogInfo("P2P: OnSystemPreShutdown");
    auto msg = emptyInit<XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION>();
	XDomainHeaderIntialization<XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION, XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION_TYPE>(msg);
	LocalHostRouterUniqueID().ToBuffer(msg.SourceUUID);

	ConnectionManager::GetInstance()->GetControllerCommandSender().SendTbtWmiMessageToFW(m_ControllerID, (PUCHAR) &msg, sizeof(msg), PDF_INTER_DOMAIN_REQUEST);
}

/**	this function is requesting the remote host properties.
 *	requests chuncks and assemblies to complete properties object.
 *	Return - remote host properties
 *	Exceptions - in case of unrecoverable protocol error
 */
std::shared_ptr<XDomainProperties::Properties> P2PDevice::GetRemoteHostProperties()
{
	try
	{
		//preparing base packet for the request, will be updated and resent for all
		//the properties chunks
		XDOMAIN_PROPERTIES_READ_REQUEST Request = PrepareReadPropertiesRequest(0);
		std::vector<uint8_t> properties_buffer;		//accumulated buffer for the properties
		//properties object has a generation (version), each message contain the properties
		//generation, in case that the generation changed the chunk can not be synced and needs to request again
        uint32_t buffer_generation = 0;

		for (int i = 0; i < XDOMAIN_MAX_RETRIES; i++) {
			//this is reset callback, define in lambda to avoid code duplication
			auto reset_request = [&]() {
				SetResetPropertiesRequest(false);
				TbtServiceLogger::LogInfo("P2P: reset properties request, Controller %s", WStringToString(m_ControllerID).c_str());
				properties_buffer.clear();
				i = 0;
			};

			//in case of cable disconnect
			if (CancelRequests()) {
				TbtServiceLogger::LogInfo("P2P: request thread canceled, Controller %s", WStringToString(m_ControllerID).c_str());
				return nullptr;
			}
			//reset request has been set due to properties change request
			if (ResetPropertiesRequest()) {
				reset_request();
				continue;
			}
			//update the requested offset to be from the end of the buffer (the next chunk)
			Request.Offset = properties_buffer.size() / sizeof(uint32_t);
			TbtServiceLogger::LogInfo("P2P: Sending request for XDomainProperties: Controller %s, DW offset: %u Attempt: %u", WStringToString(m_ControllerID).c_str(), Request.Offset,i);
			//reset the promise so we will be able to get another response
			m_properties_response = std::promise<std::vector<uint8_t>>();
			auto response = m_properties_response.get_future();

			ConnectionManager::GetInstance()->GetControllerCommandSender().SendTbtWmiMessageToFW(m_ControllerID, (uint8_t*) &Request, sizeof(XDOMAIN_PROPERTIES_READ_REQUEST), PDF_INTER_DOMAIN_REQUEST);
			//waiting for response, in a case of timeout continue to another retry
			auto status = response.wait_for(std::chrono::seconds(XDOMAIN_TIMEOUT));
			if (status == std::future_status::timeout)
				continue;

			std::vector<uint8_t> chunk;
			try {
				//get the chunk or throw exception in case of protocol error
				chunk = response.get();
			}
			catch (const XDomainException& e) {
				switch (e.error_code()) {
				//according to Apple in this case we need to continue retries
				case XDOMAIN_ERROR_RESPONSE_CODE::XDOMAIN_NOT_READY:
				case XDOMAIN_ERROR_RESPONSE_CODE::XDOMAIN_SUCCESS:
					continue;
				//these are critical errors, not recoverable and should stop the retries
				//throwing exception that will be handled in calling thread via future
				case XDOMAIN_ERROR_RESPONSE_CODE::XDOMAIN_NOT_SUPPORTED:
				case XDOMAIN_ERROR_RESPONSE_CODE::XDOMAIN_UNKNOWN_DOMAIN:
				case XDOMAIN_ERROR_RESPONSE_CODE::XDOMAIN_UNKNOWN_PACKET_TYPE:
				default:
					throw TbtException("XDOMAIN_ERROR_RESPONSE_CODE (XDOMAIN_NOT_SUPPORTED |  XDOMAIN_UNKNOWN_DOMAIN | XDOMAIN_UNKNOWN_PACKET_TYPE)");
					break;
				}
			}
			//extracting the response from the buffer for parsing
			auto pXDomainReadResponse = GetBufStruct<XDOMAIN_PROPERTIES_READ_RESPONSE>(chunk);

			//not the first chunk and the properties generation changed. need to request from start
			if (pXDomainReadResponse->Offset != 0 && buffer_generation != pXDomainReadResponse->PropertiesBlockGeneration) {
				reset_request();
				continue;
			}
			//previous request is just now arrived, irrelevant now, skipping
			if (pXDomainReadResponse->Offset*sizeof(uint32_t) != properties_buffer.size()) {
				TbtServiceLogger::LogInfo("P2P: skipping chunk with DW offset %u , already arrived", pXDomainReadResponse->Offset);
				continue;
			}

			//first part of the properties arrived, setting the buffer generation and clear the buffer
			if (pXDomainReadResponse->Offset == 0) 	{
				buffer_generation = pXDomainReadResponse->PropertiesBlockGeneration;
				properties_buffer.clear();
			}

			//pushing the chunk to buffer end
			size_t buffSize = pXDomainReadResponse->Length*sizeof(uint32_t) + XDOMAIN_PROTOCOL_OFFSET - offsetof(XDOMAIN_PROPERTIES_READ_RESPONSE, PropertiesBlockData);
			properties_buffer.insert(properties_buffer.end(), (uint8_t*) pXDomainReadResponse->PropertiesBlockData, (uint8_t*) pXDomainReadResponse->PropertiesBlockData + buffSize);
			i = 0; //reset retries for next chunk
			TbtServiceLogger::LogInfo("P2P: Received DW offset %u with size %u DW's out of total %u DW's ", pXDomainReadResponse->Offset, buffSize / sizeof(uint32_t), pXDomainReadResponse->DataLength);
			//buffer size is the same like mention in the properties response packet
			//means we got all the chunks and can build the properties object
			if (properties_buffer.size() == pXDomainReadResponse->DataLength*sizeof(uint32_t)) {
				TbtServiceLogger::LogInfo("P2P: All chunks of XDomainProperties arrived. total: %u", properties_buffer.size() / sizeof(uint32_t));
				return std::make_shared<XDomainProperties::Properties>(properties_buffer);
			}
		}
		//Time out here
		return nullptr;
	}
	catch (const std::exception& e) {
		TbtServiceLogger::LogError("Error: GetRemoteHostProperties fail retrieving remote host properties. Exception : %s", e.what());
		throw;
	}
	catch (...) {
		TbtServiceLogger::LogError("Error: P2P: fail retrieving remote host properties.");
		throw;
	}
}

void P2PDevice::SetFlag(bool enable, uint32_t flag)
{
   auto value = static_cast<uint32_t>(m_LocalHostProperties[network][protocolSettings].GetInt32());
   if (enable)
   {
      value |= flag;
   }
   else
   {
      value &= ~flag;
   }
   m_LocalHostProperties[network][protocolSettings] = value;
}

bool P2PDevice::GetEnableFullE2E() const
{
   return m_enableFullE2E;
}

bool P2PDevice::GetMatchFragmentsID() const
{
   return m_MatchFragmentsID;
}
