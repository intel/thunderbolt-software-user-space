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

#include <cinttypes>
#include <tbt/tbt_fwu_err.h>
#include "Exceptions.h"
#include <string.h>
#include <string>
#include <cstdarg>

namespace
{

std::string ErrorMessage(tbt::TbtStatus code)
{
   char zBuf[50];
   snprintf(zBuf, sizeof(zBuf), "Error: 0x%" PRIx32, code);
   return std::string(zBuf);
}

} // end anonymous namespace

namespace tbt
{

TbtException::TbtException(const char* zFile, int line, TbtStatus code, std::string message)
   : std::exception(),
     m_sFile(zFile),
     m_nLine(line),
     m_errorCode(code),
     m_message("from " + std::string(zFile) + ":" + std::to_string(line) + ": " + ErrorMessage(code) + " "
               + std::string(tbt_strerror(code))
               + ". Details: "
               + message)
{
}

TbtException::TbtException(const char* zFile, int line, TbtStatus code, const char* zFmt, ...)
   : std::exception(),
     m_sFile(zFile),
     m_nLine(line),
     m_errorCode(code),
     m_message("from " + std::string(zFile) + ":" + std::to_string(line) + ": " + ErrorMessage(code) + " ")
{
   char zMsg[256];
   va_list ap;
   va_start(ap, zFmt);
   vsnprintf(zMsg, sizeof(zMsg), zFmt, ap);
   m_message += std::string(zMsg);
   va_end(ap);
}

TbtException::TbtException(const char* zFile, int line, TbtStatus code)
   : std::exception(),
     m_sFile(zFile),
     m_nLine(line),
     m_errorCode(code),
     m_message("from " + std::string(zFile) + ":" + std::to_string(line) + ": " + ErrorMessage(code) + " "
               + std::string(tbt_strerror(code)))
{
}

const char* TbtException::what() const noexcept
{
   return m_message.c_str();
}

TbtException::~TbtException()
{
}

} // namespace tbt
