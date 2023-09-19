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

#if defined ZSPACE_USD_INTEROP
//#include "LiveSessionConfigFile.h"
#include <headers/zInterOp/core/omniUtils/LiveSessionConfigFile.h>
#include "OmniClient.h"
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <sstream>      // std::ostringstream
#include <string>
#include <toml/toml.h>


std::string LiveSessionConfigFile::GetSessionConfigValue(const char* sessionConfigFileUrl, LiveSessionConfigFile::Key key)
{
    std::string retStr("");
    toml::Value tomlValue;

    // This is a callback function for when the file is read from Nucleus
    // it will parse the contents of the toml file and 
    auto readFileCallback = [](void* userData, OmniClientResult result, char const* version, struct OmniClientContent* content) noexcept
    {
        if (result == OmniClientResult::eOmniClientResult_Ok && content && userData)
        {
            toml::Value* tomlValuePtr = static_cast<toml::Value*>(userData);
            std::istringstream ss(std::string((char*)content->buffer, content->size));

            toml::ParseResult pr = toml::parse(ss);
            if (!pr.valid()) {
                std::cout << "Reading the Session Config file failed (toml parse): " << pr.errorReason << std::endl;
                return;
            }
            *tomlValuePtr = pr.value;
        }
    };

    // Read the file from Nucleus
    omniClientWait(omniClientReadFile(sessionConfigFileUrl, &tomlValue, readFileCallback));

    // We should verify that the version is compatible here...

    const toml::Value* x = tomlValue.find(KeyToString(key));
    if (x && x->is<std::string>())
    {
        retStr = x->as<std::string>();
    }
    return retStr;
}

std::string LiveSessionConfigFile::GetSessionAdmin(const char* sessionConfigFileUrl)
{
    return GetSessionConfigValue(sessionConfigFileUrl, LiveSessionConfigFile::Key::Admin);
}

bool LiveSessionConfigFile::CreateSessionConfigFile(const char* sessionConfigFileUrl, const std::map<LiveSessionConfigFile::Key, const char*>& keyMap)
{
    bool bFileWriteSuccess = false;
    toml::Value root((toml::Table()));
    for (const auto& key : keyMap)
    {
        root.setChild(KeyToString(key.first), key.second);
    }

    // Add a version
    root.setChild(KeyToString(Key::Version), kCurrentVersion);

    std::ostringstream oss;
    root.write(&oss);
    OmniClientContent content = omniClientAllocContent(oss.str().length());
    std::memcpy(content.buffer, oss.str().c_str(), content.size);
    omniClientWait(omniClientWriteFile(
        sessionConfigFileUrl,
        &content,
        &bFileWriteSuccess,
        [](void* userData, OmniClientResult result) noexcept
        {
            bool* success = static_cast<bool*>(userData);
            *success = (result == OmniClientResult::eOmniClientResult_Ok);
        }));

    return bFileWriteSuccess;
}

bool LiveSessionConfigFile::IsVersionCompatible(const char* sessionConfigFileUrl)
{
    std::string configVersion = GetSessionConfigValue(sessionConfigFileUrl, Key::Version);
    if (configVersion.empty())
    {
        return false;
    }
    else
    {
        // Get the TOML file major version
        std::istringstream tokenStream(configVersion);
        std::string configMajorVersion;
        std::getline(tokenStream, configMajorVersion, '.');

        // Get the _current_ major version
        std::istringstream tokenStream2(kCurrentVersion);
        std::string currentMajorVersion;
        std::getline(tokenStream2, currentMajorVersion, '.');

        return (configMajorVersion == currentMajorVersion);
    }
}

std::string LiveSessionConfigFile::KeyToString(Key key)
{
    switch (key)
    {
    case(Key::Version): return "version"; break;
    case(Key::Admin): return "user_name"; break;
    case(Key::StageUrl): return "stage_url"; break;
    case(Key::Description): return "description"; break;
    case(Key::Mode): return "mode"; break;
    case(Key::Name): return "name"; break;
    default: return "";
    }
}

LiveSessionConfigFile::Key LiveSessionConfigFile::StringToKey(const std::string& keyString)
{
    if ("version" == keyString) return Key::Version;
    else if ("user_name" == keyString) return Key::Admin;
    else if ("stage_url" == keyString) return Key::StageUrl;
    else if ("description" == keyString) return Key::Description;
    else if ("mode" == keyString) return Key::Mode;
    else if ("name" == keyString) return Key::Name;
    else return Key::Invalid;
}

#endif