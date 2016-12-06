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
 ********************************************************************************/")
