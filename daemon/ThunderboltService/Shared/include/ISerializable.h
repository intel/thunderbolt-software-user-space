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


#ifndef _ISERIALIZABLE_H_
#define _ISERIALIZABLE_H_

#include <map>
#include <vector>
#include <boost/variant.hpp>
#include <boost/blank.hpp>

typedef boost::variant<std::wstring, std::vector<uint8_t>, bool, uint32_t, uint16_t, uint8_t, boost::blank> SerializationValue;
// Note: The order of the values in this enum should be correlated with the definition order in the SerializationValue template types.
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

	virtual std::wstring GetInstanceName()const=0;
	// Load all the properties in the given map into the object
	virtual void LoadFromSerializationMap(const PropertiesMap& PropertiesMapToLoad) = 0;
};


#endif // !_ISERIALIZABLE_H_
