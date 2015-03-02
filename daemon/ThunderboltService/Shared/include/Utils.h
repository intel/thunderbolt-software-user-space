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


#ifndef UTILS_H_
#define UTILS_H_
#include <memory>
#include <sstream>
#include "defines.h"
#include "MessagesWrapper.h"
#include "boost/crc.hpp"
#include "IP2PDevice.h"
#include "logger.h"
#include "tbtException.h"

template<typename T>
	void ignore(T &&) {
	}
//************************************
// Method:    GetBufStruct
// FullName:  GetBufStruct
// Access:    public 
// Returns:   
// Qualifier:
// Parameter: const std::vector<uint8_t> & Msg
//************************************
template <class T>
std::shared_ptr<T> GetBufStruct(const std::vector<uint8_t>& Msg )
{
	try
	{
		uint8_t* Buf = new uint8_t[Msg.size()];
      // We don't use memcpy because GCC doesn't support it within a template function
      for (uint32_t i = 0; i < Msg.size(); i++)
      {
         Buf[i] = Msg[i];
      }
		std::shared_ptr<T> pStruct(reinterpret_cast<T*>(Buf), [](T* t){
			delete[] reinterpret_cast<uint8_t*>(t);
		});
		return pStruct;
	}
	catch (const std::exception& e)
	{
		TbtServiceLogger::LogError("Error: GetBufStruct failed: %s",e.what());
		throw;
	}
	catch(...)
	{
		TbtServiceLogger::LogError("Error: GetBufStruct unknown exception");
		throw;
	}
}

//************************************
// Method:    StringToWString
// FullName:  StringToWString
// Access:    public 
// Returns:   std::wstring
// Qualifier:
// Parameter: const std::string & Str
//************************************
std::wstring StringToWString(const std::string& Str);

//************************************
// Method:    WStringToString
// FullName:  WStringToString
// Access:    public 
// Returns:   std::string
// Qualifier:
// Parameter: const std::wstring & Str
//************************************
std::string WStringToString(const std::wstring& Str);

//************************************
// Method:    GetTemplateClassName
// FullName:  GetTemplateClassName
// Access:    public static 
// Returns:   std::wstring - The class name of a template type
// Qualifier:
//************************************
template <class T>
std::wstring GetTemplateClassName()
{
   // The name() call returns a string of format "class <CLASS_NAME>".
   std::string TypeName = typeid(T).name();

   // Remove the "class " part from the string
   return StringToWString(TypeName.substr(6));
}

//************************************
// Method:    ArrayDwordSwap
// FullName:  ArrayDwordSwap
// Access:    public 
// Returns:   HRESULT
// Qualifier:
// Parameter: PUCHAR Array
// Parameter: LONG Size
// Swaps the bytes in the DWORDs of the given array
//************************************
void ArrayDwordSwap(uint8_t* Array, size_t Size);

//************************************
// Method:    CalculateCrc
// FullName:  CalculateCrc
// Access:    public 
// Returns:   uint32_t
// Qualifier:
// Parameter: PUCHAR Array
// Parameter: LONG Size
// Calculate the CRC of the given array
//************************************
uint32_t CalculateCrc(uint8_t* Array, size_t Size);

//************************************
// Method:    ControllerSettingsToEnum
// FullName:  ControllerSettingsToEnum
// Access:    public 
// Returns:   uint32_t
// Qualifier:
// Parameter: bool certifiedOnly
// Parameter: bool overrideFirstDepth
// Convert Flags to Uint
//************************************
uint32_t ControllerSettingsToEnum(bool certifiedOnly,bool overrideFirstDepth);

template<typename T>
inline size_t DwSizeOf()
{
	return sizeof(T) / sizeof(uint32_t);
}

enum class DEVICE_STATE_EVENT;
enum class DEVICE_STATE;

//Doubles the occurrences of '\' on input
std::wstring Backshlashify(const std::wstring& input);

std::wstring RemoveoDoubleSlash(const std::wstring& input);

// Different implementation for Windows and Linux
std::wstring utf8stringToWstring(const std::string&);

std::string BoolToString(bool val);

uint32_t GetDeviceIDFromControllerID(const controlleriD& ID);
ThunderboltGeneration GetGenerationFromControllerID(const controlleriD& ID);

uint32_t GetNomOfPortsFromControllerID(const controlleriD& ID);

uint32_t ControllerIDToToInt(const controlleriD& ID);

uint32_t IsAuthenticationSupported();

#endif
