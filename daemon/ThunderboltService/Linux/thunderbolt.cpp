/*******************************************************************************
 *
 * Intel Thunderbolt(TM) daemon
 * Copyright(c) 2014 - 2016 Intel Corporation.
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

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <dbus-c++/dbus.h>
#pragma GCC diagnostic pop

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

DBus::BusDispatcher Dispatcher;

//All clean ups should be done here
void daemon_exit(int)
{
	ConnectionManager::GetInstance()->OnServiceDown();
}

std::string moduleVersion() try
{
    std::ifstream moduleVersionPath("/sys/module/thunderbolt/version");
    moduleVersionPath.exceptions(std::ios::badbit | std::ios::failbit);
    std::string res;
    std::getline(moduleVersionPath, res);
    return res;
}
catch(...)
{
    throw std::runtime_error("Module version query failed");
}

int main()
{
	try{
		TbtServiceLogger::LogInfo("Thunderbolt daemon version: " THUNDERBOLT_VERSION);
		TbtServiceLogger::LogInfo("Thunderbolt module version: %s", moduleVersion().c_str());
		TbtServiceLogger::LogInfo("Starting initialization");
		signal(SIGTERM, daemon_exit);
		signal(SIGINT, daemon_exit);

		DBus::default_dispatcher = &Dispatcher;
		ConnectionManager::GetInstance()->RegisterExitCallback([&](){
			Dispatcher.leave();
		});

		auto connection = DBus::Connection::SystemBus();
		//As a dbus daemon we must request server name.
		connection.request_name(THUNDERBOLT_SERVER_NAME);
		LogindManagerProxy logind_proxy(connection,
					[](){ ConnectionManager::GetInstance()->OnSystemPreShutdown(); },
					nullptr);
		TbtServiceLogger::LogInfo("Daemon is running...");
		Dispatcher.enter();
		TbtServiceLogger::LogInfo("Thunderbolt daemon stopped");
	}
	catch (const std::exception& e) {
		TbtServiceLogger::LogError("Error: Exception: %s",e.what());
		Dispatcher.leave();
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
