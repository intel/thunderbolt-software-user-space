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

#pragma once

#include <string>

#include <fcntl.h> // for O_RDONLY, O_WRONLY

namespace tbtadm
{
/**
 * @brief This class wraps-around POSIX file interface for C++ style usage
 *
 * The class encapsulates the actions to be type-safe, translates errors to
 * exceptions so they will not get ignored and implements RAII-style handling of
 * the file.
 *
 * The operations work on the file as whole, e.g. reading the whole file
 * together, as this is the only use-case we need right now.
 *
 * The assumption here is that the class is used only for sysfs files (e.g. non-
 * seekable).
 *
 * The reason all this is done instead of simply using the already existing C++
 * streams is because we need the real errno value in case of an error, while
 * streams return a generic error even while setting the stream to throw on
 * error and can easily overwrite errno by other actions done after the failing
 * operations.
 * C functions could be used instead of POSIX but write error doesn't appear
 * until flushing the file, so POSIX is a better choice for this use-case.
 */
class File
{
public:
    enum class Mode
    {
        Read  = O_RDONLY,
        Write = O_WRONLY
    };

    /**
     * @brief Open the file
     *
     * @param filename  Name/path of the file to open
     * @param mode      File open mode
     */
    File(const std::string& filename, Mode mode);

    /**
     * @brief close the file
     */
    ~File();

    File(const File&) = delete;
    File& operator=(const File&) = delete;

    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;

    /**
     * @brief write the string content
     *
     * @param value     content to write
     */
    void write(const std::string& value);

    /**
     * @brief read the whole file
     *
     * @return A string with the file content
     */
    std::string read();

private:
    void close();
    static const int ERROR = -1;

    int m_fd = ERROR;
};

template <typename T>
File& operator<<(File& file, const T& t)
{
    file.write(std::to_string(t));
    return file;
}
} // namespace tbtadm
