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

#ifndef _ICONTROLLER_
#define _ICONTROLLER_

#include <map>
#include <memory>
#include <string>
#include "Port.h"
#include "Sdk.h"
#include <cstdint>

class ControllerDetails;
class ControllerSettings;
class DBusController;
class DriverCommandResponseLock;
class P2PDevice;
class Version;

typedef std::map<uint32_t, std::shared_ptr<Port>> PortsMap;

/* IController Interface
 * ---------------------
 * Represents a Thunderbolt controller on the machine
 */
class IController
{
public:
   virtual ~IController(){};

   // Methods
   virtual const controlleriD& GetControllerID() const = 0;

   virtual void AddPort(uint32_t position, std::shared_ptr<Port> PortToAdd) = 0;

   virtual SECURITY_LEVEL GetSecurityLevel() const = 0;

   virtual const PortsMap& GetPorts() const = 0;

   virtual std::shared_ptr<Port> GetPortAt(uint32_t position) const = 0;

   virtual int GetNumOfPorts() const = 0;

   virtual bool HasPort(uint32_t PortNum) const = 0;

   virtual std::shared_ptr<ControllerDetails> GetControllerData() const = 0;

   virtual void SetControllerData(std::shared_ptr<ControllerDetails> ControllerData) = 0;

   virtual int GetDmaPort() const = 0;

   virtual void SetDmaPort(int port) = 0;

   /**
    * \brief Runs the essential flow of FW update
    *
    * \param[in]  BufferSize  size of Buffer
    * \param[in]  Buffer      FW image to update with
    * \param[in]  routeString Route string of the device to update.
    *                         With HR - zeroed struct.
    * \param[in]  dmaPort     DMA port to use
    *
    * For updating a controller, use ControllerFwUpdate() that performs more needed operations as
    * disconnecting all devices before the update etc.
    *
    * \return TBTDRV_STATUS::SUCCESS_RESPONSE_CODE for successful update; otherwise - appropriate
    *         error code
    */
   virtual TBTDRV_STATUS
   DoFwUpdate(const std::vector<uint8_t>& Buffer, const ROUTE_STRING& routeString, int dmaPort) = 0;

   virtual void SetNvmVersionOffset(uint32_t address) = 0;

   virtual void SendDriverCommand(SW_TO_FW_INMAILCMD_CODE cmd) = 0;

   virtual uint32_t ControllerFwUpdate(const std::vector<uint8_t>& fwImage) = 0;
   virtual bool IsUp() const                                                = 0;
   virtual void SetIsUp(bool isUp)                                          = 0;

   virtual uint32_t GetNVMVersion(Version&, bool noFWUcheck = false) const = 0;

   virtual uint32_t ReadFirmware(uint32_t offset, uint32_t length, std::vector<uint8_t>& data) = 0;

   /**
    * \brief Returns if the controller supports full-E2E mode for better P2P performance (>= AR)
    *
    * \returns true if full-E2E mode is supported, otherwise - false
    */
   virtual bool GetSupportsFullE2E() = 0;

   /**
    * \brief Sets if the controller supports full-E2E mode for better P2P performance (>= AR)
    *
    * \param[in]  fullE2ESupport    true if full-E2E mode is supported
    */
   virtual void SetSupportsFullE2E(bool fullE2ESupport) = 0;

   virtual void Clear() = 0;

   virtual void AddP2PDevice(uint32_t port, std::shared_ptr<P2PDevice> pDevice) = 0;

   /**
    * \brief Tells if the controller information is all up-to-date (i.e. QueryDriverInformation() has finished updating
    * its settings according to driver information)
    *
    * \return true if controller information is up-to-date, otherwise - false
    */
   virtual bool IsReady() const = 0;

   /**
    * \brief Tells the controller that its information is all up-to-date (i.e. QueryDriverInformation() has finished
    * updating its settings according to driver information)
    */
   virtual void SetIsReady() = 0;

   /**
    * Return a pointer to the D-Bus wrapper for this controller object,
    * if any.  Note that the returned pointer will not outlive this IController
    * instance.
    */
   virtual DBusController* GetDBusWrapper()                     = 0;
   virtual DriverCommandResponseLock& GetFWUpdateResponseLock() = 0;
};

#endif // !_ICONTROLLER_
