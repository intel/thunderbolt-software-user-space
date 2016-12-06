/********************************************************************************
 * Thunderbolt(TM) FW update library
 * This library is distributed under the following BSD-style license:
 *
 * Copyright(c) 2016 Intel Corporation.
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

#include <exception>
#include <stdint.h>
#include <string>

namespace tbt
{

typedef uint32_t TbtStatus;

/// <summary>
/// The exception type used almost everywhere in this SDK and samples. Supports uniform error
/// message formating.
/// </summary>
/// <remarks>
/// Message property returns both the "Error: {code} {enum entry string}" string and the long
/// description given as a parameter to the c-tor or found in the message table below.
///
/// To retrieve these parts separated, use ErrorMessage() with ErrorCode property to get the
/// first component and TbtMessage property to get the second one.
/// </remarks>
class TbtException : public std::exception
{
public:
   TbtStatus ErrorCode() const noexcept { return m_errorCode; }
   const std::string& TbtMessage() const noexcept { return m_message; }
   TbtException(const char* zFile, int line, TbtStatus code, std::string message);
   TbtException(const char* zFile, int line, TbtStatus code, const char* zFmt, ...);
   TbtException(const char* zFile, int line, TbtStatus code);
   virtual ~TbtException();
   virtual const char* what() const noexcept;

private:
   std::string m_sFile;
   int m_nLine;
   TbtStatus m_errorCode;
   std::string m_message;
};

} // namespace tbt

#define TBT_THROW(...)                                            \
   do                                                             \
   {                                                              \
      throw tbt::TbtException(__FILE__, __LINE__, ##__VA_ARGS__); \
   } while (0)
