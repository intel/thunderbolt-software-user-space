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

//#include "stdafx.h"
#include <sstream>
#include <string>
#include <cstring>

#include "XDomainProperties/Properties.h"

namespace XDomainProperties
{
   //======================= DirectoryData ==================================

   Property& DirectoryData::operator[](std::string key)
   {
      if (key.length()>SIZE_OF_KEY)
      {
         throw TbtException("Key size is should be less then 8");
      }
      return m_Properties[key];
   }

   const Property& DirectoryData::operator[](std::string key) const
   {
      return m_Properties.at(key);
   }

   uint32_t DirectoryData::GetNumOfEntries() const
   {
      uint32_t counter = 0;

      for (auto ptr : m_Properties)
      {
         counter += ptr.second.GetNumOfEntries();
      }
      return counter;
   }

   size_t DirectoryData::GetSize() const
   {
      uint32_t size = 0;

      for (auto ptr : m_Properties)
      {
         size += ptr.second.GetSize();
      }
      return size + DwSizeOf<KeyValueEntry>()*GetNumOfEntries();
   }

   bool DirectoryData::IsExiest(const std::string& key) const
   {
      return m_Properties.find(key) != m_Properties.end();
   }

   //======================= ChildDirectory ==================================

   Property& ChildDirectory::operator[](const std::string& key)
   {
      return m_Data[key];
   }

   const Property& ChildDirectory::operator[](const std::string& key) const
   {
      return m_Data[key];
   }

   ChildDirectory::ChildDirectory()
   {
      m_Header.m_l = 0;
      m_Header.m_h = 0;
   }

   ChildDirectory::ChildDirectory(uuid dirctory_id) : m_Header(dirctory_id){}

   uuid ChildDirectory::GetDirectoryUUID() const
   {
      return m_Header;
   }

   uint32_t ChildDirectory::GetNumOfEntries() const
   {
      return m_Data.GetNumOfEntries();
   }

   bool ChildDirectory::IsExiest(const std::string& key) const
   {
      return m_Data.IsExiest(key);
   }

   bool operator==(const ChildDirectory& lhs, const ChildDirectory& rhs)
   {
      return (lhs.m_Data == rhs.m_Data) && (lhs.m_Header == rhs.m_Header);
   }

   bool operator==(const DirectoryData& lhs, const DirectoryData& rhs)
   {
      return lhs.m_Properties == rhs.m_Properties;
   }

   bool operator!=(const ChildDirectory& lhs, const ChildDirectory& rhs)
   {
      return !operator==(lhs, rhs);
   }

   bool operator!=(const DirectoryData& lhs, const DirectoryData& rhs)
   {
      return !operator==(lhs, rhs);
   }

   //======================= Property =====================================

   Property::Property(){}

   Property::Property(const std::vector<uint32_t>& Data)
   {
      m_Values[PropertyType::DATA] = Data;
      if (Data.size() > UINT16_MAX)
      {
         throw TbtException("data size is longer then uint16 size");
      }
   };

   Property::Property(const std::string& Text)
   {
      m_Values[PropertyType::TEXT] = Text;
      if (Text.length() + 1 > (size_t)UINT16_MAX * sizeof(uint32_t))
      {
         throw TbtException("string size is longer then uint16 size");
      }
   }

   Property::Property(const char* str)
   {
      std::string Text (str);
      m_Values[PropertyType::TEXT] = Text;
      if (Text.length() + 1 > (size_t)UINT16_MAX * sizeof(uint32_t))
      {
         throw TbtException("string size is longer then uint16 size");
      }
   }

   Property::Property(int32_t Value)
   {
      m_Values[PropertyType::VALUE] = Value;
   }

   Property::Property(const ChildDirectory& Dir)
   {
      m_Values[PropertyType::DIRECTORY] = Dir;
   }

   uint32_t Property::GetNumOfEntries() const
   {
      return m_Values.size();
   }

   ChildDirectory Property::GetDirectory()
   {
      try{
         return operator ChildDirectory();
      }
	  catch (const std::exception& ex)
      {
         throw TbtException("Error getting Directory value, probably not exists");
      }
      catch (...)
      {
         throw TbtException("Error getting Directory value, probably not exists");
      }
   }

   std::vector<uint32_t> Property::GetData()
   {
      try{
         return operator std::vector<uint32_t>();
      }
	  catch (const std::exception& ex)
      {
         throw TbtException("Error getting data value, probably not exists");
      }
      catch (...)
      {
         throw TbtException("Error getting data value, probably not exists");
      }
   }

   std::string Property::GetString() const
   {
      try{
         return operator std::string();
      }
	  catch (const std::exception& ex)
      {
         throw TbtException("Error getting string value, probably not exists");
      }
      catch (...)
      {
         throw TbtException("Error getting string value, probably not exists");
      }
   }

   int32_t Property::GetInt32() const
   {
      try{
         return operator int32_t();
      }
	  catch (const std::exception& ex)
      {
         throw TbtException("Error getting int value, probably not exists");
      }
      catch (...)
      {
         throw TbtException("Error getting int value, probably not exists");
      }
   }

   Property & Property::operator=(const ChildDirectory& Dir)
   {
      m_Values[PropertyType::DIRECTORY] = Dir;
      return *this;
   }

   Property & Property::operator=(const std::vector<uint32_t>& Data)
   {
      m_Values[PropertyType::DATA] = Data;
      return *this;
   }

   Property & Property::operator=(int32_t val)
   {
      m_Values[PropertyType::VALUE] = val;
      return *this;
   }

   Property & Property::operator=(const char* Text)
   {
      this->operator=(std::string(Text));
      return *this;
   }

   Property & Property::operator=(const std::string& Text)
   {
      m_Values[PropertyType::TEXT] = Text;
      return *this;
   }

   Property & Property::operator=(const Property & other)
   {
      m_Values = other.m_Values;
      return *this;
   }

   const Property& Property::operator[](const char* key) const
   {
      return operator[](std::string(key));
   }

   Property& Property::operator[](const char* key)
   {
      return operator[](std::string(key));
   }

   const Property& Property::operator[](const std::string& key) const
   {
      try{
         return boost::get<ChildDirectory>(m_Values.at(PropertyType::DIRECTORY))[key];
      }
	  catch (const std::exception& e)
      {
         throw std::logic_error("Property " + key + " is not a directory");
      }
   }

   Property& Property::operator[](const std::string& key)
   {
      try{
         return boost::get<ChildDirectory>(m_Values.at(PropertyType::DIRECTORY))[key];
      }
	  catch (const std::exception& e)
      {
         throw std::logic_error("Property " + key + " is not a directory");
      }
   }

   bool Property::IsExiest(const std::string& key) const
   {
      try{
         return boost::get<ChildDirectory>(m_Values.at(PropertyType::DIRECTORY)).IsExiest(key);
      }
	  catch (const std::exception& e)
      {
         throw std::logic_error("Property " + key + " is not a directory");
      }
   }

   size_t Property::GetSize() const
   {
      size_t size = 0;

      for (auto ptr : m_Values)
      {
         switch (ptr.first)
         {
         case PropertyType::DATA:
            size += boost::get<std::vector<uint32_t>>(ptr.second).size();
            break;
         case PropertyType::DIRECTORY:
            size += boost::get<ChildDirectory>(ptr.second).GetSize() + DwSizeOf<DirectoryUUID>();
            break;
         case PropertyType::VALUE:
            //included in header
            break;
         case PropertyType::TEXT:
            size += DIV_ROUND_UP(boost::get<std::string>(ptr.second).length() + 1, sizeof(uint32_t));
            break;
         default:
            break;
         }
      }
      return size;
   }

   //======================= Properties =====================================

   /**
    * this operator is for easy use with the properties structure like
    * a regulat map. (e.g. "properties["network"]["prtcid"] = 1;")
    */
   Property& Properties::operator[](const std::string& key)
   {
      m_BlockGeneration++;
      return m_RooDirectory[key];
   }

   /**
    * this operator is for easy use with the properties structure like
    * a regulat map. (e.g. "properties["network"]["prtcid"] = 1;")
    */
   const Property& Properties::operator[](const std::string& key) const
   {
      return m_RooDirectory[key];
   }

   //-----------------------Constractors-----------------------------

   /**
    * constructor - create an empty properties structure
    */
   Properties::Properties(uint32_t BusType, uint8_t version):
      m_RooDirectory(BusType, version),
      m_BlockGeneration(0)
   {
   }

    /**
     * constructor - create an properties structure with default values
     */
   Properties::Properties():
      m_RooDirectory(BUS_TYPE, VERSION),
      m_BlockGeneration(0)
   {
      m_RooDirectory["vendorid"] = 0x8086;	//intel vendor ID
      //NOTE: this is in addition to the int (0x8086) value, propety can store multiple <key,value> of different types
      //same for the next properties
      m_RooDirectory["vendorid"] = "Intel Corp.";
      m_RooDirectory["deviceid"] = 0x1;
      m_RooDirectory["deviceid"] = "THUNDERBOLT-PC"; //will be updated with real host name before being send
      m_RooDirectory["devicerv"] = 0x80000100;

      DirectoryUUID uuid = { 0xacba0048e0304fd3, 0x8fa9d08fd29c8a01 };
      m_RooDirectory["network"] = ChildDirectory(uuid);
      m_RooDirectory["network"]["prtcid"]    = 1;
      m_RooDirectory["network"]["prtcvers"]  = 1;
      m_RooDirectory["network"]["prtcrevs"]  = 1;
      m_RooDirectory["network"]["prtcstns"]  = 0;

      m_RooDirectory["authsup"] = IsAuthenticationSupported();

   }
   /**
    * doing a DW swap for the keyvalue entry
    */
   void Properties::SwapKeyStringDW(KeyValueEntry& key)
   {
      std::reverse(&key.Key[0], &key.Key[sizeof(uint32_t)]);
      std::reverse(&key.Key[sizeof(uint32_t)], &key.Key[2*sizeof(uint32_t)]);
   }

   /**
    * this function is doing DW swap and aling text to be DW align
    */
   void Properties::SwappAndDWAlignText(std::string& str)
   {
      auto size = ROUND_UP((str.length() + 1), (int)sizeof(uint32_t));
      str.resize(size, '\0');

      for (size_t i = 0; i < size/sizeof(uint32_t); i++)
      {
         std::reverse(str.begin() + i*sizeof(uint32_t), str.begin() + (i+1)*sizeof(uint32_t));
      }

   }

   /**
    * this function will fill the properties from raw data buffer that received
    * in chunk from the remote host
    */
   void Properties::BuildFromBuffer(const char* buffer, size_t size)
   {
      PropertiesBlockHeader header;
      std::stringstream stream;
      stream.write(buffer, size);
      //reading root directory header
      stream.read((char*)&header, sizeof(header));
      if (!stream)
      {
         throw TbtException("Error reading root header from buffer");
      }

      m_RooDirectory = RootDirectory(header.BusType, header.Version);

      //Passing on all the root entries and parsing
      for (uint32_t i = 0; i < header.RootDirectoryLngth / DwSizeOf<KeyValueEntry>(); i++)
      {
         KeyValueEntry propertyEntry;
         try
         {
            stream.read((char*)&propertyEntry, sizeof(propertyEntry));
            if (!stream)
            {
               throw TbtException("Error reading entry from buffer");
            }
            SwapKeyStringDW(propertyEntry);
            ParseProperty(stream, propertyEntry, (*this)[std::string(propertyEntry.Key, SIZE_OF_KEY).c_str()]);
         }
		 catch (const std::exception& e)
         {
            TbtServiceLogger::LogError("Error: Error parsing property: %s , property will be skipped", std::string(propertyEntry.Key, SIZE_OF_KEY).c_str());
         }
         catch (...)
         {
            TbtServiceLogger::LogError("Error: Error parsing property: %s , property will be skipped", std::string(propertyEntry.Key, SIZE_OF_KEY).c_str());
         }
      }
   }

   /**
    * creates the properties structure from raw data stored in vector
    */
   Properties::Properties(std::vector<uint8_t> buffer) :m_BlockGeneration(0)
   {
      BuildFromBuffer((char*)&buffer.front(), buffer.size());
   }

   /**
    * creates the properties structure from raw data stored in buffer
    */
   Properties::Properties(const char* buffer, size_t size):m_BlockGeneration(0)
   {
      BuildFromBuffer(buffer, size);
   }

   uint32_t Properties::GetBusType() const
   {
      return m_RooDirectory.GetBusType();
   }

   uint8_t Properties::GetVersion() const
   {
      return m_RooDirectory.GetVersion();
   }

   /**
    * returns num of entries that exist in the properties structure
    * (recursivly, include all sub directories in the properties structure)
    */
   uint32_t Properties::GetNumOfEntries() const
   {
      return m_RooDirectory.GetNumOfEntries();
   }

   /**
    * converts the properties structure into raw data buffer.
    * this data is going to be sent when getting properties request from remote host
    */
   std::vector<uint8_t> Properties::ToBuffer()
   {
      std::vector<uint8_t> buffer(GetSize()*sizeof(uint32_t));
      uint32_t offset = 0;
      PropertiesBlockHeader header;
      header.BusType = GetBusType();
      header.Version = GetVersion();
      header.RootDirectoryLngth = GetNumOfEntries() * DwSizeOf<KeyValueEntry>();

      WriteToBufferAndInc<PropertiesBlockHeader>(header, buffer, offset);
      uint32_t DataOffsetInDw = header.RootDirectoryLngth + DwSizeOf<PropertiesBlockHeader>();

      for (auto property : m_RooDirectory.m_Data.m_Properties)
      {
         WritePropertyToBuffer(property, buffer, offset, DataOffsetInDw);
      }
      return buffer;
   }

   //helper functions to build properties from buffer
   void Properties::ParseProperty(std::istream& stream, const KeyValueEntry& propertyEntry, Property& prop)
   {

      auto startPosition = stream.tellg();

      switch ((PropertyType)propertyEntry.Type)
      {
      case PropertyType::DATA:
         ReadData(stream, propertyEntry, prop);
         break;

      case PropertyType::DIRECTORY:
         ReadDirectory(stream, propertyEntry, prop);
         break;

      case PropertyType::TEXT:
         ReadText(stream, propertyEntry, prop);
         break;

      case PropertyType::VALUE:
         prop = propertyEntry.Value;
         break;

      case PropertyType::UNKNOWN:
      default:
         break;
      }
      stream.seekg(startPosition);
   }

   void Properties::ReadDirectory(std::istream &stream, const XDomainProperties::KeyValueEntry &propertyEntry, XDomainProperties::Property& prop)
   {
      stream.seekg(propertyEntry.Value* sizeof(uint32_t), std::ios::beg);
      DirectoryUUID uuid;
      stream.read((char*)&uuid, sizeof(uuid));

      if (!stream)
      {
         throw TbtException("Error reading entry from buffer");
      }

      ChildDirectory dir(uuid);
      KeyValueEntry childEntry;

      //not taking in acount 4 DWORDS of directory UUID
      uint32_t numEntries = (propertyEntry.Size - DwSizeOf<DirectoryUUID>()) / DwSizeOf<KeyValueEntry>();

      for (uint32_t i = 0; i < numEntries; i++)
      {
         try
         {
            stream.read((char*)&childEntry, sizeof(childEntry));
            if (!stream)
            {
               throw TbtException("Error reading entry from buffer");
            }
            SwapKeyStringDW(childEntry);
            ParseProperty(stream, childEntry, dir[std::string(childEntry.Key, SIZE_OF_KEY).c_str()]);
         }
		 catch (const std::exception& e)
         {
            TbtServiceLogger::LogError("Error: Error parsing property: %s of directory %s , property will be skipped", std::string(childEntry.Key, SIZE_OF_KEY).c_str(), std::string(propertyEntry.Key, SIZE_OF_KEY).c_str());
         }
         catch (...)
         {

         }
      }

      prop = dir;
   }

   void Properties::ReadData(std::istream &stream, const XDomainProperties::KeyValueEntry &propertyEntry, XDomainProperties::Property& prop)
   {
      stream.seekg(propertyEntry.Value * sizeof(uint32_t), std::ios::beg);

      std::istream_iterator<uint32_t> it(stream), eos;
      std::vector<uint32_t> data(propertyEntry.Size);

      for (uint32_t i = 0; i < propertyEntry.Size; i++)
      {
         if (it == eos)
         {
            std::stringstream msg;
            msg << "End of stream before reading all data after reading " << i << " Dwords out of " << propertyEntry.Size;
            throw TbtException(msg.str().c_str());
         }

         data.push_back(*it);
      }
      prop = data;
   }
   std::string Properties::ReadDWSwappedText(std::istream &stream, size_t sizeInDW)
   {
      std::string tmpstr(sizeof(uint32_t)* sizeInDW,'\0');

      for (size_t i = 0; i < sizeInDW; i++)
      {
         stream.readsome(&tmpstr.front() + i*sizeof(uint32_t), sizeof(uint32_t));
         std::reverse(tmpstr.begin() + i*sizeof(uint32_t), tmpstr.begin() + (i+1)*sizeof(uint32_t));
      }
      return tmpstr;
   }

   void Properties::ReadText(std::istream &stream, const KeyValueEntry &propertyEntry, Property& prop)
   {
      stream.seekg(propertyEntry.Value * sizeof(uint32_t), std::ios::beg);
      prop = ReadDWSwappedText(stream, propertyEntry.Size).c_str();
      //string should be null terminated, need to check here if valid in size that was given.
   }

   //helper functions for writing to buffer
   void Properties::WriteData(std::vector<uint8_t>& buffer, uint32_t& offset, KeyValueEntry& entry, const std::vector<uint32_t>& data, uint32_t& DataOffsetInDw)
   {
      entry.Size = data.size();
      entry.Value = DataOffsetInDw;
      WriteToBufferAndInc<KeyValueEntry>(entry, buffer, offset);
      WriteToBuffer(buffer, DataOffsetInDw*sizeof(uint32_t), (char*)&data.front(), entry.Size*sizeof(uint32_t));
      DataOffsetInDw += entry.Size;
   }

   void Properties::WriteDirectory(std::vector<uint8_t>& buffer, uint32_t& offset, KeyValueEntry& entry, ChildDirectory dir, uint32_t& DataOffsetInDw)
   {
      entry.Size = dir.GetNumOfEntries() * DwSizeOf<KeyValueEntry>() + DwSizeOf<DirectoryUUID>();
      entry.Value = DataOffsetInDw;
      WriteToBufferAndInc<KeyValueEntry>(entry, buffer, offset);

      uint32_t dirOffset = DataOffsetInDw*sizeof(uint32_t);
      WriteToBufferAndInc<DirectoryUUID>(dir.GetDirectoryUUID(), buffer, dirOffset);
      DataOffsetInDw += DwSizeOf<DirectoryUUID>();

      for (auto property : dir.m_Data.m_Properties)
      {
         WritePropertyToBuffer(property, buffer, dirOffset, DataOffsetInDw);
      }
      DataOffsetInDw += entry.Size - DwSizeOf<DirectoryUUID>();
   }

   void Properties::WriteValue(std::vector<uint8_t>& buffer, uint32_t& offset, KeyValueEntry& entry, int32_t data)
   {
      entry.Size = DwSizeOf<int32_t>();
      entry.Value = data;
      WriteToBufferAndInc<KeyValueEntry>(entry, buffer, offset);
   }

   void Properties::WriteText(std::vector<uint8_t>& buffer, uint32_t& offset, KeyValueEntry& entry, std::string data, uint32_t& DataOffsetInDw)
   {
      entry.Size = DIV_ROUND_UP((data.length() + 1), sizeof(uint32_t));

      SwappAndDWAlignText(data);
      entry.Value = DataOffsetInDw;

      WriteToBufferAndInc<KeyValueEntry>(entry, buffer, offset);

      WriteToBuffer(buffer, DataOffsetInDw*sizeof(uint32_t), (char*)&data.front(), entry.Size*sizeof(uint32_t));
      DataOffsetInDw += entry.Size;
   }

   template<typename T>
   void Properties::WriteToBufferAndInc(const T& entry, std::vector<uint8_t>& buffer, uint32_t& offset)
   {
      WriteToBuffer(buffer, offset, (char*)&entry, sizeof(entry));
      offset += sizeof(entry);
   }

   void Properties::WriteToBuffer(std::vector<uint8_t>& buffer, uint32_t offset, const char* src, size_t srcSize)
   {
      memcpy(&buffer.front() + offset, src, srcSize);
   }



   void Properties::WritePropertyToBuffer(const std::pair<const std::string, Property>& keyValuePair, std::vector<uint8_t>& buffer, uint32_t& offset, uint32_t& DataOffsetInDw)
   {
      for (auto prop : keyValuePair.second.m_Values)
      {
         KeyValueEntry entry;
         std::memset(&entry, 0, sizeof(entry));

         memcpy(entry.Key, &keyValuePair.first.front(), keyValuePair.first.length());

         SwapKeyStringDW(entry);

         entry.Type = (uint32_t)prop.first;

         switch (prop.first)
         {
         case PropertyType::DATA:
            WriteData(buffer, offset, entry, boost::get<std::vector<uint32_t>>(prop.second), DataOffsetInDw);
            break;
         case PropertyType::DIRECTORY:
            WriteDirectory(buffer, offset, entry, boost::get<ChildDirectory>(prop.second), DataOffsetInDw);
            break;
         case PropertyType::TEXT:
            WriteText(buffer, offset, entry, boost::get<std::string>(prop.second), DataOffsetInDw);
            break;
         case PropertyType::VALUE:
            WriteValue(buffer, offset, entry, boost::get<int32_t>(prop.second));
            break;
         case PropertyType::UNKNOWN:
         default:
            //save or throw exception?
            break;
         }
      }
   }


}
