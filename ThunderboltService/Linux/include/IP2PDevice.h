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

#ifndef _IP2P_DEVICE_H_
#define _IP2P_DEVICE_H_
#include <atomic>
#include "MessagesWrapper.h"
#include "UniqueID.h"

enum class P2P_STATE
{
   NOT_READY,          // the device is not ready, on start and time_out
   READING_PROPERTIES, // the device is currently reading or handling remote host properties
   TUNNEL,             // the device is tunneled, Ethernet adapter is up
   NOT_SUPPORTED,      // the device is not support Thunderbolt IP
   PENDING_TUNNEL      // the device support Thunderbolt IP and wait for driver to bring the Ethernet adapter up
};

/**
 * this interface if for holding a P2P device object created when connecting two
 * Thunderbolt hosts (Windows <--> Linux <--> Apple)
 */
class IP2PDevice
{
public:
   virtual ~IP2PDevice(){};
   virtual void HandleInterDomainRequest(const std::vector<uint8_t>& Msg)                              = 0;
   virtual void HandleInterDomainResponse(const std::vector<uint8_t>& Msg)                             = 0;
   virtual void SendPropertiesChangeResponse(const XDOMAIN_PROPERTIES_CHANGED_NOTIFICATION& Msg) const = 0;
   virtual void SendReadPropertiesResponse(uint32_t Offset, uint8_t sn) const = 0;

   /**
    * \brief Returns if full-E2E mode should be enabled for this P2P device
    *
    * \return true if full-E2E mode should be enabled, otherwise - false
    */
   virtual bool GetEnableFullE2E() const = 0;

   /**
   * \brief Returns if P2P should match fragments ID
   *
   * \return true if match fragments ID, otherwise - false
   */
   virtual bool GetMatchFragmentsID() const = 0;

   virtual void StartHandshake()                           = 0;
   virtual void SendPropertiesChangeRequest()              = 0;
   virtual void OnSystemPreShutdown()                      = 0;
   virtual const UniqueID& RemoteRouterUniqueID() const    = 0;
   virtual const ROUTE_STRING& LocalRouteString() const    = 0;
   virtual const UniqueID& LocalHostRouterUniqueID() const = 0;
   virtual std::string RemoteHostName() const              = 0;
   virtual bool PathEstablished() const                    = 0;
   virtual const controlleriD& ControllerID() const        = 0;
   virtual uint8_t Depth() const                           = 0;
   virtual const std::atomic<P2P_STATE>& State() const     = 0;
   virtual void SetState(P2P_STATE value)                  = 0;
};
#endif // !_IP2P_DEVICE_H_
