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

#include "controller.h"

#include <iostream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "file.h"

namespace po = boost::program_options;

namespace
{
const fs::path acltree            = "/var/lib/thunderbolt/acl";
const std::string hostRouteString = "0-0";
const std::string opt_approve_all = "approve-all";
const std::string opt_remove      = "remove";
const std::string opt_remove_all  = "remove-all";
} // namespace

tbtadm::Controller::Controller(int argc,
                               char* argv[],
                               std::ostream& out,
                               std::ostream& err)
    : m_argc(argc), m_argv(argv), m_out(out), m_err(err)
{
}

void tbtadm::Controller::run()
{
    if (m_argc >= 2)
    {
        if (m_argv[1] == opt_approve_all)
        {
            return approveAll();
        }
        if (m_argv[1] == opt_remove)
        {
            if (m_argc == 3)
            {
                return remove(m_argv[2]);
            }
        }
        if (m_argv[1] == opt_remove_all)
        {
            return removeAll();
        }
    }

    // TODO: help
    m_out << "Usage: " << opt_approve_all << " | " << opt_remove << " <uuid> | "
          << opt_remove_all << "\n";
    throw std::runtime_error("Wrong usage");
}

void tbtadm::Controller::approveAll()
{
    for (auto dir : boost::make_iterator_range(
             fs::directory_iterator("/sys/bus/thunderbolt/devices"),
             fs::directory_iterator{}))
    {
        if (dir.status().type() != fs::directory_file)
        {
            continue;
        }
        static const std::string domain = "domain";
        if (dir.path().filename().string().substr(0, domain.size()) != domain)
        {
            continue;
        }
        m_out << "Found domain " << dir << '\n';
        approveAll(dir / hostRouteString);
    }
}

void tbtadm::Controller::approveAll(const fs::path& dir)
{
    for (auto child : boost::make_iterator_range(fs::directory_iterator(dir),
                                                 fs::directory_iterator{}))
    {
        if (child.status().type() != fs::directory_file)
        {
            continue;
        }
        if (fs::exists(child / "authorized"))
        {
            m_out << "Found child " << child << '\n';
            approve(child);
            approveAll(child);
        }
    }
}

void tbtadm::Controller::approve(const fs::path& dir) try
{
    chdir(dir.c_str());
    m_out << "Authorizing " << dir << '\n';

    File authorized("authorized", File::Mode::Read);
    // TODO: check SL
    if (std::stoi(authorized.read()))
    {
        m_out << "Already authorized\n";
        return;
    }

    // TODO: Handle SL2
    addToACL();

    authorized = File("authorized", File::Mode::Write);
    authorized << 1;

    m_out << "Authorized\n";
}
catch (std::system_error& e)
{
    m_err << e.code() << ' ' << e.what() << '\n';
}
catch (std::exception& e)
{
    m_err << "Exception: " << e.what() << '\n';
}
catch (...)
{
    m_err << "Unknown exception\n";
}

void tbtadm::Controller::addToACL()
{
    File uuidFile("unique_id", File::Mode::Read);

    auto uuid = uuidFile.read();
    boost::algorithm::trim(uuid);
    auto acl = acltree / uuid;
    if (fs::exists(acl))
    {
        m_out << "Already in ACL\n";
        return;
    }

    fs::create_directories(acl);
    fs::copy("vendor_name", acl / "vendor_name");
    fs::copy("device_name", acl / "device_name");

    m_out << "Added to ACL\n";
    // TODO: add key for SL2
}

void tbtadm::Controller::remove(const std::string& uuid)
{
    auto acl = acltree / uuid;
    if (!fs::exists(acl))
    {
        throw std::runtime_error("ACL entry doesn't exist");
    }
    fs::remove_all(acl);
}

void tbtadm::Controller::removeAll()
{
    fs::remove_all(acltree);
}
