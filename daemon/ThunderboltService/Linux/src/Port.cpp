/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2009 - 2016 Intel Corporation.
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

#include "Port.h"
#include <memory>
#include <ctime>
#include <numeric>
#include "logger.h"
#include "tbtException.h"
#include "P2PDevice.h"
#include <iterator>

const unsigned int MaxConnectionTimeDiff = 2;

Port::Port(const controlleriD& controllerID, uint8_t index) : m_controllerID(controllerID), m_index(index)
{
}

bool Port::hasP2PDevice() const
{
   return m_P2PDevice != nullptr;
}

std::shared_ptr<P2PDevice> Port::getP2PDevice() const
{
   if (m_P2PDevice == nullptr)
   {
      throw TbtException("getP2PDevice failed: P2P device doesn't exist in port");
   }
   return m_P2PDevice;
}

void Port::setP2PDevice(std::shared_ptr<P2PDevice> pP2PDevice)
{
   m_P2PDevice = pP2PDevice;
}

void Port::removeP2PDevice()
{
   m_P2PDevice = nullptr;
}
