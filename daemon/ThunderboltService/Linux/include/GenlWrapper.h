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

#ifndef GENLWRAPPER_H_
#define GENLWRAPPER_H_

#include <functional>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <future>
#include <netlink/netlink.h>
#include "ISerializable.h"
#include "defines.h"
#include "GenetlinkInterface.h"
#include "logger.h"

//event handler for received messages from driver
typedef std::function<void(uint32_t controller_id,PDF_VALUE pdf,const std::vector<uint8_t>&)> events_callback;
//deleter for netlink message when shared_ptr released 
typedef std::function<void(struct nl_msg * msg)> netlink_message_deleter;
//shared ptr with custom deleter for releasing the message
typedef std::unique_ptr<struct nl_msg,netlink_message_deleter> netlink_mgs_ptr;

/**
 * this class is a wrapper for generic netlink layer.
 * it responsible of sending messages to driver through netlink and receiving
 * notifications and response from it.
 */
class GenlWrapper {
public:
	~GenlWrapper();
	static GenlWrapper& Instance();
	void send_message_sync(uint32_t controller_id, NHI_GENL_CMD cmd,std::shared_ptr<ISerializable> send_obj);
	void send_message_sync(uint32_t controller_id, NHI_GENL_CMD cmd);
	void send_message_async(uint32_t controller_id, NHI_GENL_CMD cmd);
	void register_events_callback(events_callback func);
private:
	//netlink callbacks
	static int cb_error_handler(struct sockaddr_nl *nla, struct nlmsgerr *nlerr, void *arg);
	static int cb_ack(struct nl_msg * msg, void * arg);
	static int cb_valid(struct nl_msg * msg, void * arg);

	//methods
	GenlWrapper();		//registration for the netlink family, subscribe driver
	void receive_thread();
	void register_netlink_callbacks();
	void driver_subscribe();
	void driver_unsubscribe();
	void send_message_sync(struct nl_msg* msg);
	netlink_mgs_ptr alloc_message(uint32_t controller_id, NHI_GENL_CMD cmd_type);

	//members
	int m_family_id;               //driver netlink handler for communication
	struct nl_sock* m_genl_socket; //netlink socket
	std::mutex m_send_mutex;
	//callback for receiving a events
	std::function<void(uint32_t controller_id,PDF_VALUE pdf,const std::vector<uint8_t>&)> m_events_callback;
	bool m_shutting_down;

};

#endif /* GENLWRAPPER_H_ */
