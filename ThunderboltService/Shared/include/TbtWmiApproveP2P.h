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
class TbtWmiApproveP2P : public ISerializable
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
   std::vector<uint8_t> m_LocalRouteString; // Size 2
   std::vector<uint8_t> m_LocalUniqueID;    // Size 4
   std::vector<uint8_t> m_RemoteUniqueID;   // Size 4
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
