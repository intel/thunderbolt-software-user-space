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

#pragma once

#include <memory>
#include <map>
#include <functional>
#include "defines.h"
#include "IControllerCommandSender.h"
#include "IController.h"

using std::unique_ptr;
using std::shared_ptr;
using std::map;

class P2PDevice;

/* IConnectionManager Interface
 * ----------------------------
 * Main component, handle all responses and notifications from both application and driver/FW
 *
 * Should be a singleton
 *
 */
class IConnectionManager
{
public:
   virtual ~IConnectionManager(){};
   virtual void RegisterExitCallback(std::function<void()> ExitCallback) = 0;

   // Notifications - FW
   virtual void OnInterDomainConnect(const controlleriD&, const INTER_DOMAIN_CONNECTED_NOTIFICATION& e)         = 0;
   virtual void OnInterDomainDisconnected(const controlleriD&, const INTER_DOMAIN_DISCONNECTED_NOTIFICATION& e) = 0;

   // Responses
   virtual void OnDriverReadyResponse(const controlleriD&, const DRIVER_READY_RESPONSE& e) = 0;
   virtual void OnApproveInterDomainConnectionResponse(const controlleriD&,
                                                       const APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE& e) = 0;

   // Methods
   virtual std::string GetLocalHostUTF8ComputerName() const = 0;
   virtual std::shared_ptr<P2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum)             = 0;
   virtual const std::shared_ptr<P2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum) const = 0;
   virtual std::shared_ptr<P2PDevice> CreateP2PDevice(const controlleriD& ControllerId,
                                                      const INTER_DOMAIN_CONNECTED_NOTIFICATION& e) const = 0;
   virtual void SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd) = 0;
   virtual shared_ptr<IController> GetController(const controlleriD&) const = 0;
   virtual map<controlleriD, shared_ptr<IController>> GetControllers() const = 0;
   virtual const IControllerCommandSender& GetControllerCommandSender() const              = 0;
   virtual void OnSystemPreShutdown()                                                      = 0;
   virtual void setPreShutdownMode()                                                       = 0;
   virtual bool isPreShutdownMode() const                                                  = 0;
   virtual std::shared_ptr<ControllerSettings> GetControllersSettings() const              = 0;
   virtual void SetControllersSettings(const std::shared_ptr<ControllerSettings> Settings) = 0;
};
