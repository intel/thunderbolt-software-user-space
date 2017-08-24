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
#include <random>
#include <iterator>
#include <algorithm>

#include <boost/program_options.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "file.h"

using namespace std::string_literals;

namespace po = boost::program_options;

namespace
{
const fs::path acltree          = "/var/lib/thunderbolt/acl";
const fs::path sysfsDevicesPath = "/sys/bus/thunderbolt/devices";

const std::string uniqueIDFilename   = "unique_id";
const std::string authorizedFilename = "authorized";
const std::string vendorFilename     = "vendor_name";
const std::string deviceFilename     = "device_name";
const std::string keyFilename        = "key";
const std::string securityFilename   = "security";

const std::string domain          = "domain";
const std::string hostRouteString = "-0";
const std::string domainDevtype   = "DEVTYPE=thunderbolt_domain";
const std::string deviceDevtype   = "DEVTYPE=thunderbolt_device";

// TODO: replace with boost.program_options
const std::string opt_devices     = "devices";
const std::string opt_topology    = "topology";
const std::string opt_approve_all = "approve-all";
const std::string opt_acl         = "acl";
const std::string opt_remove      = "remove";
const std::string opt_remove_all  = "remove-all";

const std::string indent     = "|   ";
const std::string indentLast = "    ";

std::string read(const fs::path& path)
{
    tbtadm::File file(path, tbtadm::File::Mode::Read);
    auto content = file.read();
    return content;
}

std::string readAndTrim(const fs::path& path)
{
    return boost::algorithm::trim_copy(read(path));
}

bool findUeventAttr(const fs::path& path, const std::string& attribute)
{
    const auto ueventFile = path / "uevent";
    if (!fs::exists(ueventFile))
    {
        return false;
    }
    try
    {
        const auto uevent = read(ueventFile);
        return uevent.find(attribute) != uevent.npos;
    }
    // assuming this is from an empty uevent file
    catch (std::runtime_error&)
    {
        return false;
    }
}

bool isDomain(const fs::path& path)
{
    return findUeventAttr(path, domainDevtype);
}

bool isRouteString(const std::string& str)
{
    return str.size() > 1 && str[1] == '-' && str.find('.') == str.npos;
}

bool isHost(const std::string& str)
{
    return str.size() == 3 && str.substr(1) == hostRouteString;
}

bool isDevice(const fs::path& path)
{
    return findUeventAttr(path, deviceDevtype)
           && !isHost(path.filename().string());
}

struct SLDetails
{
    int num;
    std::string desc;
};

const std::map<std::string, SLDetails> slMap{{"none", {0, "SL0 (none)"}},
                                             {"user", {1, "SL1 (user)"}},
                                             {"secure", {2, "SL2 (secure)"}},
                                             {"dponly", {3, "SL3 (dponly)"}}};

int findSL()
{
    if (fs::exists(sysfsDevicesPath))
    {
        for (auto dir : boost::make_iterator_range(
                 fs::directory_iterator(sysfsDevicesPath), {}))
        {
            if (dir.status().type() != fs::directory_file)
            {
                continue;
            }
            if (isDomain(dir.path()))
            {
                return slMap.find(readAndTrim(dir.path() / securityFilename))
                    ->second.num;
            }
        }
    }

    return tbtadm::Controller::UnkownSL;
}
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
    // TODO: replace with boost.program_options
    if (m_argc >= 2)
    {
        if (m_argv[1] == opt_devices)
        {
            return devices();
        }
        if (m_argv[1] == opt_topology)
        {
            return topology();
        }
        if (m_argv[1] == opt_approve_all)
        {
            return approveAll();
        }
        if (m_argv[1] == opt_acl)
        {
            return acl();
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
    const std::string sep = " | ";
    m_out << "Usage: " << opt_devices << sep << opt_topology << sep
          << opt_approve_all << sep << opt_acl << sep << opt_remove
          << " <uuid>|<route-string>" << sep << opt_remove_all << "\n";
    throw std::runtime_error("Wrong usage");
}

void tbtadm::Controller::devices()
{
    m_sl = findSL();

    // Find and print devices
    for (auto dir : boost::make_iterator_range(
             fs::directory_iterator(sysfsDevicesPath), {}))
    {
        if (dir.status().type() != fs::directory_file)
        {
            continue;
        }
        if (!isDevice(dir.path()))
        {
            continue;
        }

        ::chdir(dir.path().c_str());
        auto authorized = [] {
            return stoi(readAndTrim(authorizedFilename)) ? "authorized"
                                                         : "non-authorized";
        };
        auto inACL = [sl = m_sl]
        {
            auto aclDir = acltree / readAndTrim(uniqueIDFilename);
            if (!fs::exists(aclDir))
            {
                return "not in ACL";
            }
            if (sl == 2 && !fs::exists(aclDir / keyFilename))
            {
                return "not in ACL (no key)";
            }
            return "in ACL";
        };

        // TODO: better formatting
        const auto routeString = dir.path().filename().string();
        m_out << routeString << '\t' << readAndTrim(vendorFilename) << '\t'
              << readAndTrim(deviceFilename) << '\t' << authorized() << '\t'
              << inACL() << '\n';
    }
}

struct tbtadm::Controller::ControllerInTree
{
    ControllerInTree(std::vector<std::string>&& desc) : m_desc(std::move(desc))
    {
    }
    std::vector<std::string> m_desc;
    std::map<std::string, ControllerInTree> m_children;
};

void tbtadm::Controller::topology()
{
    std::map<int, ControllerInTree> controllers;

    for (auto dir : boost::make_iterator_range(
             fs::directory_iterator(sysfsDevicesPath), {}))
    {
        if (dir.status().type() != fs::directory_file)
        {
            continue;
        }
        auto p           = dir.path();
        auto routeString = p.filename().string();
        if (isHost(routeString))
        {
            auto num      = routeString[0];
            auto security = p.parent_path() / (domain + num) / securityFilename;
            m_sl          = slMap.find(readAndTrim(security))->second.num;
            std::vector<std::string> desc;
            desc.emplace_back("Controller "s + num + '\n');
            desc.emplace_back("Name: " + readAndTrim(p / deviceFilename) + ", "
                              + read(p / vendorFilename));
            desc.emplace_back("Security level: "
                              + slMap.find(readAndTrim(security))->second.desc
                              + '\n');
            auto i = controllers.emplace(num, std::move(desc)).first;
            createTree(i->second, dir.path());
        }
    }

    std::string indentation;
    for (const auto& host : controllers)
    {
        auto last = host.first == controllers.rbegin()->first;
        m_out << host.second.m_desc[0];
        indentation += last ? indentLast : indent;
        printDetails(
            host.second.m_children.empty(), indentation, host.second.m_desc);
        printTree(indentation, host.second.m_children);
    }
}

void tbtadm::Controller::createTree(ControllerInTree& controller,
                                    const fs::path& path)
{
    auto authorized = [](const auto& path) -> std::string {
        return stoi(readAndTrim(path / authorizedFilename)) ? "Yes" : "No";
    };
    auto inACL = [sl = m_sl](const auto& path)->std::string
    {
        auto aclDir = acltree / readAndTrim(path / uniqueIDFilename);
        if (!fs::exists(aclDir))
        {
            return "No";
        }
        if (sl == 2 && !fs::exists(aclDir / keyFilename))
        {
            return "No (no key)";
        }
        return "Yes";
    };

    for (auto dir :
         boost::make_iterator_range(fs::directory_iterator(path), {}))
    {
        if (dir.status().type() != fs::directory_file)
        {
            continue;
        }
        auto p = dir.path();
        if (!isDevice(p))
        {
            continue;
        }

        auto routeString = p.filename().string();
        std::vector<std::string> desc;
        desc.emplace_back(readAndTrim(p / deviceFilename) + ", "
                          + read(p / vendorFilename));
        desc.emplace_back("Route-string: " + routeString + "\n");
        desc.emplace_back("Authorized: " + authorized(p) + "\n");
        desc.emplace_back("In ACL: " + inACL(p) + "\n");
        desc.emplace_back("UUID: " + read(p / uniqueIDFilename));
        auto i =
            controller.m_children.emplace(routeString, std::move(desc)).first;
        createTree(i->second, dir.path());
    }
}

void tbtadm::Controller::printTree(
    std::string& indentation,
    const std::map<std::string, ControllerInTree>& map)
{
    for (const auto& device : map)
    {
        auto last = device.first == map.rbegin()->first;
        m_out << indentation << "|\n";
        m_out << indentation << "+- " << device.second.m_desc[0];
        indentation += last ? indentLast : indent;
        printDetails(device.second.m_children.empty(),
                     indentation,
                     device.second.m_desc);
        printTree(indentation, device.second.m_children);
        indentation.resize(indentation.size() - indent.size());
    }
}

void tbtadm::Controller::printDetails(bool last,
                                      std::string& indentation,
                                      const std::vector<std::string>& details)
{
    m_out << indentation << "+- Details:\n";
    indentation += last ? indentLast : indent;
    for (size_t i = 1; i < details.size(); ++i)
    {
        m_out << indentation << "+- " << details[i];
    }
    indentation.resize(indentation.size() - indent.size());
}

void tbtadm::Controller::approveAll()
{
    for (auto dir : boost::make_iterator_range(
             fs::directory_iterator(sysfsDevicesPath), {}))
    {
        if (dir.status().type() != fs::directory_file)
        {
            continue;
        }
        if (!isDomain(dir.path()))
        {
            continue;
        }
        m_out << "Found domain " << dir << '\n';
        auto domainNum = dir.path().filename().string().substr(domain.size());
        m_sl =
            slMap.find(readAndTrim(dir.path() / securityFilename))->second.num;
        if (m_sl != 1 && m_sl != 2)
        {
            m_out << "Approval not relevant in SL" << m_sl << '\n';
            return;
        }
        approveAll(dir / (domainNum + hostRouteString));
    }
}

void tbtadm::Controller::approveAll(const fs::path& dir)
{
    for (auto child :
         boost::make_iterator_range(fs::directory_iterator(dir), {}))
    {
        if (child.status().type() != fs::directory_file)
        {
            continue;
        }
        if (fs::exists(child / authorizedFilename))
        {
            m_out << "Found child " << child << '\n';
            approve(child);
            approveAll(child);
        }
    }
}

// TODO: move to tbtadm-helper
void tbtadm::Controller::approve(const fs::path& dir) try
{
    ::chdir(dir.c_str());
    m_out << "Authorizing " << dir << '\n';

    File authorized(authorizedFilename, File::Mode::Read);
    if (std::stoi(authorized.read()))
    {
        m_out << "Already authorized\n";
        return;
    }

    addToACL();

    std::ostringstream keyStream;
    if (m_sl == 2)
    {
        std::default_random_engine eng(std::random_device{}());
        std::uniform_int_distribution<> dist(0, 0xF);
        keyStream << std::hex;
        std::generate_n(std::ostream_iterator<int>(keyStream), 64, [&] {
            return dist(eng);
        });

        File key(keyFilename, File::Mode::Write);
        key << keyStream.str();
    }

    authorized = File(authorizedFilename, File::Mode::Write);
    authorized << 1;

    m_out << "Authorized\n";
    if (m_sl == 2)
    {
        File keyACL(acltree / readAndTrim(uniqueIDFilename) / keyFilename,
                    File::Mode::Write,
                    O_CREAT,
                    S_IRUSR);
        keyACL << keyStream.str();
        m_out << "Key saved in ACL\n";
    }
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
    auto acl = acltree / readAndTrim(uniqueIDFilename);
    if (fs::exists(acl))
    {
        m_out << "Already in ACL\n";
        return;
    }

    fs::create_directories(acl);
    fs::copy(vendorFilename, acl / vendorFilename);
    fs::copy(deviceFilename, acl / deviceFilename);

    m_out << "Added to ACL\n";
}

void tbtadm::Controller::acl()
{
    if (!fs::exists(acltree) || fs::is_empty(acltree))
    {
        m_out << "ACL is empty\n";
        return;
    }

    // Get UUID of all connected devices
    std::vector<std::string> uuids;
    if (fs::exists(sysfsDevicesPath))
    {
        for (auto dir : boost::make_iterator_range(
                 fs::directory_iterator(sysfsDevicesPath), {}))
        {
            if (dir.status().type() != fs::directory_file)
            {
                continue;
            }
            if (!isDevice(dir.path()))
            {
                continue;
            }
            uuids.push_back(readAndTrim(dir.path() / uniqueIDFilename));
        }
        m_sl = findSL();
    }

    auto connected = [&](const auto& uuid) {
        return std::find(cbegin(uuids), cend(uuids), uuid) != cend(uuids)
                   ? "connected"
                   : "not connected";
    };

    // Print ACL
    bool doNoKey = false;
    for (auto dir :
         boost::make_iterator_range(fs::directory_iterator(acltree), {}))
    {
        const auto p = dir.path();
        if (m_sl != 2 || fs::exists(p / keyFilename))
        {
            const auto uuid = p.filename().string();
            m_out << uuid << '\t' << readAndTrim(p / vendorFilename) << '\t'
                  << readAndTrim(p / deviceFilename) << '\t' << connected(uuid)
                  << '\n';
        }
        else
        {
            doNoKey = true;
        }
    }
    if (doNoKey)
    {
        m_out << "\nACL entries with no key (not for current security mode):\n";
        for (auto dir :
             boost::make_iterator_range(fs::directory_iterator(acltree), {}))
        {
            const auto p = dir.path();
            if (!fs::exists(p / keyFilename))
            {
                const auto uuid = p.filename().string();
                m_out << uuid << '\t' << readAndTrim(p / vendorFilename) << '\t'
                      << readAndTrim(p / deviceFilename) << '\t'
                      << connected(uuid) << '\n';
            }
        }
    }
}

// TODO: move to tbtadm-helper
void tbtadm::Controller::remove(std::string uuid)
{
    // Identify route-string argument and replace it with the UUID
    if (isRouteString(uuid))
    {
        uuid = readAndTrim(sysfsDevicesPath / uuid / uniqueIDFilename);
    }

    auto acl = acltree / uuid;
    if (!fs::exists(acl))
    {
        m_out << "ACL entry doesn't exist\n";
    }
    fs::remove_all(acl);
}

// TODO: move to tbtadm-helper
void tbtadm::Controller::removeAll()
{
    if (!fs::exists(acltree) || fs::is_empty(acltree))
    {
        m_out << "ACL is empty\n";
        return;
    }
    auto count =
        std::count_if(fs::directory_iterator(acltree),
                      {},
                      [](const auto& dir) { return fs::is_directory(dir); });
    fs::remove_all(acltree);
    m_out << count << " entries removed\n";
}
