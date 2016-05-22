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

#include "ConnectionManagerBase.h"
#include "Controller.h"
#include "Port.h"
#include "P2PDevice.h"
#include "Utils.h"
#include "ControllerSettings.h"
#include "ControllerDetails.h"
#include "tbtException.h"

ConnectionManagerBase::ConnectionManagerBase(std::shared_ptr<IControllerCommandSender>& sender) :
   m_ControllerCommandSender(sender),
   m_isPreShutdownMode(false),
   m_ControllersSettings(new ControllerSettings(false,false,false)) //TODO: put default in the settings?
{
}

/**
 * this callback is called when a physical connection is made between the two Thunderbolt hosts ( Windows <--> Linux <--> Apple)
 */
void ConnectionManagerBase::OnInterDomainConnect( const controlleriD& ControllerId,const INTER_DOMAIN_CONNECTED_NOTIFICATION& e )
{
   auto PortNum = TBT_GET_PORT_BY_LINK(e.Link);
   TbtServiceLogger::LogInfo("Inter domain connected, Controller: %s, Port %d",WStringToString(ControllerId).c_str(),PortNum);

   // Check that the controller id does exist in the controllers map
   if (m_Controllers.find(ControllerId) == m_Controllers.end())
   {
      TbtServiceLogger::LogError("Error: Inter domain connected to a controller that doesn't exist in the controllers' map in the connection manager");
      throw TbtException("Inter domain connected to a controller that doesn't exist in the controllers' map in the connection manager");
   }
   const auto pController = m_Controllers[ControllerId];

   // Create a port if it doesn't exist
   if (!pController->HasPort(PortNum))
   {
      pController->AddPort(PortNum, std::make_shared<Port>());
   }
   const auto pPort = pController->GetPortAt(PortNum);

   auto pP2PDeviceToAdd = CreateP2PDevice(ControllerId, e);

   pController->AddP2PDevice(PortNum, pP2PDeviceToAdd);

}

/**
 * this callback is called when the cable between two Thunderbolt hosts is disconnected
 */
void ConnectionManagerBase::OnInterDomainDisconnected( const controlleriD& ControllerId,const INTER_DOMAIN_DISCONNECTED_NOTIFICATION& e )
{
	TbtServiceLogger::LogInfo("Inter domain disconnected: Controller: %s", WStringToString(ControllerId).c_str());

	if (m_Controllers.find(ControllerId) == m_Controllers.end())
	{
		TbtServiceLogger::LogWarning("Warning: Inter domain disconnection at a controller that doesn't exist");
		return;
	}
	const auto pController = m_Controllers.at(ControllerId);

	uint32_t PortNum = TBT_GET_PORT_BY_LINK(e.Link);
	if (!pController->HasPort(PortNum))
	{
		TbtServiceLogger::LogWarning("Warning: Inter domain disconnection at a port that doesn't exist");
		return;
	}
	const auto pPort = pController->GetPortAt(PortNum);

   pPort->removeP2PDevice();
}

/**
 * this callback is the first message that received from the FW, it contains some details about the connected
 * controller. After this message is processed the daemon will receive messages about other devices and P2P
 */
void ConnectionManagerBase::OnDriverReadyResponse( const controlleriD& cId,const DRIVER_READY_RESPONSE& e )
{
	TbtServiceLogger::LogInfo("New controller detected: %s ", WStringToString(cId).c_str());
	//if controller is not exist we creating it (should happen only first time ever TBT device attached)
	if(m_Controllers.find(cId) == m_Controllers.end())
	{
		TbtServiceLogger::LogDebug("Create new controller class");
		m_Controllers[cId]=unique_ptr<IController>(new Controller(cId,e));
	}

	m_Controllers.at(cId)->GetControllerData()->SetSecurityLevel(e.SecurityLevel);

	// Get the current settings from the controller
	std::shared_ptr<ControllerSettings> ContSett(new ControllerSettings(!e.AllowAnyThunderboltDevice, e.Allow1stDepthDevicesToConnectAtAnyDepth, e.AllowDockDevicesToConnectAtAnyDepth));

   auto details = m_Controllers.at(cId)->GetControllerData();
   TbtServiceLogger::LogInfo("Controller device ID: %X, TBT generation: %d, port count: %d, security level: %d",
                             GetDeviceIDFromControllerID(cId),
                             details->GetGeneration(),
                             details->GetNumOfPorts(),
                             details->GetSecurityLevel());
	TbtServiceLogger::LogInfo("Controller settings: Certified only: %s, Override first depth: %s, Allow dock at any depth: %s", BoolToString(ContSett->GetCertifiedOnly()).c_str(),
																											BoolToString(ContSett->GetOverrideFirstDepth()).c_str(),
																											BoolToString(ContSett->GetAllowDockAtAnyDepth()).c_str());

	QueryDriverInformation(cId);

}

/**
 * this callback is called before the daemon is shutting down
 */
void ConnectionManagerBase::OnServiceDown()
{
	if(m_ExitCallback)
	{
		TbtServiceLogger::LogInfo("service stopped");
		m_ExitCallback();
	}
}

/**
 * returns controller by its id
 */
shared_ptr<IController> ConnectionManagerBase::GetController( const controlleriD& Cid) const
{
	shared_ptr<IController> pController = nullptr;

	try
	{
		pController = m_Controllers.at(Cid);
	}
	catch (const std::exception& e)
	{
		TbtServiceLogger::LogError("Error: Unable to find the requested controller");
		throw;
	}

   return pController;
}

/**
 * returns all controllers known to the daemon
 */
const map<controlleriD,shared_ptr<IController>>& ConnectionManagerBase::GetControllers() const
{
	return m_Controllers;
}

IControllerCommandSender& ConnectionManagerBase::GetControllerCommandSender() const
{
	return *m_ControllerCommandSender;
}

/**
 * returns p2p device by requested port
 */
const std::shared_ptr<IP2PDevice> ConnectionManagerBase::GetP2PDevice(controlleriD cId, uint32_t portNum) const
{
   return m_Controllers.at(cId)->GetPortAt(portNum)->getP2PDevice();
}

// FIXME: Remove duplicate code
std::shared_ptr<IP2PDevice> ConnectionManagerBase::GetP2PDevice(controlleriD cId, uint32_t portNum)
{
   return m_Controllers.at(cId)->GetPortAt(portNum)->getP2PDevice();
}

/**
 * registering exit callback that will be executed to stop the daemon activity.
 * usually used to exit third party event loop
 */
void ConnectionManagerBase::RegisterExitCallback(std::function<void()> ExitCallback)
{
   m_ExitCallback = ExitCallback;
}

/**
 * this callback will be executed before the system is shutting down or restarted.
 * Note: will not be called when daemon only is restarted / stoped
 */
void ConnectionManagerBase::OnSystemPreShutdown()
{
   try
   {
	   //setting pre shutdown mode
	   setPreShutdownMode();

      for (auto controller: m_Controllers) {
         for (auto port: controller.second->GetPorts()) {
            if (port.second->hasP2PDevice())
            {
               port.second->getP2PDevice()->OnSystemPreShutdown();
            }
         }
      }
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: Failed sending properties change notification %s",e.what());
	  throw;
   }
   catch(...)
   {
      TbtServiceLogger::LogError("Error: Failed sending properties change notification");
      throw;
   }

}

/**
 * returns the policy for all the controllers
 */
std::shared_ptr<ControllerSettings> ConnectionManagerBase::GetControllersSettings() const
{
	return m_ControllersSettings;
}

/**
 * setting policy for all the controllers
 */
void ConnectionManagerBase::SetControllersSettings(const std::shared_ptr<ControllerSettings> Settings)
{
	m_ControllersSettings = Settings;
}

#include "GenlWrapper.h"

/**
 * this function is getting the information from the driver
 */
void ConnectionManagerBase::QueryDriverInformation(const controlleriD& Cid)
{
	using std::wstring;
	using std::to_wstring;

	try
	{
      GenlWrapper::Instance().send_message_sync(ControllerIDToToInt(Cid),NHI_CMD_QUERY_INFORMATION);
	}
	catch (const std::exception& e)
	{
		TbtServiceLogger::LogError("Error: ConnectionManagerBase::QueryDriverInformation failed! Error : %s", e.what());
	}
	catch (...)
	{
		TbtServiceLogger::LogError("Error: ConnectionManagerBase::QueryDriverInformation failed! Error : Unknown error");
	}
}
