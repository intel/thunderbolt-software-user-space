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
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <condition_variable>
#include "IConnectionManager.h"
#include "IController.h"
#include "UniqueID.h"
#include "Utils.h"
#include "logger.h"

using std::map;
using std::unique_ptr;

/**
 * This class contains the main logic of the daemon
 */
class ConnectionManagerBase : public IConnectionManager
{
public:
   ConnectionManagerBase(std::shared_ptr<IControllerCommandSender>& sender);

   virtual ~ConnectionManagerBase() {}

   void RegisterExitCallback(std::function<void()> ExitCallback);
   // Notifications from FW
   virtual void OnInterDomainConnect(const controlleriD&, const INTER_DOMAIN_CONNECTED_NOTIFICATION& e);
   virtual void OnInterDomainDisconnected(const controlleriD&, const INTER_DOMAIN_DISCONNECTED_NOTIFICATION& e);
   // Responses from FW
   virtual void OnDriverReadyResponse(const controlleriD&, const DRIVER_READY_RESPONSE& e);

   virtual void OnServiceDown();
   virtual shared_ptr<IController> GetController(const controlleriD& Cid) const;
   virtual map<controlleriD, shared_ptr<IController>> GetControllers() const;
   virtual IControllerCommandSender& GetControllerCommandSender() const;

   /**
    * \brief Queries driver for information about the driver and a specific controller
    *
    * Please note: this function just sent the query, the data retrival happens in
    * LinuxControllerCommandSender::OnEvent()
    *
    * \param[in]  Cid   Controller ID as retrieved from the driver
    */
   virtual void QueryDriverInformation(const controlleriD& Cid);

   virtual void SetFwUpdateStatus(bool isInFwUpdateProcess);

   virtual bool IsDuringFwUpdate() const noexcept;
   virtual void OnFwIsInSafeMode(const controlleriD&);

   virtual std::shared_ptr<ControllerSettings> GetControllersSettings() const;
   virtual void SetControllersSettings(const std::shared_ptr<ControllerSettings> Settings);
   virtual void setPreShutdownMode() { m_isPreShutdownMode = true; };
   virtual bool isPreShutdownMode() const { return m_isPreShutdownMode; };
   std::shared_ptr<P2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum);
   const std::shared_ptr<P2PDevice> GetP2PDevice(controlleriD cId, uint32_t portNum) const;
   virtual void OnSystemPreShutdown();

protected:
   shared_ptr<IControllerCommandSender> m_ControllerCommandSender;
   map<controlleriD, shared_ptr<IController>> m_Controllers;
   mutable std::mutex m_ControllersGuard;
   bool m_isPreShutdownMode;
   std::function<void()> m_ExitCallback;

protected:
   std::shared_ptr<ControllerSettings> m_ControllersSettings;
   bool m_DuringPowerCycle;
   bool m_DuringFwUpadte;

private:
   // No Copy or assignment to prevent memory dup
   ConnectionManagerBase(const ConnectionManagerBase&);
   ConnectionManagerBase& operator=(const ConnectionManagerBase&);
};
