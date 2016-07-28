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

   /**
    * \brief C-tor to fill the class members with data
    *
    * \param[in]   LocalRouteString
    * \param[in]   LocalUniqueID
    * \param[in]   RemoteUniqueID
    * \param[in]   PeerOS
    * \param[in]   LocalDepth
    * \param[in]   EnableFullE2E      True to tell the driver to enable full-E2E mode for better
    *                                 P2P performance (relevant if both controllers are >=AR)
    * \param[in]   MatchFragmentsID   True to tell the driver to match fragments with same ID
    *                                 to a specific packet
    *
    * \todo
    * - Init m_InstanceName
    * - Fill description
    */
   TbtWmiApproveP2P(const ROUTE_STRING& LocalRouteString,
                    const UniqueID& LocalUniqueID,
                    const UniqueID& RemoteUniqueID,
                    uint32_t PeerOS,
                    uint8_t LocalDepth,
                    bool EnableFullE2E,
                    bool MatchFragmentsID);

   virtual PropertiesMap GetWriteableProperties() const;

   virtual PropertiesMap GetReadOnlyProperties() const;

   virtual PropertiesMap GetAllProperties() const;

   virtual std::wstring GetInstanceName() const;

   virtual void LoadFromSerializationMap(const PropertiesMap&);

   void SetLocalRouteString(ROUTE_STRING LocalRouteString);

   void SetPeerOs(uint32_t PeerOS);

   void SetLocalDepth(uint8_t LocalDepth);

   /**
    * \brief Tells the driver if we want to enable full-E2E mode (relevant if both controllers are >=AR)
    *
    * \param[in]  EnableFullE2E    True to tell the driver to enable full-E2E mode
    */
   void SetEnableFullE2E(bool EnableFullE2E);

private:
   void SetLocalUniqueID(UniqueID LocalUniqueID);

   void SetRemoteUniqueID(UniqueID RemoteUniqueID);

   std::wstring m_InstanceName;
   std::vector<uint8_t> m_LocalRouteString;           // Size 2
   std::vector<uint8_t> m_LocalUniqueID;              // Size 4
   std::vector<uint8_t> m_RemoteUniqueID;             // Size 4
   uint8_t m_LocalDepth;
   uint32_t m_PeerOS;

   /// True if we want to enable full-E2E mode (relevant if both controllers are >=AR)
   bool m_EnableFullE2E = false;

   /// True if we want the P2P driver to match fragments with same ID to a specific packet
   bool m_MatchFragmentsID = false;

   /// \name Property key names
   ///@{
   /** Property key name */
   static const std::wstring InstanceName;
   static const std::wstring LocalRouteString;
   static const std::wstring LocalUniqueID;
   static const std::wstring RemoteUniqueID;
   static const std::wstring PeerOS;
   static const std::wstring LocalDepth;
   static const std::wstring EnableFullE2E;
   static const std::wstring MatchFragmentsID;
   ///@}
};

#endif // !_TBT_WMI_APPROVE_P2P_H_
