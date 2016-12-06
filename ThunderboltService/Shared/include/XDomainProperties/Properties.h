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

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <cstdint>

#include <boost/variant.hpp>
#include "../../../shared/general.h"
#include "Utils.h"
#include "tbtException.h"

namespace XDomainProperties
{
enum class XDOMAIN_RESPONSE_STATUS
{
   RESPONSE_RECEIVED_PARTIAL,  // part of the properties arrived (need to request the rest)
   RESPONSE_RECEIVED_ALL,      // all packets of XDomainProperties arrived
   RESPONSE_FAILED,            // fatal error on request
   RESPONSE_NOT_READY,         // remote host is busy
   RESPONSE_GENERATION_CHANGED // properties has changed from last part that arrived
};
static const size_t SIZE_OF_KEY = 8;
typedef struct
{
   uint64_t m_l;
   uint64_t m_h;
} uuid;
inline bool operator==(const uuid& lhs, const uuid& rhs)
{
   return lhs.m_l == rhs.m_l && lhs.m_h == rhs.m_h;
}
inline bool operator!=(const uuid& lhs, const uuid& rhs)
{
   return !operator==(lhs, rhs);
}

typedef struct
{
   uint32_t Version : 8;
   uint32_t BusType : 24;
   uint32_t RootDirectoryLngth;
} PropertiesBlockHeader;
typedef uuid ChildDirectoryHeader; // Maybe later uuid will be member of class ChildDirectoryHeader

/**
 * this is the header for the root directory in the properties structure
 * includes the Bustype and the version
 */
class RootDirectoryHeader
{
   friend class RootDirectory;

public:
   RootDirectoryHeader(uint32_t busType, uint8_t version) : m_BusType(busType), m_Version(version)
   {
      if ((BITFIELD_MASK(24, 31) & busType) != 0)
      {
         // TbtServiceLogger::LogError("Bus type must be in 24 bit size, actual value: %d", busType);
         throw TbtException("RootDirectoryHeader Ctor: Bus type must be in 24 bit size");
      }
   };
   friend inline bool operator==(const RootDirectoryHeader& lhs, const RootDirectoryHeader& rhs)
   {
      return lhs.m_BusType == rhs.m_BusType && lhs.m_Version == rhs.m_Version;
   }
   friend inline bool operator!=(const RootDirectoryHeader& lhs, const RootDirectoryHeader& rhs)
   {
      return !operator==(lhs, rhs);
   }

private:
   uint32_t m_BusType; // should be 24 bits in buffer .  from general & BITFIELD_MASK(startbit, endbit)
   uint8_t m_Version;
};

/**
*  Generic Key-Value entry:
*  ========================
*
*  Size (in DWORDS):
*     * Immediate value    - 1
*     * Data (leaf data)   - Data block size in DWORDS
*     * Text               - Number of DWORDS that the text take ( 1 DWORDS = 4 chars)S
*     * Directory          - Number of DWORDS of directory entries (Not include any data pointed by offsets)
*
*  Value:
*     * Immediate value    - the value itself (int32)
*     * Data (leaf data)   - leaf data offset
*     * Text (leaf text)   - leaf data offset
*     * Directory          - directory data (entries only) length
*
*/
typedef struct
{
   char Key[8];
   uint32_t Size : 16;
   uint32_t Reserved : 8;
   uint32_t Type : 8;
   int32_t Value;
} KeyValueEntry;

enum class PropertyType
{
   UNKNOWN   = 0x00,
   DATA      = 0x64,
   TEXT      = 0x74,
   VALUE     = 0x76,
   DIRECTORY = 0x44
   /* VENDOR DEFINE TYPES = 0x80 - 0xFF*/
};

typedef uuid DirectoryUUID;

class Property;
/**
 * internal directory data, can be used as a map with the [] operator for getting the
 * directory entries
 */
class DirectoryData
{
   friend class ChildDirectory;
   friend class RootDirectory;
   friend class Properties;

public:
   Property& operator[](std::string key);
   const Property& operator[](std::string key) const; // check in UT if throwing exception
   friend bool operator==(const DirectoryData& lhs, const DirectoryData& rhs);
   friend bool operator!=(const DirectoryData& lhs, const DirectoryData& rhs);
   bool Exists(const std::string& key) const;
   uint32_t GetNumOfEntries() const;
   size_t GetSize() const;

protected:
   // collection of the dirctory properties
   std::map<std::string, Property> m_Properties;
};

/**
 * for each directory that is not the root directory in the XDomain properties
 * structure.
 * each directory has its header that includes uuid for the directory
 * and properties entries ( DirectoryData )
 */
class ChildDirectory
{
   friend class Properties;

public:
   ChildDirectory();
   ChildDirectory(uuid dirctory_id);
   uuid GetDirectoryUUID() const;
   uint32_t GetNumOfEntries() const;
   Property& operator[](const std::string& key);
   const Property& operator[](const std::string& key) const;
   friend bool operator==(const ChildDirectory& lhs, const ChildDirectory& rhs);
   friend bool operator!=(const ChildDirectory& lhs, const ChildDirectory& rhs);
   bool Exists(const std::string& key) const;
   size_t GetSize() const { return m_Data.GetSize(); }
private:
   ChildDirectoryHeader m_Header; // uuid of the directory
   DirectoryData m_Data;
};

typedef boost::variant<ChildDirectory, std::vector<uint32_t>, std::string, int32_t> PropertyValue;

/**
 * single property, can be Directory/value/text/data
 */
class Property
{
   friend class Properties;

public:
   Property();
   Property(const std::vector<uint32_t>& Data);
   Property(const std::string& Text);
   Property(const char* str);
   Property(int32_t Value);
   Property(const ChildDirectory& Dir);

   operator ChildDirectory()
   {
      ChildDirectory dir = boost::get<ChildDirectory>(m_Values[PropertyType::DIRECTORY]);
      return dir;
   }; // TODO: is it right to do it this way;
   operator std::string() const { return boost::get<std::string>(m_Values.at(PropertyType::TEXT)); };
   operator int32_t() const { return boost::get<int32_t>(m_Values.at(PropertyType::VALUE)); };
   operator std::vector<uint32_t>() { return boost::get<std::vector<uint32_t>>(m_Values[PropertyType::DATA]); };

   Property& operator[](const std::string& key);

   const Property& operator[](const std::string& key) const;

   Property& operator[](const char* key);
   ;

   const Property& operator[](const char* key) const;
   ;

   Property& operator=(const Property& other);

   Property& operator=(const std::string& Text);

   Property& operator=(const char* Text);

   Property& operator=(int32_t val);

   Property& operator=(const std::vector<uint32_t>& Data); // shouldn't be ref?;

   Property& operator=(const ChildDirectory& Dir);

   int32_t GetInt32() const;

   std::string GetString() const;

   std::vector<uint32_t> GetData();

   ChildDirectory GetDirectory();

   uint32_t GetNumOfEntries() const;
   size_t GetSize() const;
   // In case of Directory
   bool Exists(const std::string& key) const;

   friend inline bool operator==(const Property& lhs, const Property& rhs) { return lhs.m_Values == rhs.m_Values; };
   friend inline bool operator!=(const Property& lhs, const Property& rhs) { return !operator==(lhs, rhs); };
private:
   std::map<PropertyType, PropertyValue> m_Values;
};

/**
 * represent the root directory of the XDomain properties structure
 * contains properties entries ( DirectoryData ) and root header (Bus type & version)
 */
class RootDirectory
{
   friend class Properties;

public:
   RootDirectory() : m_Header(0, 0) {}
   RootDirectory(uint32_t busType, uint8_t version) : m_Header(busType, version) {}

   Property& operator[](const std::string& key) { return m_Data[key]; }
   const Property& operator[](const std::string& key) const { return m_Data[key]; };
   friend inline bool operator==(const RootDirectory& lhs, const RootDirectory& rhs)
   {
      return lhs.m_Data == rhs.m_Data && lhs.m_Header == rhs.m_Header;
   };
   friend inline bool operator!=(const RootDirectory& lhs, const RootDirectory& rhs) { return !operator==(lhs, rhs); };
   uint32_t GetBusType() const { return m_Header.m_BusType; }
   uint8_t GetVersion() const { return m_Header.m_Version; }
   uint32_t GetNumOfEntries() const { return m_Data.GetNumOfEntries(); }
   bool Exists(const std::string& key) const { return m_Data.Exists(key); };
   size_t GetSize() const { return m_Data.GetSize() + DwSizeOf<RootDirectoryHeader>(); };
private:
   RootDirectoryHeader m_Header;
   DirectoryData m_Data;
};

/**
 * main class that represent the complete XDomain properties structure
 */
class Properties
{
   // Methods
public:
   Property& operator[](const std::string& key);
   const Property& operator[](const std::string& key) const;
   friend inline bool operator==(const Properties& lhs, const Properties& rhs)
   {
      return lhs.m_RooDirectory == rhs.m_RooDirectory;
   };
   friend inline bool operator!=(const Properties& lhs, const Properties& rhs) { return !operator==(lhs, rhs); };
   Properties(uint32_t BusType, uint8_t version);
   Properties();
   Properties(const char* buffer, size_t size);
   Properties(std::vector<uint8_t> buffer);
   uint32_t GetBusType() const;
   uint8_t GetVersion() const;
   uint32_t GetNumOfEntries() const;
   uint32_t GetBlockGeneration() const { return m_BlockGeneration; };
   std::vector<uint8_t> ToBuffer();
   bool Exists(const std::string& key) const { return m_RooDirectory.Exists(key); };
   size_t GetSize() const { return m_RooDirectory.GetSize(); }
private:
   void BuildFromBuffer(const char* buffer, size_t size);

   // From buffer helper functions
   static void ParseProperty(std::istream& stream,
                             const XDomainProperties::KeyValueEntry& propertyEntry,
                             XDomainProperties::Property& prop);
   static void ReadDirectory(std::istream& stream,
                             const XDomainProperties::KeyValueEntry& propertyEntry,
                             XDomainProperties::Property& prop);
   static void ReadData(std::istream& stream,
                        const XDomainProperties::KeyValueEntry& propertyEntry,
                        XDomainProperties::Property& prop);
   static void ReadText(std::istream& stream, const KeyValueEntry& propertyEntry, Property& prop);
   static std::string ReadDWSwappedText(std::istream& stream, size_t sizeinDW);
   static void SwapKeyStringDW(KeyValueEntry& key);

   // To buffer helper functions
   static void WriteData(std::vector<uint8_t>& buffer,
                         uint32_t& offset,
                         KeyValueEntry& entry,
                         const std::vector<uint32_t>& data,
                         uint32_t& DataOffsetInDw);
   static void WriteDirectory(std::vector<uint8_t>& buffer,
                              uint32_t& offset,
                              KeyValueEntry& entry,
                              ChildDirectory dir,
                              uint32_t& DataOffsetInDw);
   static void WriteValue(std::vector<uint8_t>& buffer, uint32_t& offset, KeyValueEntry& entry, int32_t data);
   static void WriteText(std::vector<uint8_t>& buffer,
                         uint32_t& offset,
                         KeyValueEntry& entry,
                         std::string data,
                         uint32_t& DataOffsetInDw);
   static void WriteToBuffer(std::vector<uint8_t>& buffer, uint32_t offset, const char* src, size_t srcSize);
   template <typename T>
   static void WriteToBufferAndInc(const T& entry, std::vector<uint8_t>& buffer, uint32_t& offset);
   static void SwappAndDWAlignText(std::string& str);
   static void WritePropertyToBuffer(const std::pair<const std::string, Property>& keyValuePair,
                                     std::vector<uint8_t>& buffer,
                                     uint32_t& offset,
                                     uint32_t& DataOffsetInDw);

   RootDirectory m_RooDirectory;
   uint32_t m_BlockGeneration;

   static const uint32_t BUS_TYPE = 0x555844;
   static const uint8_t VERSION   = 0x01;
};
}
