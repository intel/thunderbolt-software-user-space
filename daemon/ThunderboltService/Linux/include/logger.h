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
#pragma once
#include <string>
#include <syslog.h>
#include <thread>
#include <sstream>
#include <stdarg.h>
#include "config.h"
#include <string.h>
#include <mutex>

// undef of Log* macros - Otherwise the preprocessor replaces the TbtServiceLogger member functions' names with these macros.
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
		auto id = std::this_thread::get_id();
		std::stringstream ss;
		ss << id;
                printf("%s: ", levelToString(level));
		vprintf(format, args);
                printf("      Thread: %s, func %s line %d (%s)", ss.str().c_str(), func, line, file);
		printf("\n");
		fflush(stdout);

	}
};


// The ## is for omitting the comma before the __VA_ARGS__ if __VA_ARGS__ is empty
#define LogError(msg, ...) LogError(__FILE__,__func__,__LINE__ ,msg, ##__VA_ARGS__)
#define LogWarning(msg, ...) LogWarning(__FILE__,__func__,__LINE__ ,msg, ##__VA_ARGS__)
#define LogInfo(msg, ...) LogInfo(__FILE__,__func__,__LINE__ ,msg, ##__VA_ARGS__)
#define LogDebug(msg, ...) LogDebug(__FILE__,__func__,__LINE__ ,msg, ##__VA_ARGS__)
