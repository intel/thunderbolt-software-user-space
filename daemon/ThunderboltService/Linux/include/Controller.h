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

#ifndef _CONTROLLER_
#define _CONTROLLER_

#include <map>
#include <memory>
#include <future>
#include "IController.h"
#include "P2PDevice.h"
#include "boost/optional.hpp"

#include "DBusController.h"
#include "DriverCommandResponseLock.h"
#include "Utils.h"

/**
 * this class represents a Thunderbolt controller on the machine
 */
class Controller : public IController
{
public:
   Controller(controlleriD id, DRIVER_READY_RESPONSE controllerData);

   // This controller is for use during safe mode ONLY!
   Controller(controlleriD id);

   virtual const controlleriD& GetControllerID() const;
   virtual void SetNvmVersionOffset(uint32_t address);
   virtual SECURITY_LEVEL GetSecurityLevel() const;
   virtual const PortsMap& GetPorts() const;
   virtual std::shared_ptr<Port> GetPortAt(uint32_t position) const;
   virtual void AddPort(uint32_t position, std::shared_ptr<Port> PortToAdd);
   virtual int GetNumOfPorts() const;
   virtual std::shared_ptr<ControllerDetails> GetControllerData() const;

   virtual void SetControllerData(std::shared_ptr<ControllerDetails> ControllerData);
   virtual bool HasPort(uint32_t PortNum) const;
   virtual int GetDmaPort() const;
   virtual void SetDmaPort(int port);

   virtual void SendDriverCommand(SW_TO_FW_INMAILCMD_CODE code);

   virtual DBusController* GetDBusWrapper();

   virtual uint32_t GetNVMVersion(Version& nvmVersion, bool noFWUCheck = false) const;

   bool m_inFWUpdate = false; // includes EP FW update

   /**
    * \brief Returns if the controller supports full-E2E mode for better P2P performance (>= AR)
    *
    * \returns true if full-E2E mode is supported, otherwise - false
    */
   bool GetSupportsFullE2E() override;

   /**
    * \brief Sets if the controller supports full-E2E mode for better P2P performance (>= AR)
    *
    * \param[in]  fullE2ESupport    true if full-E2E mode is supported
    */
   void SetSupportsFullE2E(bool fullE2ESupport) override;

   virtual void Clear();

   virtual void AddP2PDevice(uint32_t Port, std::shared_ptr<P2PDevice> pDeviceToAdd);

   uint32_t ControllerFwUpdate(const std::vector<uint8_t>& Buffer) override;

   TBTDRV_STATUS DoFwUpdate(const std::vector<uint8_t>& Buffer, const ROUTE_STRING& routeString, int dmaPort) override;
   uint32_t ReadFirmware(uint32_t offset, uint32_t length, std::vector<uint8_t>& data);

   inline virtual DriverCommandResponseLock& GetFWUpdateResponseLock();
   virtual void SetIsUp(bool isUp);
   virtual bool IsUp() const { return m_IsUp; }

   /**
    * \brief Tells if the controller information is all up-to-date (i.e. QueryDriverInformation() has finished updating
    * its settings according to driver information)
    *
    * \return true if controller information is up-to-date, otherwise - false
    */
   bool IsReady() const override { return m_isReady; }

   /**
    * \brief Tells the controller that its information is all up-to-date (i.e. QueryDriverInformation() has finished
    * updating its settings according to driver information)
    */
   void SetIsReady() override { m_isReady = true; }

private:
   controlleriD m_Id;                                      // the controller identifier
   std::shared_ptr<ControllerDetails> m_ControllerDetails; // controller details

   PortsMap m_Ports;     // ports that hold the connected devices
   int m_DmaPort;        // the controller DMA port number
   uint32_t m_NvmOffset; // offset of the controller NVM
   bool m_IsUp;          // the power mode of the controller
   bool m_SupportsFullE2E = false;

   std::atomic_bool m_isReady;

   std::unique_ptr<DBusController> m_pDBusWrapper;

   DriverCommandResponseLock m_FWUpdateResponseLock;
};

#endif // !_CONTROLLER_
