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


#ifndef MESSAGE_FROM_FW_H 
#define MESSAGE_FROM_FW_H

#include "defines.h"

/**
 * event handler class for handling messages received from FW
 */
class MessageFromFwEvent 
{

public:
	static void HandleFwToSwResponse(controlleriD cId, const std::vector<uint8_t>& Msg);
	static void HandleFwToSwNotification(controlleriD cId, const std::vector<uint8_t>& Msg);
};

#endif // MESSAGE_FROM_FW_H
