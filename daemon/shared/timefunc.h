/********************************************************************************
 * Thunderbolt(TM) FW update library
 * This library is distributed under the following BSD-style license:
 *
 * Copyright(c) 2016 Intel Corporation.
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

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

namespace tbt
{

namespace time
{

/**
 * Return the current time in microseconds since the epoch.
 */
inline uint64_t NowUS()
{
   struct timeval tv;
   (void)gettimeofday(&tv, nullptr);
   return (uint64_t(tv.tv_sec) * uint64_t(1000000) + uint64_t(tv.tv_usec));
}

/**
 * Return a relative, monotonically increasing ticks counter in microsecond
 * resolution.  If you want to measure time deltas, it is better to use
 * this interface than NowUS(), because NowUS() could move backward e.g. if
 * NTP adjusts the clock.
 *
 * TicksUS() is guaranteed to be monotonically increasing, but it is not
 * guaranteed to represent any particular absolute wall clock time.  Thus it
 * is only useful for calculating deltas.
 */
inline uint64_t TicksUS()
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   uint64_t usec(ts.tv_sec * 1000000);
   usec += ts.tv_nsec / 1000;
   return usec;
}

/**
 * Return the current time in milliseconds since the epoch.
 */
inline uint64_t NowMS()
{
   return NowUS() / 1000;
}

} // namespace time

} // namespace tbt
