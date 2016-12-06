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
#include "ControllerDetails.h"
#include <string>

ControllerDetails::ControllerDetails(std::wstring FWVersion,
                                     std::wstring DriverVersion,
                                     uint32_t SecurityLevel,
                                     ThunderboltGeneration Generation,
                                     uint32_t NumOfPorts)
   : m_NumOfPorts(NumOfPorts),
     m_Generation(Generation),
     m_SecurityLevel(SecurityLevel),
     m_DriverVersion(DriverVersion),
     m_FWVersion(FWVersion),
     m_IsInSafeMode(false)
{
}

ControllerDetails::ControllerDetails(const DRIVER_READY_RESPONSE& e,
                                     ThunderboltGeneration Generation,
                                     uint32_t NumOfPorts)
   : m_NumOfPorts(NumOfPorts),
     m_Generation(Generation),
     m_SecurityLevel(e.SecurityLevel),
     m_DriverVersion(L""),
     m_FWVersion(std::to_wstring(e.FwRamVersion) + L"." + std::to_wstring(e.FwRomVersion)),
     m_IsInSafeMode(false)
{
}

ControllerDetails::~ControllerDetails(void)
{
}
