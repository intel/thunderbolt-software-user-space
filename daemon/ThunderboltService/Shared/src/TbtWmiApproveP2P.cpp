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

#include "TbtWmiApproveP2P.h"
#include "MessagesWrapper.h"
#include "UniqueID.h"

PropertiesMap TbtWmiApproveP2P::GetWriteableProperties() const
{
   PropertiesMap Properties;
   
   Properties[L"LocalRouteString"] = SerializationValue(m_LocalRouteString);
   Properties[L"LocalUniqueID"] = SerializationValue(m_LocalUniqueID);
   Properties[L"RemoteUniqueID"] = SerializationValue(m_RemoteUniqueID);
   Properties[L"PeerOS"] = SerializationValue(m_PeerOS);
   Properties[L"LocalDepth"] = SerializationValue(m_LocalDepth);

   return Properties;
}

PropertiesMap TbtWmiApproveP2P::GetReadOnlyProperties() const
{
   PropertiesMap Properties;

   Properties[L"InstanceName"] = m_InstanceName;

   return Properties;
}

PropertiesMap TbtWmiApproveP2P::GetAllProperties() const
{
   PropertiesMap Properties = GetReadOnlyProperties();
   PropertiesMap WriteableProperties = GetWriteableProperties();

   Properties.insert(WriteableProperties.begin(), WriteableProperties.end());

   return Properties;
}

std::wstring TbtWmiApproveP2P::GetInstanceName() const
{
   return m_InstanceName;
}

void TbtWmiApproveP2P::LoadFromSerializationMap(const PropertiesMap& PropertiesMapToLoad)
{
   throw std::logic_error("The method or operation is not implemented.");
}

TbtWmiApproveP2P::TbtWmiApproveP2P( const ROUTE_STRING& LocalRouteString, 
                                    const UniqueID& LocalUniqueID, 
                                    const UniqueID& RemoteUniqueID, 
                                    uint32_t PeerOS, 
                                    uint8_t LocalDepth) :m_PeerOS(PeerOS),
                                                         m_LocalDepth(LocalDepth)
                                                         
{
   SetLocalRouteString(LocalRouteString);
   SetLocalUniqueID(LocalUniqueID);
   SetRemoteUniqueID(RemoteUniqueID);
}

TbtWmiApproveP2P::TbtWmiApproveP2P()
{
}
void TbtWmiApproveP2P::SetLocalRouteString(ROUTE_STRING LocalRouteString)
{
   m_LocalRouteString.clear();
   std::copy_n((PUCHAR)&LocalRouteString, sizeof(ROUTE_STRING), std::back_inserter(m_LocalRouteString));
}

void TbtWmiApproveP2P::SetLocalUniqueID(UniqueID LocalUniqueID)
{
   m_LocalUniqueID.clear();
   std::copy_n((PUCHAR)LocalUniqueID.data(), sizeof(UNIQUE_ID), std::back_inserter(m_LocalUniqueID));
}

void TbtWmiApproveP2P::SetRemoteUniqueID(UniqueID RemoteUniqueID)
{
   m_RemoteUniqueID.clear();
   std::copy_n((PUCHAR)RemoteUniqueID.data(), sizeof(UNIQUE_ID), std::back_inserter(m_RemoteUniqueID));
}

void TbtWmiApproveP2P::SetLocalDepth(uint8_t LocalDepth)
{
   m_LocalDepth = LocalDepth;
}

void TbtWmiApproveP2P::SetPeerOs(uint32_t PeerOS)
{
   m_PeerOS = PeerOS;
}
