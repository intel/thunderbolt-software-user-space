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

#ifndef _ISERIALIZABLE_H_
#define _ISERIALIZABLE_H_

#include <map>
#include <vector>
#include <boost/variant.hpp>
#include <boost/blank.hpp>

typedef boost::variant<std::wstring, std::vector<uint8_t>, bool, uint32_t, uint16_t, uint8_t, boost::blank>
   SerializationValue;
// Note: The order of the values in this enum should be correlated with the definition order in the SerializationValue
// template types.
enum class SerializationType
{
   WSTRING,
   BUFFER,
   BOOL,
   UINT32,
   UINT16,
   UINT8,
   BLANK
};
typedef std::map<std::wstring, SerializationValue> PropertiesMap;

//
// Any class that derives from this interface should have a default construct (For the WmiSerializer to work).
//
class ISerializable
{
public:
   virtual ~ISerializable(){};

   // Returns a map of properties that should be saved once the object is being serialized
   virtual PropertiesMap GetWriteableProperties() const = 0;

   virtual PropertiesMap GetReadOnlyProperties() const = 0;

   virtual PropertiesMap GetAllProperties() const = 0;

   virtual std::wstring GetInstanceName() const = 0;
   // Load all the properties in the given map into the object
   virtual void LoadFromSerializationMap(const PropertiesMap& PropertiesMapToLoad) = 0;
};

#endif // !_ISERIALIZABLE_H_
