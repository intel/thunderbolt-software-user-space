/*******************************************************************************
 *
 * Intel Thunderbolt(TM) daemon
 * Copyright(c) 2014 - 2016 Intel Corporation.
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


#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <vector>
#include <mutex>
#include <chrono>
#include <errno.h>
#include <cstring>
#include <endian.h>
#include <algorithm>
#include <mutex>
#include "GenlWrapper.h"
#include "logger.h"
#include "tbtException.h"
#include "MessagesWrapper.h"
#include "GenetlinkInterface.h"
#include "Utils.h"

#define NETLINK_ACK_TIME_OUT 100

//logger mutex for safe threads logging
std::mutex TbtServiceLogger::log_mutex;

//mutex for sync receive/send messages
static std::mutex _receive_mutex;
//condition variable for syncing send/receive threads
static std::condition_variable _send_receice_cv;
//store remote exceptions in async messages
static std::exception_ptr _remote_exception = nullptr;

/**
 * Netlink policy, describe attributes types for driver/daemon messages
 */
static struct nla_policy nhi_gnl_policy[NHI_ATTR_MAX + 1] = {
    emptyInit<nla_policy>(),
    [NHI_ATTR_DRIVER_VERSION]     = { .type = NLA_STRING, .minlen=0, .maxlen = 0},
    [NHI_ATTR_NVM_VER_OFFSET]     = { .type = NLA_U16,    .minlen=0, .maxlen = 0},
    [NHI_ATTR_NUM_PORTS]          = { .type = NLA_U8,     .minlen=0, .maxlen = 0},
    [NHI_ATTR_DMA_PORT]           = { .type = NLA_U8,     .minlen=0, .maxlen = 0},
    [NHI_ATTR_SUPPORT_FULL_E2E]   = { .type = NLA_FLAG,   .minlen=0, .maxlen = 0},
    [NHI_ATTR_MAILBOX_CMD]        = { .type = NLA_U32,    .minlen=0, .maxlen = 0},
    [NHI_ATTR_PDF]                = { .type = NLA_U32,    .minlen=0, .maxlen = 0},
    [NHI_ATTR_MSG_TO_ICM]         = { .type = NLA_UNSPEC, .minlen=0, .maxlen = MAX_FW_FRAME_SIZE},
    [NHI_ATTR_MSG_FROM_ICM]       = { .type = NLA_UNSPEC, .minlen=0, .maxlen = MAX_FW_FRAME_SIZE},
    [NHI_ATTR_LOCAL_ROUTE_STRING] = { .type = NLA_UNSPEC, .minlen=0, .maxlen = sizeof(ROUTE_STRING)},
    [NHI_ATTR_LOCAL_UNIQUE_ID]    = { .type = NLA_UNSPEC, .minlen=0, .maxlen = sizeof(UNIQUE_ID)},
    [NHI_ATTR_REMOTE_UNIQUE_ID]   = { .type = NLA_UNSPEC, .minlen=0, .maxlen = sizeof(UNIQUE_ID)},
    [NHI_ATTR_LOCAL_DEPTH]        = { .type = NLA_U8,     .minlen=0, .maxlen = 0},
    [NHI_ATTR_ENABLE_FULL_E2E]    = { .type = NLA_FLAG,   .minlen=0, .maxlen = 0},
    [NHI_ATTR_MATCH_FRAME_ID]     = { .type = NLA_FLAG,   .minlen=0, .maxlen = 0},
};

//ISerializable attribute to generic netlink attributes map converter
static std::map<std::string,NHI_GENL_ATTR> netlink_properties_converter = {
		{"InstanceName",NHI_ATTR_UNSPEC},
		{"PeerOS",NHI_ATTR_UNSPEC},
		//NHI_CMD_APPROVE_TBT_NETWORKING
		{"LocalRouteString",NHI_ATTR_LOCAL_ROUTE_STRING},
		{"LocalUniqueID",NHI_ATTR_LOCAL_UNIQUE_ID},
		{"RemoteUniqueID",NHI_ATTR_REMOTE_UNIQUE_ID},
		{"LocalDepth",NHI_ATTR_LOCAL_DEPTH},
		{"EnableFullE2E",NHI_ATTR_ENABLE_FULL_E2E},
		{"MatchFragmentsID",NHI_ATTR_MATCH_FRAME_ID},

		{"Command",NHI_ATTR_MAILBOX_CMD},

		//NHI_CMD_MSG_TO_ICM
		{"PDF",NHI_ATTR_PDF},
		{"Message",NHI_ATTR_MSG_TO_ICM}
};

/**
 * singleton get instance function
 *
 */
GenlWrapper& GenlWrapper::Instance() {
	static GenlWrapper _instance;
	return _instance;
}

/**
 * this function return safe allocated netlink message that is free when
 * has no reference
 */
netlink_mgs_ptr GenlWrapper::alloc_message(uint32_t controller_id, NHI_GENL_CMD cmd_type)
{
	std::unique_ptr<struct nl_msg,netlink_message_deleter> msg(nlmsg_alloc(),[](struct nl_msg * m){
		TbtServiceLogger::LogDebug("netlink message was free");
		nlmsg_free(m);
	});

	auto hdr = genlmsg_put(   msg.get(),
	                            NL_AUTO_PORT,
	                            NL_AUTO_SEQ,
	                            m_family_id,
	                            NHI_GENL_USER_HEADER_SIZE,
	                      			0,
	                            cmd_type,
	                            NHI_GENL_VERSION);
	if (hdr == nullptr) {
		TbtServiceLogger::LogError("Error: genlmsg_put failed");
		throw TbtException("genlmsg_put failed");
	}
	*reinterpret_cast<uint32_t*>(hdr) = controller_id;
	TbtServiceLogger::LogDebug("netlink message was allocated");
	return std::move(msg);
}

/**
 * subscribe to Thunderbolt driver,
 * after subscribing daemon will start getting messages from FW
 */
void GenlWrapper::driver_subscribe() {

	try {
		TbtServiceLogger::LogInfo("subscribing driver");
		send_message_sync(0,NHI_CMD_SUBSCRIBE);
	}
	catch(const std::exception& e) 	{
		TbtServiceLogger::LogError("Error: driver_subscribe failed");
		throw;
	}
}

/**
 * unsubscribe driver function, letting the driver know the daemon is down
 */
void GenlWrapper::driver_unsubscribe() {
	TbtServiceLogger::LogInfo("unsubscribing driver");
	send_message_async(0,NHI_CMD_UNSUBSCRIBE);
}

/**
 * initialization
 * NOTE: the constructor will fail if it is unable to register and communicate
 * with the driver. should happen only if Thunderbolt module is not loaded
 */
GenlWrapper::GenlWrapper(): m_family_id(0),
                            m_genl_socket(nullptr),
                            m_events_callback(nullptr),
                            m_shutting_down(false) {
	try {
		m_genl_socket = nl_socket_alloc();

		if(m_genl_socket == nullptr) {
			throw TbtException("Can't allocate netlink socket");
		}

		// disable sequence number check, required because of driver events can be
    	// send in a middle of request/request ack
		nl_socket_disable_seq_check(m_genl_socket);

		if(genl_connect(m_genl_socket)) {
			throw TbtException("unable to connect generic netlink");
		}

		//get netlink family ID associated with NHI_GENL_NAME,
		//the ID is a reference for the remote netlink server
		m_family_id  = genl_ctrl_resolve(m_genl_socket, NHI_GENL_NAME);

        if(m_family_id < 0) {
			throw TbtException("can't extract family id, is driver installed and loaded?");
		}

		//registering callbacks
		register_netlink_callbacks();

		//start messages receiver thread
		std::thread([this](){receive_thread();}).detach();

		//subscribe driver
		driver_subscribe();
	}
	catch(const std::exception& e)
	{
		TbtServiceLogger::LogError("Error: %s",e.what());
		if(m_genl_socket != nullptr) {
			nl_socket_free(m_genl_socket);
		}
		throw;
	}
}

/**
 * registering generic netlink callbacks for receive, errors, and acks
 */
void GenlWrapper::register_netlink_callbacks() {

	if(nl_socket_modify_cb(m_genl_socket, NL_CB_VALID, NL_CB_CUSTOM,cb_valid, NULL)) {
		TbtServiceLogger::LogError("Error: failed register netlink callback NL_CB_VALID");
		throw TbtException("failed register netlink callback NL_CB_VALID");
	}
	if(nl_socket_modify_cb(m_genl_socket, NL_CB_ACK, NL_CB_CUSTOM,cb_ack, NULL)) {
		TbtServiceLogger::LogError("Error: failed register netlink callback NL_CB_ACK");
		throw TbtException("failed register netlink callback NL_CB_ACK");
	}
	if(nl_socket_modify_err_cb(m_genl_socket, NL_CB_CUSTOM,cb_error_handler, NULL)) {
		TbtServiceLogger::LogError("Error: failed register netlink error callback");
		throw TbtException("failed register netlink error callback");
	}
}

/**
 * registering receive event callback that will be called for each received
 * message from driver
 */
void GenlWrapper::register_events_callback(events_callback func) {
	TbtServiceLogger::LogInfo("GenlWrapper::register_events_callback entry");
	m_events_callback = func;
	TbtServiceLogger::LogInfo("GenlWrapper::register_events_callback exit");
}

/**
 * error handler for errors that occurs on netlink send messages
 * the function is storing remote exception that will be thrown on the thread that
 * waiting for response for netlink sent message
 */
int GenlWrapper::cb_error_handler(sockaddr_nl*, nlmsgerr* nlerr, void*) {
	std::lock_guard<std::mutex> lock(_receive_mutex);
	TbtServiceLogger::LogError("GenlWrapper::cb_error_handler entry, error arrived");
	_remote_exception = std::make_exception_ptr(TbtException((std::string("cb_error_handler: netlink error: ") + nl_geterror(nlerr->error)).c_str()));
	_send_receice_cv.notify_one();
	return NL_SKIP;
}

/**
 * this callback is called when netlink message sent without errors
 */
int GenlWrapper::cb_ack(nl_msg*, void*) {
	std::lock_guard<std::mutex> lock(_receive_mutex);
	TbtServiceLogger::LogInfo("Netlink ACK received ");
	_remote_exception = nullptr;
	_send_receice_cv.notify_one();
	return NL_SKIP;
}

/**
 * handler for valid received messages from driver
 */
int GenlWrapper::cb_valid(nl_msg* msg, void*) {
	TbtServiceLogger::LogInfo("GenlWrapper::cb_valid entry, message arrived");
	try {
		{
			std::lock_guard<std::mutex> lock(TbtServiceLogger::log_mutex);
			nl_msg_dump(msg, stdout);
			fflush(stdout);
		}
		struct nlmsghdr * hdr = nlmsg_hdr(msg);
		if(hdr->nlmsg_type!= Instance().m_family_id) {
		  TbtServiceLogger::LogWarning("Warning: netlink message received from source \
                                             != Thunderbolt driver");
		  return NL_SKIP;
		}
		struct genlmsghdr * gnlh = reinterpret_cast<struct genlmsghdr *>(nlmsg_data(hdr));
		//TODO: genlmsg_data is deprecated, should be using genlmsg_user_hdr,
    //      but it doesn't compile need to check why
		auto controller_id = *reinterpret_cast<uint32_t*>(genlmsg_data(gnlh));
		int valid = genlmsg_validate( hdr,
                                  NHI_GENL_USER_HEADER_SIZE,
                                  NHI_ATTR_MAX,
                                  nhi_gnl_policy);
		TbtServiceLogger::LogDebug("validate netlink message: %s", valid ? "FAILED" : "PASS");
		struct nlattr * attrs[NHI_ATTR_MAX + 1];
		if (genlmsg_parse(hdr,
                      NHI_GENL_USER_HEADER_SIZE,
                      attrs,
                      NHI_ATTR_MAX,
                      nhi_gnl_policy) < 0) {
		  TbtServiceLogger::LogError("Error: genlsmg_parse failed");
		  return NL_SKIP;
		}
		std::vector<uint8_t> data;
    PDF_VALUE pdf;

    switch(gnlh->cmd)
		{
      case NHI_CMD_QUERY_INFORMATION:
      {
         if( !(attrs[NHI_ATTR_DRIVER_VERSION] &&
      		attrs[NHI_ATTR_NVM_VER_OFFSET] &&
      		attrs[NHI_ATTR_NUM_PORTS] &&
      		attrs[NHI_ATTR_DMA_PORT] )) {
      	   TbtServiceLogger::LogError("Error: NHI_CMD_QUERY_INFORMATION is missing  attributes");
      	   return NL_SKIP;
        }

        QueryDriverInformation info = {controller_id,
                                       nla_get_u16(attrs[NHI_ATTR_NVM_VER_OFFSET]),
                                       nla_get_u8(attrs[NHI_ATTR_NUM_PORTS]),
                                       nla_get_u8(attrs[NHI_ATTR_DMA_PORT]),
                                       {}, // Eliminates g++ warning for missing initializer. It's initialized below
                                       static_cast<bool>(nla_get_flag(attrs[NHI_ATTR_SUPPORT_FULL_E2E]))};

        auto const& version = nla_get_string(attrs[NHI_ATTR_DRIVER_VERSION]);
        strncpy(info.driver_version,version,sizeof(info.driver_version));
        auto const pInfo = reinterpret_cast<unsigned char*>(&info);
        data = std::vector<uint8_t>(pInfo,pInfo + sizeof(info));
        pdf = PDF_DRIVER_QUERY_DRIVER_INFORMATION;
        break;
      }
      case NHI_CMD_MSG_FROM_ICM:
      {
      	if( !(  attrs[NHI_ATTR_PDF] &&
      			attrs[NHI_ATTR_MSG_FROM_ICM] )) {
      	   TbtServiceLogger::LogError("Error: NHI_CMD_MSG_FROM_ICM is missing attributes");
      	   return NL_SKIP;
      	}
      	auto msg_data = reinterpret_cast<uint8_t*>(nla_data(attrs[NHI_ATTR_MSG_FROM_ICM]));
      	auto msg_size = nla_len(attrs[NHI_ATTR_MSG_FROM_ICM]);
      	auto data_start = reinterpret_cast<uint32_t*>(msg_data);
      	auto data_end = data_start + msg_size/sizeof(uint32_t);

        //DW swap for FW message
      	std::for_each(data_start, data_end,[&](uint32_t& x){ x=be32toh(x); });
      	data = std::vector<uint8_t>(msg_data,msg_data + msg_size);
      	pdf = static_cast<PDF_VALUE>(nla_get_u32(attrs[NHI_ATTR_PDF]));
      	break;
      }
      default:
        TbtServiceLogger::LogWarning("Warning: Unknown message arrived, NETLINK CMD = %d", gnlh->cmd);
        return NL_SKIP;
      }

    if (Instance().m_events_callback == nullptr) {
			TbtServiceLogger::LogWarning("Warning: no receive callback is registered");
			return NL_SKIP;
		}
		Instance().m_events_callback(controller_id,pdf,data);
	}
	catch (const std::exception& e) {
		TbtServiceLogger::LogError("GenlWrapper::cb_valid exception!!!: %s",e.what());
	}
	TbtServiceLogger::LogInfo("GenlWrapper::cb_valid exit, message arrived");
	return NL_SKIP;
}

/**
 * send unsubscribe to driver and release netlink socket
 */
GenlWrapper::~GenlWrapper() {
  m_shutting_down = true;
	driver_unsubscribe();
	nl_socket_free(m_genl_socket);
}

/**
 * this function is running in a thread that is responsible for receiving netlink
 * messages from the driver
 */
void GenlWrapper::receive_thread() {
	while(!m_shutting_down) {
	    try {
	      nl_recvmsgs_default(m_genl_socket);
	    }
	    catch(const std::exception& e) {
	      TbtServiceLogger::LogError("Error: netlink message receive failed (Exception: %s)", e.what());
	    }
	}
	TbtServiceLogger::LogInfo("netlink receive thread canceled");
}

/**
 * this function is for sending ISerializable messages and wait until send ACK
 */
void GenlWrapper::send_message_sync(uint32_t controller_id,NHI_GENL_CMD cmd, std::shared_ptr<ISerializable> send_obj)
{

  try {
	  TbtServiceLogger::LogInfo("GenlWrapper::send_message_sync entry, sending netlink message: ISerializable");
		int put_res = 0;
		auto msg = alloc_message(controller_id,cmd);
		auto properties = send_obj->GetAllProperties();

		for(auto attr: properties) {
			auto genl_att_type = netlink_properties_converter[WStringToString(attr.first)];

			//this is for attributes that not need to be send in linux (e.g.InstanceName);
			if (genl_att_type == NHI_ATTR_UNSPEC)
				continue;

			switch (static_cast<SerializationType>(attr.second.which())) {
				case SerializationType::BOOL:
					if (boost::get<bool>(attr.second))
						put_res = nla_put_flag(msg.get(), genl_att_type);
					break;
				case SerializationType::BUFFER:
				{
					auto vec = boost::get<std::vector<uint8_t>>(attr.second);
					put_res = nla_put(msg.get(), genl_att_type, vec.size(), vec.data());
					break;
				}
				case SerializationType::UINT16:
					put_res = nla_put_u16(msg.get(),genl_att_type,boost::get<uint16_t>(attr.second));
					break;
				case SerializationType::UINT32:
					put_res = nla_put_u32(msg.get(),genl_att_type,boost::get<uint32_t>(attr.second));
					break;
				case SerializationType::UINT8:
					put_res = nla_put_u8(msg.get(),genl_att_type,boost::get<uint8_t>(attr.second));
					break;
				case SerializationType::WSTRING: //TODO: check about this wstring and if needed, currently converting to string
				{
					auto str = WStringToString(boost::get<std::wstring>(attr.second));
					put_res = nla_put_string(msg.get(),genl_att_type,str.c_str());
					break;
				}
				default:
					break;
			}
			if(put_res < 0)
				throw TbtException((std::string("Failed put netlink attribute: ") + WStringToString(attr.first)).c_str());
		}
		send_message_sync(msg.get());
	}
	catch (const std::exception& e) {
		TbtServiceLogger::LogError("Error: Fail sending message: controller = %x CMD = %d, exception( %s )",controller_id , cmd, e.what());
		throw;
	}
	TbtServiceLogger::LogInfo("GenlWrapper::send_message_sync exit, sending netlink message: ISerializable");
}

/**
 * sending command (enum value) message to driver and wait for send ACK
 */
void GenlWrapper::send_message_sync(uint32_t controller_id, NHI_GENL_CMD cmd)
{
	TbtServiceLogger::LogInfo("GenlWrapper::send_message_sync entry, sending netlink message: command only");
	try {
		auto msg = alloc_message(controller_id,cmd);
		send_message_sync(msg.get());
	}
	catch (const std::exception& e) {
		TbtServiceLogger::LogError("Error: Fail sending message: controller = %x CMD = %d, exception( %s )",controller_id , cmd, e.what());
		throw;
	}
	TbtServiceLogger::LogInfo("GenlWrapper::send_message_sync exit, sending netlink message: command only");
}

/**
 * sending netlink raw data message to driver and wait for send ACK
 * with 100 millisecond timeout
 */
void GenlWrapper::send_message_sync(struct nl_msg* msg)
{
	std::unique_lock<std::mutex> cv_lock(_receive_mutex);
	TbtServiceLogger::LogInfo("GenlWrapper::send_message_sync(struct nl_msg* msg) entry");
	{
		std::lock_guard<std::mutex> lock(TbtServiceLogger::log_mutex);
		printf("Send Message:\n");
		nl_msg_dump(msg, stdout);
		fflush(stdout);
	}
	int err = 0;
	if((err = nl_send_auto(m_genl_socket, msg)) >= 0)
	{
		if(_send_receice_cv.wait_for(cv_lock,std::chrono::milliseconds(NETLINK_ACK_TIME_OUT)) == std::cv_status::timeout)
		{
			TbtServiceLogger::LogWarning("Warning: sending message ACK timeout");
		}
	}
	if(err < 0)
		throw TbtException((std::string("send_message_sync failed: netlink error: %s") + nl_geterror(err)).c_str());

	if(_remote_exception)
		std::rethrow_exception(_remote_exception);
	TbtServiceLogger::LogInfo("GenlWrapper::send_message_sync(struct nl_msg* msg) exit");
}

/**
 * sending message with enum command without waiting for send ack
 */
void GenlWrapper::send_message_async(uint32_t controller_id, NHI_GENL_CMD cmd)
{
	TbtServiceLogger::LogInfo("GenlWrapper::send_message entry, sending netlink message: command only");
  try {
	  	auto msg = alloc_message(controller_id,cmd);
	    {
			std::lock_guard<std::mutex> lock(TbtServiceLogger::log_mutex);
			printf("Send Message:\n");
			nl_msg_dump(msg.get(), stdout);
			fflush(stdout);
	  	}
	  	if(nl_send_auto(m_genl_socket, msg.get())<=0)
		{
			TbtServiceLogger::LogError("Error: sending netlink message failed");
		}

	}
	catch (const std::exception& e) {
    	TbtServiceLogger::LogError("Error: fail sending message, exception( %s )",e.what());
		throw;
	}
	TbtServiceLogger::LogInfo("GenlWrapper::send_message exit, sending netlink message: command only");
}
