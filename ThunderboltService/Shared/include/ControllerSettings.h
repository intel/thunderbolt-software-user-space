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

#ifndef CONTROLLER_SETTINGS_H
#define CONTROLLER_SETTINGS_H
#include "Utils.h"

/**
 * the controller policies set by the FW
 */
class ControllerSettings
{
public:
   // ControllerSettings(void);
   ControllerSettings(const DRIVER_READY_RESPONSE& controllerData);
   ControllerSettings(bool CertifiedOnly, bool OverrideFirstDepth, bool AllowDockAtAnyDepth);

   bool operator==(const ControllerSettings& other) const
   {
      return m_CertifiedOnly == other.m_CertifiedOnly && m_OverrideFirstDepth == other.m_OverrideFirstDepth
             && m_AllowDockAtAnyDepth == other.m_AllowDockAtAnyDepth;
   }

   ~ControllerSettings(void);
   bool GetCertifiedOnly() const { return m_CertifiedOnly; };
   bool GetOverrideFirstDepth() const { return m_OverrideFirstDepth; };
   bool GetAllowDockAtAnyDepth() const { return m_AllowDockAtAnyDepth; };
   void SetCertifiedOnly(bool val) { m_CertifiedOnly = val; };
   void SetOverrideFirstDepth(bool val) { m_OverrideFirstDepth = val; };
   uint32_t ToFlags() { return ControllerSettingsToEnum(m_CertifiedOnly, m_OverrideFirstDepth); };
private:
   bool m_CertifiedOnly, m_OverrideFirstDepth, m_AllowDockAtAnyDepth;
};

#endif // !CONTROLLER_SETTINGS_H
