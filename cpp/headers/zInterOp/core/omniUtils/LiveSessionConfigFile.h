/*###############################################################################
#
# Copyright 2022 NVIDIA Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
###############################################################################*/

#if defined(ZSPACE_USD_INTEROP) 

#ifndef ZSPACE_INTEROP_OMNILIVECONFIG_H
#define ZSPACE_INTEROP_OMNILIVECONFIG_H

#pragma once

#include <string>
#include <map>

/*
 * Live Session Config File class
 *
 * This class is intended to assist in reading and writing the live session configuration file.
 *
 * The config file is TOML-formatted and is primarily used to denote the owner, or admin of the
 * live session.
 *
 * possible modes - "default" = "root_authoring", "auto_authoring", "project_authoring"
*/

class LiveSessionConfigFile
{
public:
    enum class Key
    {
        Version,
        Admin,
        StageUrl,
        Description,
        Mode,
        Name,
        Invalid
    };

    using KeyMap = std::map<LiveSessionConfigFile::Key, const char*>;
    static constexpr const char* kCurrentVersion = "1.0";

    LiveSessionConfigFile() {};
    ~LiveSessionConfigFile() {};

    std::string GetSessionConfigValue(const char* sessionConfigFileUrl, LiveSessionConfigFile::Key key);
    bool CreateSessionConfigFile(const char* sessionConfigFileUrl, const LiveSessionConfigFile::KeyMap& keyMap);
    std::string GetSessionAdmin(const char* sessionConfigFileUrl);

    /* Check that a version is compatible [major.minor]
     * If major is the same, return true, else return false
     * This works under the assumption that future minor versions will still work
     */
    bool IsVersionCompatible(const char* sessionConfigFileUrl);

    std::string KeyToString(Key key);
    Key StringToKey(const std::string& keyString);
};

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterOp/core/omniUtils/LiveSessionConfigFile.cpp>
#endif


#endif

#endif