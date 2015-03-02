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

#ifndef _TBT_WMI_APPROVE_P2P_H_
#define _TBT_WMI_APPROVE_P2P_H_

#include <string>
#include "ISerializable.h"
#include "MessagesWrapper.h"

class UniqueID;

/**
 * This class implements a messaging sent to driver after properties exchange
 * with Thunderbolt IP supported remote host.
 * After driver receives the info, it will create an Ethernet adapter fot the P2P 
 * and builds the rx/tx path
 */
class TbtWmiApproveP2P :
   public ISerializable
{

public:
   TbtWmiApproveP2P();
   TbtWmiApproveP2P(const ROUTE_STRING& LocalRouteString,const UniqueID& LocalUniqueID, const UniqueID& RemoteUniqueID, uint32_t PeerOS,uint8_t LocalDepth);

   virtual PropertiesMap GetWriteableProperties() const;

   virtual PropertiesMap GetReadOnlyProperties() const;

   virtual PropertiesMap GetAllProperties() const;

   virtual std::wstring GetInstanceName() const;

   virtual void LoadFromSerializationMap(const PropertiesMap& PropertiesMapToLoad);

   void SetLocalRouteString(ROUTE_STRING LocalRouteString);

   void SetLocalUniqueID(UniqueID LocalUniqueID);

   void SetRemoteUniqueID(UniqueID RemoteUniqueID);

   void SetPeerOs(uint32_t PeerOS);

   void SetLocalDepth(uint8_t LocalDepth);

private:
   std::wstring m_InstanceName;
   std::vector<uint8_t> m_LocalRouteString;           // Size 2
   std::vector<uint8_t> m_LocalUniqueID;              // Size 4
   std::vector<uint8_t> m_RemoteUniqueID;             // Size 4
   uint8_t m_LocalDepth;
   uint32_t m_PeerOS;
};

#endif // !_TBT_WMI_APPROVE_P2P_H_
