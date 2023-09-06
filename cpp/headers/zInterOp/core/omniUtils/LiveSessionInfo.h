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

#ifndef ZSPACE_INTEROP_OMNILIVESESSION_H
#define ZSPACE_INTEROP_OMNILIVESESSION_H

#pragma once

#include <string>
#include <vector>

/*
 * Live Session Info class
 *
 * This class attempts to collect all of the logic around the live session file paths and URLs.
 * It should be first instantiated with the stage URL (omniverse://server/folder/stage.usd), then
 * GetSessionFolderPathForStage() can be used to list the available sessions.
 *
 * Once a session is selected, SetSessionName() will finish the initialization of all of the paths
 * and the other methods can be used.
 *
 * In the folder that contains the USD to be live-edited, there exists this folder structure:
 * <.live> / < my_usd_file.live> / <session_name.live> / root.live

 * GetSessionFolderPathForStage:   <stage_folder> / <.live> / <my_usd_file.live>
 * GetLiveSessionFolderPath:       <stage_folder> / <.live> / <my_usd_file.live> / <session_name.live>
 * GetLiveSessionUrl:              <stage_folder> / <.live> / <my_usd_file.live> / <session_name.live> / root.live
 * GetLiveSessionTomlUrl:          <stage_folder> / <.live> / <my_usd_file.live> / <session_name.live> / __session__.toml
 * GetMessageChannelUrl:           <stage_folder> / <.live> / <my_usd_file.live> / <session_name.live> / __session__.channel
*/

class LiveSessionInfo
{
public:
    const char* OMNIVERSE_CHANNEL_FILE_NAME = "__session__.channel";
    const char* LIVE_SUBFOLDER = "/.live";
    const char* LIVE_SUBFOLDER_SUFFIX = ".live";
    const char* DEFAULT_LIVE_FILE_NAME = "root.live";
    const char* SESSION_TOML_FILE_NAME = "__session__.toml";

    LiveSessionInfo() {}
    LiveSessionInfo(const char* stageUrl);
    virtual ~LiveSessionInfo() {}

    // Initialize the live session info pahts
    void Initialize(const char* stageUrl);
    std::string BuildUrlString(const char* path);
    std::string GetSessionFolderPathForStage();

    // Much of the class methods don't function correctly until this method is called
    // Session names must start with an alphabetical character, but may contain alphanumeric,
    //  hyphen, or underscore characters.
    // 
    // Returns true if the name is valid, false if it contains invalid characters
    bool SetSessionName(const char* session_name);

    // Get the live session name
    std::string GetSessionName() { return mSessionName; }

    // This checks for the existance of the session config (TOML) file
    bool DoesSessionExist();

    // This returns the absolute path of the live session folder where live session files live
    // * root.live           - the live layer with the live session USD contents
    // * __session__.toml    - the session config file
    // * __session__.channel - the omni channel file
    std::string GetLiveSessionFolderPath();

    // Returns a list of existing sessions for the stage
    std::vector<std::string> GetLiveSessionList();

    // Returns the root stage URL for the live session
    std::string GetStageUrl() { return mStageUrlStr; }

    // Get the URL for the folder in which the stage resides
    std::string GetStageFolderUrl();

    // Get the filename of the root stage
    std::string GetStageFileName();

    // Get the URL for the .live layer file
    std::string GetLiveSessionUrl();

    // Get the URL for the TOML configuration file
    std::string GetLiveSessionTomlUrl();

    // Get the URL for the Omniverse Channel file
    std::string GetMessageChannelUrl();

    // A utility method for the class or any application - will report if a file exists
    // on local disk or on a Nucleus server
    bool fileExists(const char* fileUrl);

private:
    struct OmniClientUrl* mOmniUrl = nullptr;
    std::string mStageUrlStr;
    std::string mStageFolderUrlStr;
    std::string mLiveFileUrlStr;
    std::string mSessionName;
    std::string mOmniSessionFolderPath;
    std::string mSessionFolderString;
};


#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterOp/core/omniUtils/LiveSessionInfo.cpp>
#endif


#endif

#endif