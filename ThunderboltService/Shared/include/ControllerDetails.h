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

#ifndef CONTROLLER_DETAILS_H
#define CONTROLLER_DETAILS_H
#include "Utils.h"

/*
 * this class stores the controller details e.g
 * Driver & FW Version, TBT generation, # of ports etc...
 */
class ControllerDetails
{
public:
   ControllerDetails(std::wstring FWVersion,
                     std::wstring DriverVersion,
                     uint32_t SecurityLevel,
                     ThunderboltGeneration Generation,
                     uint32_t NumOfPorts);

   // only to be used from safemode flow
   ControllerDetails()
      : m_NumOfPorts(0), m_Generation(ThunderboltGeneration::THUNDERBOLT_3), m_SecurityLevel(0), m_IsInSafeMode(true)
   {
   }

   ControllerDetails(const DRIVER_READY_RESPONSE& DriverReadyResponseNotification,
                     ThunderboltGeneration Generation,
                     uint32_t NumOfPorts);
   ~ControllerDetails(void);
   void SetFWVersion(std::wstring version) { m_FWVersion = version; };
   void SetDriverVersion(std::wstring version) { m_DriverVersion = version; };
   void SetSecurityLevel(uint32_t val) { m_SecurityLevel = val; };
   void SetGeneration(ThunderboltGeneration val) { m_Generation = val; };
   void SetNumOfPorts(uint32_t val) { m_NumOfPorts = val; };
   void SetNVMVersion(const Version& val) { m_NVMVersion = val; };
   void SetIsInSafeMode(bool val) { m_IsInSafeMode = val; };

   std::wstring GetFWVersion() const { return m_FWVersion; };
   std::wstring GetDriverVersion() const { return m_DriverVersion; };
   uint32_t GetSecurityLevel() const { return m_SecurityLevel; };
   ThunderboltGeneration GetGeneration() const { return m_Generation; };
   uint32_t GetNumOfPorts() const { return m_NumOfPorts; };
   Version GetNVMVersion() const { return m_NVMVersion; };
   bool GetIsInSafeMode() const noexcept { return m_IsInSafeMode; };

private:
   uint32_t m_NumOfPorts;              // number of ports for this controller
   ThunderboltGeneration m_Generation; // Thunderbolt generetion (e.g. Thunderbolt 2)
   uint32_t m_SecurityLevel;           // the controller security level
   Version m_NVMVersion;

   std::wstring m_DriverVersion; // the version of the kernel module
   std::wstring m_FWVersion;     // the FW version for this controller

   bool m_IsInSafeMode;
};

#endif // CONTROLLER_DETAILS_H
