/*******************************************************************************
 *
 * Intel Thunderbolt daemon
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
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/


#ifndef _GENERAL_H_
#define _GENERAL_H_

#define ROUND_DOWN(n, g)   ( ( (g) & ((g) - 1) ) ? ( ((n) / (g)) * (g) ) : ( (n) & (-(g)) ) )
#define ROUND_UP(n, g)     ROUND_DOWN( (n) + (g) - 1, g )
#define ROUND(n, g)        ROUND_DOWN( (n) + ((g) / 2), g )
#define DIV_ROUND_UP(n, g) ( ((n) + (g) - 1) / (g) )
#define DIV_ROUND(n, g)    ( ((n) + ((g) / 2)) / (g) )

#define BITFIELD_RANGE(startbit, endbit)  ( (endbit) - (startbit) + 1 )
#define BITFIELD_BIT(bit)                 1
#define BITFIELD_MASK(startbit, endbit)   ( ( ( 1 << BITFIELD_RANGE( startbit, endbit ) ) - 1 ) << (startbit) )

#define TURN_ON_BIT(num) (1 << num)
#endif   //_GENERAL_H_
