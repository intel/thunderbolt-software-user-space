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
#include <syslog.h>
#include <thread>
#include <sstream>
#include <stdarg.h>
#include <string.h>
#include <mutex>

// undef of Log* macros - Otherwise the preprocessor replaces the TbtServiceLogger member functions' names with these
// macros.
// These macros are redefined at the end of this file (#include "LoggerMacros.h").
#undef LogError
#undef LogWarning
#undef LogInfo
#undef LogDebug

/**
 * logger for the Thunderbolt daemon
 *
 */
class TbtServiceLogger
{
public:
   static std::mutex log_mutex;
   static void LogError(const char* file, const char* func, int line, const char* format, ...)
   {
      std::lock_guard<std::mutex> lock(TbtServiceLogger::log_mutex);
      va_list args;
      va_start(args, format);
      sendLog(LOG_ERR, file, func, line, format, args);
      va_end(args);
   }

   static void LogWarning(const char* file, const char* func, int line, const char* format, ...)
   {

      std::lock_guard<std::mutex> lock(log_mutex);
      va_list args;
      va_start(args, format);
      sendLog(LOG_WARNING, file, func, line, format, args);
      va_end(args);
   }

   static void LogInfo(const char* file, const char* func, int line, const char* format, ...)
   {

      std::lock_guard<std::mutex> lock(log_mutex);
      va_list args;
      va_start(args, format);
      sendLog(LOG_INFO, file, func, line, format, args);
      va_end(args);
   }

   static void LogDebug(const char* file, const char* func, int line, const char* format, ...)
   {

      std::lock_guard<std::mutex> lock(log_mutex);
      va_list args;
      va_start(args, format);
      sendLog(LOG_DEBUG, file, func, line, format, args);
      va_end(args);
   }

   static void SetLogLevel(int level) { s_logLevel = level; }
   static int GetLogLevel() { return s_logLevel; }

private:
   static const char* levelToString(int level)
   {
      switch (level)
      {
      case LOG_DEBUG:
         return "Debug";
      case LOG_INFO:
         return "Info";
      case LOG_WARNING:
         return "Warning";
      case LOG_ERR:
         return "Error";
      default:
         return "Unknown level";
      }
   }

   static void sendLog(int level, const char* file, const char* func, int line, const char* format, va_list& args)
   {
      if (level > s_logLevel)
      {
         return;
      }
      auto id = std::this_thread::get_id();
      std::stringstream ss;
      ss << id;
      fprintf(stderr, "%s: ", levelToString(level));
      vfprintf(stderr, format, args);
      fprintf(stderr, "      Thread: %s, func %s line %d (%s)\n", ss.str().c_str(), func, line, file);
   }

   static int s_logLevel;
};

// The ## is for omitting the comma before the __VA_ARGS__ if __VA_ARGS__ is empty
#define LogError(msg, ...) LogError(__FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define LogWarning(msg, ...) LogWarning(__FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define LogInfo(msg, ...) LogInfo(__FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
#define LogDebug(msg, ...) LogDebug(__FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
