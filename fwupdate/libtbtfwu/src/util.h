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

#include <vector>
#include <string>
#include <stdint.h>

namespace tbt
{

/// <summary>
/// Check if a pointer comes from FW is valid
/// </summary>
/// <param name="pointer">Pointer value as comes from FW</param>
/// <param name="pointerSize">
/// The real size of the pointer (to be able to handle 24-bit pointer)
/// </param>
/// <returns>True if the pointer is valid, otherwise - false</returns>
/// <remarks>Invalid pointer is NULL pointer or -1 (0xFFFFFFFF in uint)</remarks>
bool ValidNVMPointer(uint32_t pointer, uint32_t pointerSize);

uint32_t ToUInt32(const std::vector<uint8_t>& vec, size_t offset);
uint16_t ToUInt16(const std::vector<uint8_t>& vec, size_t offset);

template <typename T>
void CLEAR_PTR(T*& X
#ifdef NDEBUG
               __attribute__((unused))
#endif
               )
{
#ifndef NDEBUG
   X = (T*)0xbadbadff;
#endif
}
}
