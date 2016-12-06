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
#include "boost/optional.hpp"

template <typename T>
void ignore(T&&)
{
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
std::shared_ptr<T> GetBufStruct(const std::vector<uint8_t>& Msg)
{
   try
   {
      uint8_t* Buf = new uint8_t[Msg.size()];
      // We don't use memcpy because GCC doesn't support it within a template function
      for (uint32_t i = 0; i < Msg.size(); i++)
      {
         Buf[i] = Msg[i];
      }
      std::shared_ptr<T> pStruct(reinterpret_cast<T*>(Buf), [](T* t) { delete[] reinterpret_cast<uint8_t*>(t); });
      return pStruct;
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: GetBufStruct failed: %s", e.what());
      throw;
   }
   catch (...)
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
uint32_t ControllerSettingsToEnum(bool certifiedOnly, bool overrideFirstDepth);

template <typename T>
inline size_t DwSizeOf()
{
   return sizeof(T) / sizeof(uint32_t);
}

enum class DEVICE_STATE_EVENT;
enum class DEVICE_STATE;

// Doubles the occurrences of '\' on input
std::wstring Backshlashify(const std::wstring& input);

std::wstring RemoveoDoubleSlash(const std::wstring& input);

// Different implementation for Windows and Linux
std::wstring utf8stringToWstring(const std::string&);

std::string BoolToString(bool val);

uint32_t GetDeviceIDFromControllerID(const controlleriD& ID);
ThunderboltGeneration GetGenerationFromControllerID(const controlleriD& ID);

uint32_t GetNomOfPortsFromControllerID(const controlleriD& ID);

uint32_t ControllerIDToToInt(const controlleriD& ID);

int32_t IsAuthenticationSupported();

/**
 * \brief Representing version information (used currently for NVM version)
 *
 * It handles conversion to/from string, and can hold the special string "N/A"
 * (by getting it explicitly or by default c-tor). In this case, major() returns
 * an empty 'optional'.
 *
 * The number parts are with uint32_t instead of a narrower type because the
 * internal conversion uses streams and they have to be treated as numbers, not
 * as char/wchar_t.
 */
class Version
{
public:
   static const std::wstring NA;

   Version(uint32_t major, uint32_t minor);
   explicit Version(const std::wstring& version = NA);

   boost::optional<uint32_t> major() const { return m_major; }
   boost::optional<uint32_t> minor() const { return m_minor; }
   const std::wstring& wstr() const { return m_str; }

private:
   boost::optional<uint32_t> m_major;
   boost::optional<uint32_t> m_minor;
   std::wstring m_str;
};

template <typename String = std::string, typename T>
String toHexString(T v, bool showbase = true)
{
   std::basic_ostringstream<typename String::value_type> ret;
   ret << (showbase ? std::showbase : std::noshowbase) << std::hex << v;
   return ret.str();
}

/**
 * \brief Save and automatically restore stream flags so you can set stream flags
 * without affecting later uses of the stream
 */
struct StreamFlagProtector
{
   StreamFlagProtector(std::ios_base& stream) : stream(stream), flags(stream.flags()) {}
   ~StreamFlagProtector() { stream.flags(flags); }

   std::ios_base& stream;
   std::ios_base::fmtflags flags;
};

#endif /* UTILS_H_ */
