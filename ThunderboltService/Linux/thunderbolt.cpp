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

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tbtdbus.h"

#include <memory>
#include <string>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include "ConnectionManager.h"
#include "logger.h"
#include "tbtException.h"
#include "logind_manager_proxy.h"
#include "config.h"
#include "version.h"
#include "GenlWrapper.h"

#include "DBusController.h"
#include "DBusControllers.h"

#include <stdio.h>

// All clean ups should be done here
// ^^^^^^^ No, normally clean-up should not be done from within a signal
// handler!  The signal may have interrupted something sensitive, such as
// a data structure manipulation.  If the signal handler then calls into
// code that touches the same data structure, then undefined behavior could
// result.
//
// An example is any code that uses malloc or free (or new or delete):
// there is a linked list (or other, more complex data structure) underlying
// those functions, and those may be in an intermediate state on entry into the
// signal handler.  Hence the signal handler may not call anything that calls
// down into those heap management functions.
void daemon_exit(int)
{
   // This is double-bad: not only does it call into code that calls
   // a heap allocation function, it also may throw an exception.
   // Throwing out of a signal handler is a bad idea.
   ConnectionManager::GetInstance()->OnServiceDown(); // FIXME: not safe
}

std::string moduleVersion() try
{
   std::ifstream moduleVersionPath("/sys/module/thunderbolt_icm/version");
   moduleVersionPath.exceptions(std::ios::badbit | std::ios::failbit);
   std::string res;
   std::getline(moduleVersionPath, res);
   return res;
}
catch (...)
{
   throw std::runtime_error("Module version query failed");
}

DBus::BusDispatcher Dispatcher;

extern "C" void dbus_shutdown(void);

int main(int argc, char** argv)
{
   int level = LOG_DEBUG; // Default log level.

   for (int argIdx = 1; argIdx < argc; ++argIdx)
   {
      if (strncmp(argv[argIdx], "-q", 2) == 0 || strcmp(argv[argIdx], "--quiet") == 0)
      {
         level = -1;
      }
      else if (strncmp(argv[argIdx], "-v", 2) == 0 || strcmp(argv[argIdx], "--verbose") == 0)
      {
         if (level == -1)
         {
            level = LOG_ERR;
         }
         else
         {
            ++level;
         }
         if (argv[argIdx][0] == '-' && argv[argIdx][1] == 'v')
         {
            char* p = &argv[argIdx][2];
            while (*p == 'v')
            {
               ++level;
               ++p;
            }
            if (*p != 0)
            {
               fprintf(stderr, "unknown argument '%s'\n", argv[argIdx]);
               return 2;
            }
         }
      }
      else
      {
         fprintf(stderr, "unknown argument '%s'\n", argv[argIdx]);
         return 2;
      }
   }
   if (level != -1)
   {
      TbtServiceLogger::SetLogLevel(level);
   }
   try
   {
      TbtServiceLogger::LogInfo("Thunderbolt daemon version: " THUNDERBOLT_VERSION);
      TbtServiceLogger::LogInfo("Thunderbolt module version: %s", moduleVersion().c_str());
      TbtServiceLogger::LogInfo("Starting initialization");
      signal(SIGTERM, daemon_exit);
      signal(SIGINT, daemon_exit);

      DBus::default_dispatcher = &Dispatcher;
      auto connection          = DBus::Connection::SystemBus();

      // As a dbus daemon we must request server name.
      connection.request_name(THUNDERBOLT_SERVER_NAME);

      auto cmgr = ConnectionManager::GetInstance();
      cmgr->SetDBusConnection(&connection);
      cmgr->RegisterExitCallback([&]() { Dispatcher.leave(); });

      DBusControllers controllersAPI(connection);
      LogindManagerProxy logind_proxy(
         connection, []() { ConnectionManager::GetInstance()->OnSystemPreShutdown(); }, nullptr);
      TbtServiceLogger::LogInfo("Daemon is running...");
      Dispatcher.enter();
      TbtServiceLogger::LogInfo("Thunderbolt daemon stopped");
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: Exception: %s", e.what());
      Dispatcher.leave();
      return 1;
   }

   // Running this here causes error messages to spam the log.  We need
   // to wait for the other thread to shut down before running this.
   // dbus_shutdown();
   return 0;
}
