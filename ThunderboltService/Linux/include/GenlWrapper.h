/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2014 - 2016 Intel Corporation.
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

// event handler for received messages from driver
typedef std::function<void(uint32_t controller_id, PDF_VALUE pdf, const std::vector<uint8_t>&)> events_callback;
// deleter for netlink message when shared_ptr released
typedef std::function<void(struct nl_msg* msg)> netlink_message_deleter;
// shared ptr with custom deleter for releasing the message
typedef std::unique_ptr<struct nl_msg, netlink_message_deleter> netlink_mgs_ptr;

/**
 * this class is a wrapper for generic netlink layer.
 * it responsible of sending messages to driver through netlink and receiving
 * notifications and response from it.
 */
class GenlWrapper
{
public:
   ~GenlWrapper();
   static GenlWrapper& Instance();
   void send_message_sync(uint32_t controller_id, NHI_GENL_CMD cmd, std::shared_ptr<ISerializable> send_obj);
   void send_message_sync(uint32_t controller_id, NHI_GENL_CMD cmd);
   void send_message_async(uint32_t controller_id, NHI_GENL_CMD cmd);
   void register_events_callback(events_callback func);

private:
   // netlink callbacks
   int cb_error_handler(sockaddr_nl*, nlmsgerr* nlerr);
   int cb_ack(nl_msg*);
   int cb_valid(nl_msg* msg);

   // methods
   GenlWrapper(); // registration for the netlink family, subscribe driver
   void receive_thread();
   void register_netlink_callbacks();
   void driver_subscribe();
   void driver_unsubscribe();
   void send_message_sync(struct nl_msg* msg);
   netlink_mgs_ptr alloc_message(uint32_t controller_id, NHI_GENL_CMD cmd_type);

   // members
   int m_family_id;               // driver netlink handler for communication
   struct nl_sock* m_genl_socket; // netlink socket
   std::mutex m_send_mutex;
   // callback for receiving a events
   std::function<void(uint32_t controller_id, PDF_VALUE pdf, const std::vector<uint8_t>&)> m_events_callback;
   bool m_shutting_down;

   std::mutex m_register_mutex;
   std::condition_variable m_register_cv;
   bool m_registered;
};

#endif /* GENLWRAPPER_H_ */
