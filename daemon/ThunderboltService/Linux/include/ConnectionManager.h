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
#include "ConnectionManagerBase.h"

#include "tbtdbus.h"

class ControllerSettings;

/**
 * This class is used as singleton and contain the main logic of the daemon
 */
class ConnectionManager : public ConnectionManagerBase
{
public:
   ConnectionManager(std::shared_ptr<IControllerCommandSender> sender);

   static std::shared_ptr<ConnectionManager> GetInstance();

   /**
    * Set the D-Bus connection instance.  This is used to instantiate
    * DBusController wrappers around the individual controller abstraction
    * objects we create when we become aware of controllers.  Any currently
    * existing DBusController wrappers are unaffected.  This method is expected
    * to be called at startup time, before entering the main dispatch loop.
    */
   void SetDBusConnection(DBus::Connection* pCx);
   DBus::Connection* GetDBusConnection();

   void SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd);
   void QueryDriverInformation(const controlleriD& Cid) override;
   std::string GetLocalHostUTF8ComputerName() const;
   std::shared_ptr<P2PDevice> CreateP2PDevice(const controlleriD& ControllerId,
                                              const INTER_DOMAIN_CONNECTED_NOTIFICATION& e) const;
   void OnApproveInterDomainConnectionResponse(const controlleriD&, const APPROVE_INTER_DOMAIN_CONNECTION_RESPONSE&);
   void OnInterDomainConnect(const controlleriD& ControllerId, const INTER_DOMAIN_CONNECTED_NOTIFICATION& e);

private:
   /**
    * Handle to the D-Bus connection.
    */
   DBus::Connection* m_pDBus;
};
