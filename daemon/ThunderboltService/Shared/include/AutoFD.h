/********************************************************************************
 * Thunderbolt(TM) daemon
 * This daemon is distributed under the following BSD-style license:
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

namespace tbt
{

/**
 * Simple RAII wrapper around a file descriptor.
 */
class AutoFD
{
public:
   explicit AutoFD(int fd_) noexcept;
   AutoFD() noexcept;
   ~AutoFD();

   /**
    * The "swapperator".  Exchange this object's underlying FD with that of
    * 'other'.
    */
   void swap(AutoFD& other) noexcept;

   /**
    * Move from another auto FD.  Any currently owned FD will be closed, and
    * 'other's owned FD will be taken from 'other'.
    */
   AutoFD(AutoFD&& other) noexcept;

   /**
    * @return true if and only if this AutoFD is holding a file descriptor.
    */
   bool empty() const noexcept;

   /**
    * @return the underlying file descriptor if one is being held, or -1
    * otherwise.
    */
   int get() const noexcept;

   /**
    * Take ownership of 'fd'.  Any currently owned FD will be closed.
    */
   void reset(int fd) noexcept;

   /**
    * Assignment from AutoFD.  Any currently owned FD will be closed, and
    * 'other's owned FD will be taken from 'other'.
    */
   AutoFD& operator=(AutoFD&& other) noexcept;

   bool operator==(int fd) const noexcept;

private:
   int m_fd;
};

} // end of namespace tbt
