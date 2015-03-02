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

#ifndef _MESSAGES_WRAPPER_H_
#define _MESSAGES_WRAPPER_H_
#define INTEL_P2P_PROTOCOL_SUPPORTED

#include "typedef.h"

#include <assert.h>
#include <stddef.h>

#define C_ASSERT(a) static_assert(a,"wrong size") //for replacing the C_ASSERT in driver
#define FIELD_OFFSET(type,filed) offsetof(type,filed)

#define SFINIT(f, a) f = {a}

#include "messages.h"
#endif
