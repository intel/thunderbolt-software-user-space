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

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <iosfwd>
#include "MessagesWrapper.h"

class IController;

/**
 * \brief This header includes various utils to make FW message handling easier
 * and more natural to C++
 */

namespace MessageSender
{
enum class Result
{
   Success,
   /// \brief FW failure
   Failure,
   Timeout,
};

std::ostream& operator<<(std::ostream& ostr, Result result);
std::string to_string(Result result);
} // namespace MessageSender

/**
 * \brief This class wraps around I2C_REGISTERS_ACCESS_COMMAND. Use the nested
 * CommandDetails class for creating the actual command, preferably by having an
 * appropriate factory function to create the needed command
 */
class I2CRegisterAccessCommand
{
public:
   enum class AccessMode
   {
      read,
      write
   };

   /**
    * \brief Includes only the command-specific details, separating them from
    * command information that is related to the routing (link, depth, etc.)
    */
   class CommandDetails
   {
      friend class I2CRegisterAccessCommand;

   public:
      CommandDetails(AccessMode mode, int length, int offset, const std::vector<uint32_t>& data);

   private:
      const int m_write  = 0;
      const int m_length = 0;
      const int m_offset = 0;
      const std::vector<uint32_t> m_data;
   };

   /**
    * \brief Construct command targeted for a controller
    *
    * \param[in]  controller  Controller to send it to
    * \param[in]  details     Details of the command to send
    *
    * controller isn't const because send() needs it non const
    */
   I2CRegisterAccessCommand(IController& controller, const CommandDetails& details);

   /**
    * \brief Send the command to FW
    *
    * Not const due to SendTbtWmiMessageToFW() implementation :(
    *
    * \return enum Result (success/failure/timeout)
    */
   MessageSender::Result send();

   /**
    * \brief Check if I2C register access command is supported with a controller
    *
    * \param[in]  controller  Controller to check for support
    *
    * \return true if the controller supports I2C command, otherwise - false
    */
   static bool supported(const IController& controller);

private:
   IController& m_controller;
   I2C_REGISTERS_ACCESS_COMMAND m_command = {};

   static const int supportedNvmVersion = 0x10;
};

std::ostream& operator<<(std::ostream& ostr, const I2C_REGISTERS_ACCESS_RESPONSE& msg);
std::string to_string(const I2C_REGISTERS_ACCESS_RESPONSE& msg);
