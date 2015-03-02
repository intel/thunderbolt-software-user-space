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

#include "Port.h"
#include <memory>
#include <ctime>
#include "UniqueID.h"
#include "logger.h"
#include "tbtException.h"

const unsigned int MaxConnectionTimeDiff = 2;

Port::Port(void) :
   m_P2PDevice(nullptr)
{
}

Port::~Port(void)
{

}

bool Port::hasP2PDevice() const
{
   return m_P2PDevice != nullptr;
}

std::shared_ptr<IP2PDevice> Port::getP2PDevice() const
{
   if (m_P2PDevice == nullptr)
   {
      throw TbtException("getP2PDevice failed: P2P device doesn't exist in port");
   }
   return m_P2PDevice;
}

void Port::setP2PDevice(std::shared_ptr<IP2PDevice> pP2PDevice)
{
   m_P2PDevice = pP2PDevice;
}

void Port::removeP2PDevice()
{
	if(m_P2PDevice)
	{
		m_P2PDevice = nullptr;
	}
}
