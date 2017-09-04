/*******************************************************************************
 * Thunderbolt(TM) tbtadm tool
 * This code is distributed under the following BSD-style license:
 *
 * Copyright(c) 2017 Intel Corporation.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include "file.h"

#include <cerrno>
#include <system_error>
#include <unistd.h>

namespace fs = boost::filesystem;

namespace
{
[[noreturn]] void throwErrno()
{
    throw std::system_error(errno, std::system_category());
}
} // namespace

tbtadm::File::File(const fs::path& path, Mode mode, int flags, int perm)
    : File(path.string(), mode, flags, perm)
{
}

tbtadm::File::File(const std::string& filename, Mode mode, int flags, int perm)
    : File(filename.c_str(), mode, flags, perm)
{
}

tbtadm::File::File(const char* filename, Mode mode, int flags, int perm)
    : m_fd(perm ? ::open(filename, static_cast<int>(mode) | flags, perm)
                : ::open(filename, static_cast<int>(mode) | flags))
{
    if (m_fd == ERROR)
    {
        throwErrno();
    }
}

tbtadm::File::~File()
{
    close();
}

tbtadm::File::File(tbtadm::File&& other) noexcept
{
    std::swap(m_fd, other.m_fd);
}

tbtadm::File& tbtadm::File::operator=(tbtadm::File&& other) noexcept
{
    close();
    std::swap(m_fd, other.m_fd);
    return *this;
}

void tbtadm::File::write(const std::string& value)
{
    errno    = 0;
    auto ret = ::write(m_fd, value.data(), value.size());
    if (ret == ERROR || (!ret && errno))
    {
        throwErrno();
    }
}

std::string tbtadm::File::read()
{
    std::string content;
    errno = 0;
    while (true)
    {
        char c;
        auto ret = ::read(m_fd, &c, sizeof(c));
        if (ret == ERROR || (!ret && errno))
        {
            throwErrno();
        }
        if (!ret)
        {
            break;
        }
        content.push_back(c);
    }
    if (content.empty())
    {
        throw std::runtime_error("No data could be read");
    }
    return content;
}

void tbtadm::File::close()
{
    if (m_fd != ERROR)
    {
        ::close(m_fd);
        m_fd = ERROR;
    }
}

void tbtadm::chdir(const fs::path&dir)
{
    if (::chdir(dir.c_str()) == File::ERROR)
    {
        throwErrno();
    }
}
