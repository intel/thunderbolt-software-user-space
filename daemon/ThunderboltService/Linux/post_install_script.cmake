#update udev rules
execute_process(COMMAND udevadm control --reload-rules
                RESULT_VARIABLE Result
                OUTPUT_VARIABLE Output
                ERROR_VARIABLE  Error)

if(Result EQUAL 0)
    message(STATUS "udev rule updated")
else()
    message(FATAL_ERROR "udev rule update failed\nResult - ${Result}\nOutput - ${Output}\nError - ${Error}")
endif()

#enable thunderbolt service
execute_process(COMMAND systemctl enable thunderbolt
                RESULT_VARIABLE Result
                OUTPUT_VARIABLE Output
                ERROR_VARIABLE  Error)

if(Result EQUAL 0)
    message(STATUS "enabling thunderbolt service done")
else()
    message(FATAL_ERROR "enabling thunderbolt service failed\nResult - ${Result}\nOutput - ${Output}\nError - ${Error}")
endif()

#reload systemd
execute_process(COMMAND systemctl daemon-reload
                RESULT_VARIABLE Result
                OUTPUT_VARIABLE Output
                ERROR_VARIABLE  Error)

if(Result EQUAL 0)
    message(STATUS "reload systemd done")
else()
    message(FATAL_ERROR "reload systemd failed\nResult - ${Result}\nOutput - ${Output}\nError - ${Error}")
endif()

#start thunderbolt daemon
execute_process(COMMAND systemctl restart thunderbolt
                RESULT_VARIABLE Result
                OUTPUT_VARIABLE Output
                ERROR_VARIABLE  Error)

if(Result EQUAL 0)
    message(STATUS "start thunderbolt daemon")
else()
    message(WARNING "start thunderbolt daemon failed\nResult - ${Result}\nOutput - ${Output}\nError - ${Error}")
endif()

message("
/*******************************************************************************
 *
 * Intel Thunderbolt daemon
 * Copyright(c) 2014 - 2016 Intel Corporation.
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
 * the file called \"COPYING\".
 *
 * Contact Information:
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/")
