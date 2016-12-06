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

#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>
#include <sstream>
#include <future>
#include <unistd.h>
#include <stdlib.h>
#include <mutex>
#include "LinuxControllerCommandSender.h"
#include "P2PDevice.h"
#include "TbtWmiMessageToFW.h"

#include "GenetlinkInterface.h"
#include "logger.h"
#include "tbtException.h"
#include "GenlWrapper.h"
#include "MessageFromFwEvent.h"
#include "ConnectionManager.h"
#include "Utils.h"
#include "TbtWmiApproveP2P.h"
#include "TbtWmiMailboxCommand.h"
#include "ControllerDetails.h"
#include "DriverCommandResponseLock.h"

/**
 * registering received event with the netlink wrapper
 */
LinuxControllerCommandSender::LinuxControllerCommandSender()
{
   GenlWrapper::Instance().register_events_callback(LinuxControllerCommandSender::OnEvent);
}

/**
 * this is the receiving callback for notifications and messages from driver,
 * every message that arrives from driver/FW is going through this callback
 */
void LinuxControllerCommandSender::OnEvent(uint32_t controller_id, PDF_VALUE pdf, const std::vector<uint8_t>& data)
{
   TbtServiceLogger::LogDebug("LinuxControllerCommandSender::OnEvent entry, PDF: %d", pdf);
   try
   {
      char buffer[100];
      int cx;

      // converting uint to string in its Hex form with 8 digits
      cx = snprintf(buffer, 100, "%.8x", controller_id);
      if (ConnectionManager::GetInstance()->isPreShutdownMode())
      {
         TbtServiceLogger::LogDebug("System is shutting down, ignoring message");
         return;
      }
      const controlleriD cId(buffer, buffer + cx + 1);

      switch (pdf)
      {

      case PDF_FW_TO_SW_RESPONSE:
         TbtServiceLogger::LogDebug("Message from FW: PDF_FW_TO_SW_RESPONSE, Controller: %x", controller_id);
         MessageFromFwEvent::HandleFwToSwResponse(cId, data);
         break;
      case PDF_FW_TO_SW_NOTIFICATION:
         TbtServiceLogger::LogDebug("Message from FW: PDF_FW_TO_SW_NOTIFICATION, Controller: %x", controller_id);
         MessageFromFwEvent::HandleFwToSwNotification(cId, data);
         break;
      case PDF_INTER_DOMAIN_REQUEST:
         TbtServiceLogger::LogDebug("Message from FW: PDF_INTER_DOMAIN_REQUEST, Controller: %x", controller_id);
         ConnectionManager::GetInstance()
            ->GetP2PDevice(
               cId,
               TBT_GET_PORT_BY_LINK(GetBufStruct<XDOMAIN_PROPERTIES_READ_RESPONSE>(data)->RouteString.Level0PortNum))
            ->HandleInterDomainRequest(data);
         break;
      case PDF_INTER_DOMAIN_RESPONSE:
         TbtServiceLogger::LogDebug("Message from FW: PDF_INTER_DOMAIN_RESPONSE, Controller: %x", controller_id);
         ConnectionManager::GetInstance()
            ->GetP2PDevice(
               cId,
               TBT_GET_PORT_BY_LINK(GetBufStruct<XDOMAIN_PROPERTIES_READ_RESPONSE>(data)->RouteString.Level0PortNum))
            ->HandleInterDomainResponse(data);
         break;
      case PDF_ERROR_NOTIFICATION:
         TbtServiceLogger::LogError("Error: Message from FW: PDF_ERROR_NOTIFICATION, Controller: %x", controller_id);
         break;
      case PDF_DRIVER_QUERY_DRIVER_INFORMATION: // NOTE: currently only message from driver is Query information
      {
         TbtServiceLogger::LogInfo("Netlink message received from Driver");
         QueryDriverInformation driver_information = *reinterpret_cast<const QueryDriverInformation*>(&data.front());

         // updating controller details with driver and device information
         ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->SetDriverVersion(
            StringToWString(driver_information.driver_version));
         ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->SetGeneration(
            GetGenerationFromControllerID(cId));
         ConnectionManager::GetInstance()->GetController(cId)->GetControllerData()->SetNumOfPorts(
            GetNomOfPortsFromControllerID(cId));
         ConnectionManager::GetInstance()->GetController(cId)->SetDmaPort(driver_information.dma_port);
         ConnectionManager::GetInstance()->GetController(cId)->SetNvmVersionOffset(driver_information.nvm_offset);
         ConnectionManager::GetInstance()->GetController(cId)->SetSupportsFullE2E(driver_information.supportsFullE2E);
         break;
      }
      case PDF_WRITE_CONFIGURATION_REGISTERS:
      {
         TbtServiceLogger::LogDebug("Message from FW: PDF_WRITE_CONFIGURATION_REGISTERS, Controller: %s",
                                    WStringToString(cId).c_str());
         ConnectionManager::GetInstance()->GetController(cId)->GetFWUpdateResponseLock().UnlockWithResult(0);

         break;
      }
      case PDF_READ_CONFIGURATION_REGISTERS:
      {
         TbtServiceLogger::LogDebug("Message from FW: PDF_READ_CONFIGURATION_REGISTERS, Controller: %s",
                                    WStringToString(cId).c_str());
         LinuxControllerCommandSender::HandleFwToSwReadConfigurationRegistersResponse(cId, data);
         break;
      }
      case PDF_FW_IS_IN_SAFE_MODE_NOTIFICATION:
      {
         TbtServiceLogger::LogDebug("Message from FW: Controller %s is in safe mode.", WStringToString(cId).c_str());
         ConnectionManager::GetInstance()->OnFwIsInSafeMode(cId);
         break;
      }

      default:
         TbtServiceLogger::LogWarning("Warning: Message from FW: Unknown message PDF = %x", pdf);
         break;
      }

      TbtServiceLogger::LogInfo("Netlink message received");
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError(
         "Error: LinuxControllerCommandSender::OnEvent failed, PDF: %d, (Exception: %s)", pdf, e.what());
   }
   TbtServiceLogger::LogDebug("LinuxControllerCommandSender::OnEvent exit, PDF: %d", pdf);
}

void LinuxControllerCommandSender::HandleFwToSwReadConfigurationRegistersResponse(controlleriD cId,
                                                                                  const std::vector<uint8_t>& Msg)
{

   std::shared_ptr<READ_CONFIGURATION_REGISTERS_RESPONSE> pReadConfigurationRegistersResponse =
      GetBufStruct<READ_CONFIGURATION_REGISTERS_RESPONSE>(Msg);

   switch (pReadConfigurationRegistersResponse->ConfigurationSpace)
   {
   case CIO_CONFIGURATION_SPACE::PORT_CONFIG_SPACE:
      switch (pReadConfigurationRegistersResponse->DWIndex)
      {
      case MAIL_IN: // flash authe
         ConnectionManager::GetInstance()->GetController(cId)->GetFWUpdateResponseLock().UnlockWithResult(
            pReadConfigurationRegistersResponse->ReadData.Operation.NotCompleted);
         break;
      case MAIL_OUT:
      {
         auto statusForCommand = pReadConfigurationRegistersResponse->ReadData.StatusResponse.StatusForCommand;
         switch (statusForCommand)
         {
         case NON_ACTIVE_FLASH_REGION_WRITE_REQUEST_CODE:
         case FLASH_REGION_READ_REQUEST_CODE:
         case FLASH_UPDATE_AUTHENTICATE_REQUEST_CODE:
         {
            auto status = pReadConfigurationRegistersResponse->ReadData.StatusResponse.Status;
            if (status != MAILBOX_RESPONSE_CODE::COMMAND_COMPLETED_SUCCESS_RESPONSE_CODE)
            {
               TbtServiceLogger::LogInfo("Error from FW. Command: %u, Status: %u, Controller: %s",
                                         statusForCommand,
                                         status,
                                         WStringToString(cId).c_str());
            }
            ConnectionManager::GetInstance()->GetController(cId)->GetFWUpdateResponseLock().UnlockWithResult(status);
         }
         break;
         case POWER_CYCLE_REQUEST_CODE:
         // FIXME: mark that flash done
         default:
            break;
         }
      }
      break;
      case MAIL_DATA_BASE:
         ConnectionManager::GetInstance()->GetController(cId)->GetFWUpdateResponseLock().UnlockWithResult(
            pReadConfigurationRegistersResponse->ReadData.Mail.Data[0]);
         break;
      default:
         TbtServiceLogger::LogWarning("Warning: Unhandled READ_CONFIGURATION_REGISTERS_RESPONSE for PORT_CONFIG_SPACE");
         break;
      }
      break;

   case CIO_CONFIGURATION_SPACE::DEVICE_CONFIG_SPACE:
      ConnectionManager::GetInstance()->GetController(cId)->GetFWUpdateResponseLock().UnlockWithResult(
         pReadConfigurationRegistersResponse->ReadData.Mail.Data[0]);
      break;

   default:
      TbtServiceLogger::LogWarning("Warning: READ_CONFIGURATION_REGISTERS_RESPONSE for unhandled config space");
      break;
   }
}
/**
 * sending command to driver
 */
void LinuxControllerCommandSender::SendDriverCommand(const controlleriD& cID, SW_TO_FW_INMAILCMD_CODE cmd) const
{
   std::shared_ptr<TbtWmiMailboxCommand> msg = std::make_shared<TbtWmiMailboxCommand>();
   msg->SetCommand(cmd);
   GenlWrapper::Instance().send_message_sync(ControllerIDToToInt(cID), NHI_CMD_MAILBOX, msg);
}

void LinuxControllerCommandSender::SendPowerCycleCommand(const controlleriD& cID,
                                                         int dmaPort,
                                                         const ROUTE_STRING& routeString) const
{
   TbtServiceLogger::LogInfo(
      "Performing power reset for controller: %s, DMA port=%d", WStringToString(cID).c_str(), dmaPort);

   // Init write configurations struct
   WRITE_CONFIGURATION_REGISTERS_REQUEST PowerCycleRequest;
   memset(&PowerCycleRequest, 0, sizeof(WRITE_CONFIGURATION_REGISTERS_REQUEST));

   // DWORD 0-1
   PowerCycleRequest.RouteString = routeString;

   // DWORD 2
   PowerCycleRequest.DWIndex = MAIL_IN;
   PowerCycleRequest.Length =
      (sizeof(PowerCycleRequest.WriteData.PowerCycleRequest) - sizeof(uint32_t)) / sizeof(uint32_t);
   PowerCycleRequest.Port               = dmaPort;
   PowerCycleRequest.ConfigurationSpace = PORT_CONFIG_SPACE;
   PowerCycleRequest.SequenceNumber     = 1;

   // DWORD 3-4
   PowerCycleRequest.WriteData.PowerCycleRequest.Command          = POWER_CYCLE_REQUEST_CODE;
   PowerCycleRequest.WriteData.PowerCycleRequest.Crc              = 0; // Will be filled by the wmi serializer
   PowerCycleRequest.WriteData.PowerCycleRequest.OperationRequest = 1;

   uint32_t ResetCommandLength = FIELD_OFFSET(WRITE_CONFIGURATION_REGISTERS_REQUEST, WriteData)
                                 + sizeof(PowerCycleRequest.WriteData.PowerCycleRequest);
   TbtServiceLogger::LogDebug("SEND WRITE_CONFIGURATION_REGISTERS_REQUEST POWER CYCLE REQUEST with port = 0x%x "
                              "configspace = 0x%x seqno = 1 Command = 0x%x OperationRequest = 1, FlashRegionRead "
                              "overall = 0x%" PRIx32 "",
                              dmaPort,
                              PORT_CONFIG_SPACE,
                              POWER_CYCLE_REQUEST_CODE,
                              (*(uint32_t*)(&PowerCycleRequest.WriteData.PowerCycleRequest)));
   SendTbtWmiMessageToFW(cID, (PUCHAR)&PowerCycleRequest, ResetCommandLength, PDF_WRITE_CONFIGURATION_REGISTERS);
}

/**
 * sending message to driver
 */
void LinuxControllerCommandSender::SendTbtWmiMessageToFW(const controlleriD& Cid,
                                                         uint8_t* message,
                                                         size_t messageSize,
                                                         PDF_VALUE pdf) const
{
   std::shared_ptr<TbtWmiMessageToFW> msg = std::make_shared<TbtWmiMessageToFW>();
   msg->SetMessage(static_cast<unsigned char*>(message), messageSize);
   msg->SetPdf(pdf);
   GenlWrapper::Instance().send_message_sync(ControllerIDToToInt(Cid), NHI_CMD_MSG_TO_ICM, msg);
}

/**
 * this message is sending to driver after exchance P2P XDomain properties and
 * verifying that the remote host support Thunderbolt IP.
 * after driver receive this message it will create an Ethernet adapter for the
 * Thunderbolt P2P adapter
 */
void LinuxControllerCommandSender::SendTbtWmiApproveP2P(const IP2PDevice& Device) const
{
   TbtServiceLogger::LogInfo("Send approve P2P command for %s", Device.RemoteHostName().c_str());
   auto msg = std::make_shared<TbtWmiApproveP2P>(Device.LocalRouteString(),
                                                 Device.LocalHostRouterUniqueID(),
                                                 Device.RemoteRouterUniqueID(),
                                                 0xFFFFF,
                                                 Device.Depth(),
                                                 Device.GetEnableFullE2E(),
                                                 Device.GetMatchFragmentsID());

   GenlWrapper::Instance().send_message_sync(
      ControllerIDToToInt(Device.ControllerID()), NHI_CMD_APPROVE_TBT_NETWORKING, msg);
   TbtServiceLogger::LogDebug("Approve P2P message has been sent to the driver");
}

TBTDRV_STATUS LinuxControllerCommandSender::ReadBlockFromFlash(const controlleriD& cID,
                                                               std::vector<uint8_t>& destination_buffer,
                                                               uint32_t source_offset,
                                                               uint32_t bytes_count,
                                                               const ROUTE_STRING& routeString,
                                                               int dmaPort) const
{
   try
   {
      WRITE_CONFIGURATION_REGISTERS_REQUEST ReadBlockRequest = {};

      // Send read request
      ReadBlockRequest.RouteString = routeString;
      ReadBlockRequest.DWIndex     = MAIL_IN;
      ReadBlockRequest.Length =
         (sizeof(ReadBlockRequest.WriteData.FlashRegionRead) - sizeof(uint32_t)) / sizeof(uint32_t);
      ReadBlockRequest.Port                              = dmaPort;
      ReadBlockRequest.ConfigurationSpace                = PORT_CONFIG_SPACE;
      ReadBlockRequest.SequenceNumber                    = 1;
      ReadBlockRequest.WriteData.FlashRegionRead.Command = FLASH_REGION_READ_REQUEST_CODE;
      ReadBlockRequest.WriteData.FlashRegionRead.DWCount =
         DIV_ROUND_UP(bytes_count + source_offset % sizeof(uint32_t), sizeof(uint32_t)); // dword_count must be <= 16
      ReadBlockRequest.WriteData.FlashRegionRead.FlashAddress     = source_offset / sizeof(uint32_t);
      ReadBlockRequest.WriteData.FlashRegionRead.OperationRequest = 1;
      ReadBlockRequest.WriteData.FlashRegionRead.Crc              = 0;

      uint32_t ReadRequestLength = FIELD_OFFSET(WRITE_CONFIGURATION_REGISTERS_REQUEST, WriteData)
                                   + sizeof(ReadBlockRequest.WriteData.FlashRegionRead);

      TbtServiceLogger::LogDebug("SEND WRITE_CONFIGURATION_REGISTERS_REQUEST with port = 0x%x configspace = 0x%x seqno "
                                 "= 1 Command = 0x%x DWCount = 0x%x FlashAddress = 0x%x OperationRequest = 1, "
                                 "FlashRegionRead overall = 0x%" PRIx32 "",
                                 dmaPort,
                                 PORT_CONFIG_SPACE,
                                 FLASH_REGION_READ_REQUEST_CODE,
                                 ReadBlockRequest.WriteData.FlashRegionRead.DWCount,
                                 ReadBlockRequest.WriteData.FlashRegionRead.FlashAddress,
                                 (*(uint32_t*)(&ReadBlockRequest.WriteData.FlashRegionRead)));
      SendTbtWmiMessageToFW(cID, (PUCHAR)&ReadBlockRequest, ReadRequestLength, PDF_WRITE_CONFIGURATION_REGISTERS);

      if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse())
      {
         return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
      }

      // check mail_in done
      READ_CONFIGURATION_REGISTERS_REQUEST CheckFinishRequest = {};
      CheckFinishRequest.RouteString                          = routeString;
      CheckFinishRequest.DWIndex                              = MAIL_IN;
      CheckFinishRequest.Length                               = 1;
      CheckFinishRequest.Port                                 = dmaPort;
      CheckFinishRequest.ConfigurationSpace                   = PORT_CONFIG_SPACE;
      CheckFinishRequest.SequenceNumber                       = 1;

      ReadRequestLength     = sizeof(CheckFinishRequest);
      uint32_t NotCompleted = 1;
      int retries           = 0;
      // wait for mail_out to be ready
      while (NotCompleted == 1) // define retries timeout
      {
         SendTbtWmiMessageToFW(cID, (PUCHAR)&CheckFinishRequest, ReadRequestLength, PDF_READ_CONFIGURATION_REGISTERS);
         if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse(
                &NotCompleted)
             || retries++ == 5)
         {
            return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
         }
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      if (static_cast<TBTDRV_STATUS>(NotCompleted) != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
      {
         return static_cast<TBTDRV_STATUS>(NotCompleted);
      }

      // check in mail_out status StatusResponse
      CheckFinishRequest.DWIndex     = MAIL_OUT;
      MAILBOX_RESPONSE_CODE response = MAILBOX_RESPONSE_CODE::GENERAL_ERROR_RESPONSE_CODE;

      SendTbtWmiMessageToFW(cID, (PUCHAR)&CheckFinishRequest, ReadRequestLength, PDF_READ_CONFIGURATION_REGISTERS);
      if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse(
             (uint32_t*)&response))
      {
         return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
      }
      if (response != MAILBOX_RESPONSE_CODE::COMMAND_COMPLETED_SUCCESS_RESPONSE_CODE)
      {
         return static_cast<TBTDRV_STATUS>(response);
      }

      CheckFinishRequest.DWIndex = MAIL_DATA_BASE;
      SendTbtWmiMessageToFW(cID, (PUCHAR)&CheckFinishRequest, ReadRequestLength, PDF_READ_CONFIGURATION_REGISTERS);
      uint32_t resultInDW = 0;
      if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse(&resultInDW))
      {
         return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
      }
      destination_buffer.clear();
      std::copy_n((uint8_t*)&resultInDW, sizeof(uint32_t), std::back_inserter(destination_buffer));
      // memcpy(&destination_buffer.front(), &resultInDW, sizeof(uint32_t)); //need to be change to support more then
      // one byte
      return TBTDRV_STATUS::SUCCESS_RESPONSE_CODE;
   }
   catch (...)
   {
      // We used to translate this into
      // TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE.  But we would prefer for
      // what() to propagate upward a level to the D-Bus layer, where it can
      // be passed onward to the client.
      throw;
   }
}

TBTDRV_STATUS LinuxControllerCommandSender::WriteToFlash(const controlleriD& cID,
                                                         uint8_t* source,
                                                         uint32_t destination_offset,
                                                         FLASH_REGION destination_region,
                                                         uint32_t dword_count,
                                                         const ROUTE_STRING& routeString,
                                                         int dmaPort) const
{
   try
   {
      // Init write configurations struct
      WRITE_CONFIGURATION_REGISTERS_REQUEST NVMWriteRequest = {};

      // Send NVM flash data
      NVMWriteRequest.RouteString        = routeString;
      NVMWriteRequest.DWIndex            = MAIL_DATA_BASE;
      NVMWriteRequest.Length             = dword_count;
      NVMWriteRequest.Port               = dmaPort;
      NVMWriteRequest.ConfigurationSpace = PORT_CONFIG_SPACE;
      NVMWriteRequest.SequenceNumber     = 1;

      std::copy_n(
         source, dword_count * sizeof(uint32_t), reinterpret_cast<uint8_t*>(NVMWriteRequest.WriteData.Mail.Data));

      uint32_t NVMBlockDataLength =
         FIELD_OFFSET(WRITE_CONFIGURATION_REGISTERS_REQUEST, WriteData) + sizeof(uint32_t) * (dword_count + 1);
      SendTbtWmiMessageToFW(cID, (PUCHAR)&NVMWriteRequest, NVMBlockDataLength, PDF_WRITE_CONFIGURATION_REGISTERS);

      if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse())
      {
         TbtServiceLogger::LogError("Error: WriteToFlash failed on timeout(writing data). "
                                    "WRITE_CONFIGURATION_REGISTERS_REQUEST:MAIL_DATA_BASE, DW count: %d ",
                                    dword_count);
         return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
      }

      // Send commit command
      NVMWriteRequest.DWIndex = MAIL_IN;
      NVMWriteRequest.Length =
         (sizeof(NVMWriteRequest.WriteData.NonActiveFlashRegionWrite) - sizeof(uint32_t)) / sizeof(uint32_t);
      NVMWriteRequest.WriteData.NonActiveFlashRegionWrite.OperationRequest = 1;
      NVMWriteRequest.WriteData.NonActiveFlashRegionWrite.CssHeader =
         (destination_region == FLASH_REGION::CSS_HEADER) ? 1 : 0;
      NVMWriteRequest.WriteData.NonActiveFlashRegionWrite.FlashAddress = destination_offset;
      NVMWriteRequest.WriteData.NonActiveFlashRegionWrite.HighDW       = dword_count - 1;
      NVMWriteRequest.WriteData.NonActiveFlashRegionWrite.Command      = NON_ACTIVE_FLASH_REGION_WRITE_REQUEST_CODE;
      NVMWriteRequest.WriteData.NonActiveFlashRegionWrite.Crc          = 0; // Will be filled by the wmi serialize

      NVMBlockDataLength = FIELD_OFFSET(WRITE_CONFIGURATION_REGISTERS_REQUEST, WriteData)
                           + sizeof(NVMWriteRequest.WriteData.NonActiveFlashRegionWrite);

      // TbtServiceLogger::LogInfo("Write to offset: %d", destination_offset);
      SendTbtWmiMessageToFW(cID, (PUCHAR)&NVMWriteRequest, NVMBlockDataLength, PDF_WRITE_CONFIGURATION_REGISTERS);
      if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse())
      {
         TbtServiceLogger::LogError("Error: WriteToFlash failed. "
                                    "WRITE_CONFIGURATION_REGISTERS_REQUEST:MAIL_IN:NON_ACTIVE_FLASH_REGION_WRITE_"
                                    "REQUEST_CODE, offset: %d",
                                    destination_offset);
         return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
      }

      // check if block flashing done
      READ_CONFIGURATION_REGISTERS_REQUEST CheckFinishRequest = {};
      CheckFinishRequest.RouteString                          = routeString;
      CheckFinishRequest.DWIndex                              = MAIL_IN;
      CheckFinishRequest.Length                               = 1; // FIXME: Check with Amir polish doesnt have it
      CheckFinishRequest.Port                                 = dmaPort;
      CheckFinishRequest.ConfigurationSpace                   = PORT_CONFIG_SPACE;
      CheckFinishRequest.SequenceNumber                       = 1;

      NVMBlockDataLength    = sizeof(CheckFinishRequest);
      uint32_t NotCompleted = 1;
      int retries           = 0;
      // wait for flash to finish
      while (NotCompleted == 1) // define retries timeout
      {
         SendTbtWmiMessageToFW(cID, (PUCHAR)&CheckFinishRequest, NVMBlockDataLength, PDF_READ_CONFIGURATION_REGISTERS);
         if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse(
                &NotCompleted)
             || retries++ == 1000)
         {
            TbtServiceLogger::LogError("Error: WriteToFlash failed. READ_CONFIGURATION_REGISTERS_REQUEST:MAIL_IN");
            return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
         }
         else if (NotCompleted == 1)
         {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
         }
      }
      if (static_cast<TBTDRV_STATUS>(NotCompleted) != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
         return static_cast<TBTDRV_STATUS>(NotCompleted);

      // check in mail_out status StatusResponse
      CheckFinishRequest.DWIndex     = MAIL_OUT;
      MAILBOX_RESPONSE_CODE response = MAILBOX_RESPONSE_CODE::COMMAND_COMPLETED_SUCCESS_RESPONSE_CODE;

      SendTbtWmiMessageToFW(cID, (PUCHAR)&CheckFinishRequest, NVMBlockDataLength, PDF_READ_CONFIGURATION_REGISTERS);
      if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse(
             (uint32_t*)&response))
      {
         TbtServiceLogger::LogError(
            "Error: WriteToFlash failed read status. READ_CONFIGURATION_REGISTERS_REQUEST:MAIL_OUT");
         return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
      }
      return static_cast<TBTDRV_STATUS>(response);
   }
   catch (...)
   {
      return TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE;
   }
}

TBTDRV_STATUS LinuxControllerCommandSender::FlashAuthentication(const controlleriD& cID,
                                                                const ROUTE_STRING& routeString,
                                                                int dmaPort) const
{
   try
   {
      WRITE_CONFIGURATION_REGISTERS_REQUEST AuthenticateUpdate = {};

      // Send NVM flash data
      AuthenticateUpdate.RouteString = routeString;
      AuthenticateUpdate.DWIndex     = MAIL_IN;
      AuthenticateUpdate.Length =
         (sizeof(AuthenticateUpdate.WriteData.FlashUpdateAuthenticate) - sizeof(uint32_t)) / sizeof(uint32_t);
      AuthenticateUpdate.Port               = dmaPort;
      AuthenticateUpdate.ConfigurationSpace = PORT_CONFIG_SPACE;
      AuthenticateUpdate.SequenceNumber     = 1;

      AuthenticateUpdate.WriteData.FlashUpdateAuthenticate.OperationRequest = 1;
      AuthenticateUpdate.WriteData.FlashUpdateAuthenticate.Command          = FLASH_UPDATE_AUTHENTICATE_REQUEST_CODE;
      AuthenticateUpdate.WriteData.FlashUpdateAuthenticate.Crc              = 0;

      uint32_t AuthenticateUpdateLength = FIELD_OFFSET(WRITE_CONFIGURATION_REGISTERS_REQUEST, WriteData)
                                          + sizeof(AuthenticateUpdate.WriteData.FlashUpdateAuthenticate);

      SendTbtWmiMessageToFW(
         cID, (PUCHAR)&AuthenticateUpdate, AuthenticateUpdateLength, PDF_WRITE_CONFIGURATION_REGISTERS);

      if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse())
      {
         return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
      }

      // FIXME: find a solution for this sleep
      const int authenticationSleep = 20;

      for (int i = 0; i < authenticationSleep; ++i)
      {
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      // check mail_in done
      READ_CONFIGURATION_REGISTERS_REQUEST CheckFinishRequest = {};
      CheckFinishRequest.RouteString                          = routeString;
      CheckFinishRequest.DWIndex                              = MAIL_IN;
      CheckFinishRequest.Length                               = 1; //????
      CheckFinishRequest.Port                                 = dmaPort;
      CheckFinishRequest.ConfigurationSpace                   = PORT_CONFIG_SPACE;
      CheckFinishRequest.SequenceNumber                       = 1;

      AuthenticateUpdateLength = sizeof(CheckFinishRequest);
      uint32_t NotCompleted    = 1;
      int retries              = 0;
      // wait for mail_out to be ready
      while (NotCompleted == 1) // define retries timeout
      {
         SendTbtWmiMessageToFW(
            cID, (PUCHAR)&CheckFinishRequest, AuthenticateUpdateLength, PDF_READ_CONFIGURATION_REGISTERS);
         if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse(
                &NotCompleted)
             || retries++ == 5)
         {
            return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
         }
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      if (static_cast<TBTDRV_STATUS>(NotCompleted) != TBTDRV_STATUS::SUCCESS_RESPONSE_CODE)
         return static_cast<TBTDRV_STATUS>(NotCompleted);

      // check in mail_out status StatusResponse
      CheckFinishRequest.DWIndex     = MAIL_OUT;
      MAILBOX_RESPONSE_CODE response = MAILBOX_RESPONSE_CODE::AUTHENTICATION_IN_PROGRESS_RESPONSE_CODE;
      retries                        = 0;
      while (response == MAILBOX_RESPONSE_CODE::AUTHENTICATION_IN_PROGRESS_RESPONSE_CODE) // define retries timeout
      {
         SendTbtWmiMessageToFW(
            cID, (PUCHAR)&CheckFinishRequest, AuthenticateUpdateLength, PDF_READ_CONFIGURATION_REGISTERS);
         if (!ConnectionManager::GetInstance()->GetController(cID)->GetFWUpdateResponseLock().WaitForResponse(
                (uint32_t*)&response)
             || retries++ == 100)
         {
            return TBTDRV_STATUS::FW_RESPONSE_TIMEOUT_CODE;
         }
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      return static_cast<TBTDRV_STATUS>(response);
   }
   catch (...)
   {
      return TBTDRV_STATUS::SERVICE_INTERNAL_ERROR_CODE;
   }
}
