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
 
#ifndef CONTROLLER_SETTINGS_H
#define CONTROLLER_SETTINGS_H
#include "Utils.h"

/**
 * the controller policies set by the FW
 */
class ControllerSettings
{
public:
	//ControllerSettings(void);
	ControllerSettings(const DRIVER_READY_RESPONSE& controllerData);
	ControllerSettings(bool CertifiedOnly, bool OverrideFirstDepth, bool AllowDockAtAnyDepth);
	
	bool operator==(const ControllerSettings &other) const
	{ 
		return m_CertifiedOnly == other.m_CertifiedOnly &&
			m_OverrideFirstDepth == other.m_OverrideFirstDepth &&
			m_AllowDockAtAnyDepth == other.m_AllowDockAtAnyDepth;
	}

	~ControllerSettings(void);
	bool GetCertifiedOnly() const {return m_CertifiedOnly;};
	bool GetOverrideFirstDepth() const {return m_OverrideFirstDepth;};
	bool GetAllowDockAtAnyDepth() const {return m_AllowDockAtAnyDepth;};
	void SetCertifiedOnly(bool val){m_CertifiedOnly=val;};
	void SetOverrideFirstDepth(bool val){m_OverrideFirstDepth=val;};
	uint32_t ToFlags(){ return ControllerSettingsToEnum(m_CertifiedOnly,m_OverrideFirstDepth);};
private:
   bool  m_CertifiedOnly,
         m_OverrideFirstDepth,
         m_AllowDockAtAnyDepth;
};

#endif // !CONTROLLER_SETTINGS_H
