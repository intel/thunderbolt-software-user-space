/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
 *
 * Copyright(c) 2009 - 2016 Intel Corporation.
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

#include "MessageUtils.h"
#include <sstream>
#include "ControllerDetails.h"
#include "ConnectionManager.h"
#include "DriverCommandResponseLock.h"
#include "tbtException.h"
#include "Utils.h"

namespace MessageSender
{
std::ostream& operator<<(std::ostream& ostr, Result result)
{
   ostr << to_string(result);
   return ostr;
}

std::string to_string(Result result)
{
   switch (result)
   {
   case Result::Success:
      return "Success";
   case Result::Failure:
      return "Failure";
   case Result::Timeout:
      return "Timeout";
   default:
   {
      std::ostringstream res;
      res << "Unknown enum value (" << /*toUnderlineType(*/ result /*)*/ << ")";
      TbtServiceLogger::LogError("Error: %s", res.str().c_str());
      return res.str();
   }
   }
}

/**
 * \brief Send the command to FW
 *
 * Not const due to SendTbtWmiMessageToFW() implementation :(
 *
 * \param[in]  controller  The controller to send the message to
 * \param[in]  message     Message struct to send
 * \param[in]  size        Size of the message to send (size of 'message' buffer)
 * \param[in]  wait        True to wait for response, false to return right after
 *                         the sending
 *
 * \return enum Result (success/failure/timeout); if 'wait' is false, it's just
 * says that the sending itself succeeded (otherwise it throws)
 */
Result send(IController& controller, uint8_t* message, size_t size, bool wait)
{
   try
   {
      ConnectionManager::GetInstance()->GetControllerCommandSender().SendTbtWmiMessageToFW(
         controller.GetControllerID(), message, size, PDF_SW_TO_FW_COMMAND);
   }
   catch (const std::exception& e)
   {
      TbtServiceLogger::LogError("Error: Exception : %s", e.what());
      throw;
   }
   catch (...)
   {
      TbtServiceLogger::LogError("Error: Unknown error");
      throw;
   }

   if (!wait)
   {
      return Result::Success;
   }

   uint32_t response  = 0;
   auto gotFWResponse = controller.GetFWUpdateResponseLock().WaitForResponse(&response);

   if (!gotFWResponse)
   {
      return Result::Timeout;
   }
   return response ? Result::Failure : Result::Success;
}
} // namespace MessageSender

std::ostream& operator<<(std::ostream& ostr, const I2C_REGISTERS_ACCESS_RESPONSE& msg)
{
   StreamFlagProtector protector(ostr);
   // clang-format off
   ostr << "I2C registers access response: "
           "error flag: "                          << msg.ErrorFlag  << ", "
           "write: "                               << msg.Write      << ", "
           "depth: "                               << msg.LocalDepth << ", "
           "port: "                                << msg.TbtPort    << ", "
           "offset: " << std::hex << std::showbase << msg.Offset;
   // clang-format on
   return ostr;
}

std::string to_string(const I2C_REGISTERS_ACCESS_RESPONSE& msg)
{
   std::ostringstream oss;
   oss << msg;
   return oss.str();
}

namespace // anon. namespace
{
struct PrintEnabler
{
};

struct CIOLinkSpeed : private PrintEnabler
{
   CIOLinkSpeed(uint8_t speed) : value(speed) {}
   uint8_t value;

   const char* to_c_str() const
   {
      switch (value)
      {
      case 0x0:
         return "10 Gbps";
      case 0x1:
         return "20 Gbps";
      case 0x2:
         return "40 Gbps";
      default:
         return "Reserved";
      }
   }
};

struct CIOLinkWidth : private PrintEnabler
{
   CIOLinkWidth(uint8_t width) : value(width) {}
   uint8_t value;

   const char* to_c_str() const
   {
      switch (value)
      {
      case 0x0:
         return "Single lane";
      case 0x1:
         return "Dual lane";
      default:
         return "Reserved";
      }
   }
};

struct DPRate : private PrintEnabler
{
   DPRate(uint8_t rate) : value(rate) {}
   uint8_t value;

   const char* to_c_str() const
   {
      switch (value)
      {
      case 0x0:
         return "DP not active";
      case 0x1:
         return "1.62 Gbps";
      case 0x2:
         return "2.7 Gbps";
      case 0x3:
         return "5.4 Gbps";
      case 0x4:
         return "8.1 Gbps";
      case 0x5:
         return "10 Gbps";
      default:
         return "Reserved";
      }
   }
};

struct DPLanes : private PrintEnabler
{
   DPLanes(uint8_t lanes) : value(lanes) {}
   uint8_t value;

   const char* to_c_str() const
   {
      switch (value)
      {
      case 0x1:
         return "1 lane";
      case 0x2:
         return "2 lanes";
      case 0x3:
         return "4 lanes";
      default: // incl. 0
         return "Reserved";
      }
   }
};

template <typename T, typename std::enable_if<std::is_base_of<PrintEnabler, T>::value>::type* = nullptr>
std::ostream& operator<<(std::ostream& ostr, T data)
{
   static_assert(sizeof(data.value) <= sizeof(uint32_t), "We can't simply casting here");
   StreamFlagProtector protector(ostr);
   ostr << std::hex << std::showbase << static_cast<uint32_t>(data.value) << " (" << data.to_c_str() << ")";
   return ostr;
}
} // anon. namespace

I2CRegisterAccessCommand::CommandDetails::CommandDetails(AccessMode mode,
                                                         int length,
                                                         int offset,
                                                         const std::vector<uint32_t>& data)
   : m_write(mode == AccessMode::write ? 1 : 0), m_length(length), m_offset(offset), m_data(data)
{
}

I2CRegisterAccessCommand::I2CRegisterAccessCommand(IController& controller, const CommandDetails& details)
   : m_controller(controller)
{
   if (!supported(controller))
   {
      throw TbtException("ERROR: Controller is not supported for I2C");
   }

   m_command.RequestCode = I2C_REGISTER_ACCESS_COMMAND_CODE;
   m_command.Write       = details.m_write;
   m_command.Length      = details.m_length;
   m_command.Offset      = details.m_offset;
   m_command.TbtPort     = 1; // We always query portA, assuming both PD have the same FW
   std::copy(details.m_data.cbegin(), details.m_data.cend(), m_command.Data);
}

MessageSender::Result I2CRegisterAccessCommand::send()
{
   TbtServiceLogger::LogInfo("Sending I2C access registers command");
   return MessageSender::send(m_controller, reinterpret_cast<uint8_t*>(&m_command), sizeof(m_command), true);
}

bool I2CRegisterAccessCommand::supported(const IController& controller)
{
   const auto& controllerDetails = controller.GetControllerData();
   TbtServiceLogger::LogDebug("Is I2C Reg access cmd supported?  Controller gen = %d, min is %d",
                              controllerDetails->GetGeneration(),
                              ThunderboltGeneration::THUNDERBOLT_3);
   if (controllerDetails->GetGeneration() < ThunderboltGeneration::THUNDERBOLT_3)
   {
      return false;
   }
   uint32_t devid = GetDeviceIDFromControllerID(controller.GetControllerID());
   if (devid == 0x1575 || devid == 0x1577 || devid == 0x15DD)
   {
      auto major = controllerDetails->GetNVMVersion().major();
      TbtServiceLogger::LogDebug("  we are alpine ridge.  major version = %d", major ? *major : -1);
      if (!major || *major < supportedNvmVersion)
      {
         return false;
      }
   }
   return true;
}
