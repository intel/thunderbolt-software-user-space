/*******************************************************************************
 *
 * Intel Thunderbolt(TM) daemon
 * Copyright(c) 2014 - 2016 Intel Corporation.
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
#include "logger.h"
#include "Utils.h"

const std::wstring TbtWmiApproveP2P::InstanceName     = L"InstanceName";
const std::wstring TbtWmiApproveP2P::LocalRouteString = L"LocalRouteString";
const std::wstring TbtWmiApproveP2P::LocalUniqueID    = L"LocalUniqueID";
const std::wstring TbtWmiApproveP2P::RemoteUniqueID   = L"RemoteUniqueID";
const std::wstring TbtWmiApproveP2P::PeerOS           = L"PeerOS";
const std::wstring TbtWmiApproveP2P::LocalDepth       = L"LocalDepth";
const std::wstring TbtWmiApproveP2P::EnableFullE2E    = L"EnableFullE2E";
const std::wstring TbtWmiApproveP2P::MatchFragmentsID = L"MatchFragmentsID";

namespace // anon. namespace
{
/**
 * \brief used internally to log the flag status
 * 
 * \param[in]  flag
 * \param[in]  description  description of the flag
 */
void logFlag(bool flag, std::string description)
{
   std::string infoStr = description + " is set as ";
   infoStr += (flag ? "enabled" : "disabled");
   infoStr += " in WMI Approve P2P message to the driver";
   TbtServiceLogger::LogDebug(infoStr.c_str());
}
} // anon. namespace

PropertiesMap TbtWmiApproveP2P::GetWriteableProperties() const
{
   PropertiesMap Properties;
   
   Properties[LocalRouteString] = SerializationValue(m_LocalRouteString);
   Properties[LocalUniqueID] = SerializationValue(m_LocalUniqueID);
   Properties[RemoteUniqueID] = SerializationValue(m_RemoteUniqueID);
   Properties[PeerOS] = SerializationValue(m_PeerOS);
   Properties[LocalDepth] = SerializationValue(m_LocalDepth);
   Properties[EnableFullE2E] = SerializationValue(m_EnableFullE2E);
   Properties[MatchFragmentsID] = SerializationValue(m_MatchFragmentsID);

   return Properties;
}

PropertiesMap TbtWmiApproveP2P::GetReadOnlyProperties() const
{
   PropertiesMap Properties;

   Properties[InstanceName] = m_InstanceName;

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

void TbtWmiApproveP2P::LoadFromSerializationMap(const PropertiesMap&)
{
   throw std::logic_error("The method or operation is not implemented.");
}

TbtWmiApproveP2P::TbtWmiApproveP2P(const ROUTE_STRING& LocalRouteString,
                                   const UniqueID& LocalUniqueID,
                                   const UniqueID& RemoteUniqueID,
                                   uint32_t PeerOS,
                                   uint8_t LocalDepth,
                                   bool EnableFullE2E,
                                   bool MatchFragmentsID)
   : m_LocalDepth(LocalDepth), m_PeerOS(PeerOS), m_EnableFullE2E(EnableFullE2E), m_MatchFragmentsID(MatchFragmentsID)
{
   SetLocalRouteString(LocalRouteString);
   SetLocalUniqueID(LocalUniqueID);
   SetRemoteUniqueID(RemoteUniqueID);
   logFlag(m_EnableFullE2E, "Full E2E support");
   logFlag(m_MatchFragmentsID, "Match fragments ID");
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
   // Driver expects big endian UUID
   ArrayDwordSwap(m_LocalUniqueID.data(), m_LocalUniqueID.size());
}

void TbtWmiApproveP2P::SetRemoteUniqueID(UniqueID RemoteUniqueID)
{
   m_RemoteUniqueID.clear();
   std::copy_n((PUCHAR)RemoteUniqueID.data(), sizeof(UNIQUE_ID), std::back_inserter(m_RemoteUniqueID));
   // Driver expects big endian UUID
   ArrayDwordSwap(m_RemoteUniqueID.data(), m_RemoteUniqueID.size());
}

void TbtWmiApproveP2P::SetLocalDepth(uint8_t LocalDepth)
{
   m_LocalDepth = LocalDepth;
}

void TbtWmiApproveP2P::SetPeerOs(uint32_t PeerOS)
{
   m_PeerOS = PeerOS;
}

void TbtWmiApproveP2P::SetEnableFullE2E(bool EnableFullE2E)
{
   m_EnableFullE2E = EnableFullE2E;
   logFlag(m_EnableFullE2E, "Full E2E support");
}
