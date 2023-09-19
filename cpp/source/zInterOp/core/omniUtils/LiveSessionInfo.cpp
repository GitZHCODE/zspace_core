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

//#include "LiveSessionInfo.h"
#include <headers/zInterOp/core/omniUtils/LiveSessionInfo.h>
#include <OmniClient.h>
#include <algorithm>
#include <regex>
#ifdef _WIN32
#include <conio.h>
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

LiveSessionInfo::LiveSessionInfo(const char* stage_url)
{
    Initialize(stage_url);
}

void LiveSessionInfo::Initialize(const char* stageUrl)
{
    mStageUrlStr = stageUrl;
    mOmniUrl = omniClientBreakUrl(stageUrl);
    // construct the folder that would contain sessions - <.live> / < my_usd_file.live> / <session_name> / root.live
    // self.omni_session_folder_path = os.path.dirname(self.omni_url.path) + self.LIVE_SUBFOLDER + "/" + self.get_stage_file_name() + self.LIVE_SUBFOLDER_SUFFIX
    // self.session_folder_string = omni.client.make_url(self.omni_url.scheme, self.omni_url.user, self.omni_url.host, self.omni_url.port, self.omni_session_folder_path
    fs::path p(mOmniUrl->path);

    // Because std::experimental::filesystem normalizes the path (on Windows) and we need forward slashes
    std::string parent_path_str = p.parent_path().string();
    std::replace(parent_path_str.begin(), parent_path_str.end(), '\\', '/');
    mStageFolderUrlStr = BuildUrlString(parent_path_str.c_str());
    mOmniSessionFolderPath = parent_path_str + LIVE_SUBFOLDER + "/" + GetStageFileName() + LIVE_SUBFOLDER_SUFFIX;
    mSessionFolderString = BuildUrlString(mOmniSessionFolderPath.c_str());
}

std::string LiveSessionInfo::BuildUrlString(const char* path)
{
    std::string urlString;
    OmniClientUrl omniUrl = *mOmniUrl;
    omniUrl.path = path;

    size_t expectedSize = 0;
    omniClientMakeUrl(&omniUrl, nullptr, &expectedSize);
    char* buf = new char[expectedSize];
    omniClientMakeUrl(&omniUrl, buf, &expectedSize);
    urlString.assign(buf);
    delete[] buf;
    return urlString;
}

std::string LiveSessionInfo::GetSessionFolderPathForStage()
{
    return mSessionFolderString;
}

bool LiveSessionInfo::SetSessionName(const char* sessionName)
{
    // Session names must start with an alphabetical character, but may contain alphanumeric, hyphen, or underscore characters.
    std::regex re("^[a-zA-Z][a-zA-Z0-9_-]+");
    std::cmatch match;
    if (std::regex_match(sessionName, match, re))
    {
        mSessionName.assign(sessionName);
        return true;
    }
    else
    {
        return false;
    }
}

bool LiveSessionInfo::DoesSessionExist()
{
    return fileExists(GetLiveSessionTomlUrl().c_str());
}

std::string LiveSessionInfo::GetLiveSessionFolderPath()
{
    return mOmniSessionFolderPath + "/" + mSessionName + LIVE_SUBFOLDER_SUFFIX;
}

std::vector<std::string> LiveSessionInfo::GetLiveSessionList()
{
    std::string sessionFolderForStage = GetSessionFolderPathForStage();
    std::vector<std::string> folders;
    omniClientWait(omniClientList(sessionFolderForStage.c_str(), &folders, [](void* userData, OmniClientResult result, uint32_t numEntries, struct OmniClientListEntry const* entries) noexcept {
        std::vector<std::string>* foldersPtr = static_cast<std::vector<std::string>*>(userData);
        for (uint32_t i = 0; i < numEntries; i++)
        {
            foldersPtr->push_back(entries[i].relativePath);
        }
        }));

    std::vector<std::string> sessionNames;
    for (int i = 0; i < folders.size(); i++)
    {
        fs::path p(folders[i]);
        sessionNames.push_back(p.stem().string());
    }
    return sessionNames;
}

std::string LiveSessionInfo::GetStageFolderUrl()
{
    return mStageFolderUrlStr;
}

std::string LiveSessionInfo::GetStageFileName()
{
    // find the stage file's root name
    fs::path p(mOmniUrl->path);
    return p.stem().string();
}

std::string LiveSessionInfo::GetLiveSessionUrl()
{
    std::string live_session_path = GetLiveSessionFolderPath() + "/" + DEFAULT_LIVE_FILE_NAME;
    return BuildUrlString(live_session_path.c_str());
}

std::string LiveSessionInfo::GetLiveSessionTomlUrl()
{
    std::string live_session_toml_path = GetLiveSessionFolderPath() + "/" + SESSION_TOML_FILE_NAME;
    return BuildUrlString(live_session_toml_path.c_str());
}

std::string LiveSessionInfo::GetMessageChannelUrl()
{
    std::string live_session_channel_path = GetLiveSessionFolderPath() + "/" + OMNIVERSE_CHANNEL_FILE_NAME;
    return BuildUrlString(live_session_channel_path.c_str());
}

bool LiveSessionInfo::fileExists(const char* fileUrl)
{
    bool bFileExists = false;
    omniClientWait(omniClientStat(
        fileUrl,
        &bFileExists,
        [](void* userData, OmniClientResult result, struct OmniClientListEntry const* entry) noexcept
        {
            bool* fileExists = static_cast<bool*>(userData);
            *fileExists = (result == eOmniClientResult_Ok);
        }
    ));
    return bFileExists;
}

#endif