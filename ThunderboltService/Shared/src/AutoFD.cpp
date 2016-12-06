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

#include "logger.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "AutoFD.h"

namespace tbt
{

AutoFD::AutoFD(int fd_) noexcept : m_fd(fd_)
{
}
AutoFD::AutoFD() noexcept : m_fd(-1)
{
}
AutoFD::AutoFD(AutoFD&& other) noexcept : m_fd(-1)
{
   m_fd       = other.m_fd;
   other.m_fd = -1;
}
AutoFD::~AutoFD()
{
   if (m_fd < 0)
   {
      return;
   }
   int rc;
   do
   {
      rc = ::close(m_fd);
   } while (rc == -1 && errno == EINTR);
   if (rc == -1)
   {
      TbtServiceLogger::LogWarning("Could not close FD %d: %s\n", m_fd, strerror(errno));
   }
   m_fd = -1;
}

bool AutoFD::empty() const noexcept
{
   return m_fd == -1;
}

int AutoFD::get() const noexcept
{
   return m_fd;
}

bool AutoFD::operator==(int fd) const noexcept
{
   return m_fd == fd;
}

void AutoFD::reset(int fd) noexcept
{
   AutoFD(fd).swap(*this);
}

AutoFD& AutoFD::operator=(AutoFD&& other) noexcept
{
   AutoFD(std::move(other)).swap(*this);
   return *this;
}

void AutoFD::swap(AutoFD& other) noexcept
{
   std::swap(m_fd, other.m_fd);
}
};
