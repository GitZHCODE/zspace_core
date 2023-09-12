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

#ifndef ZSPACE_INTEROP_OMNICHANNEL_H
#define ZSPACE_INTEROP_OMNICHANNEL_H

#pragma once

#include <OmniClient.h>
#include <string>
#include <memory>
#include <vector>


/*
 * Omniverse Channel Message class
 *
 * This class describes a JSON-formatted message that is sent and received through
 * the Omniverse channel file contained in the live session folder.
*/
class OmniChannelMessageImpl;
class OmniChannelMessage
{
public:

    enum class MessageType
    {
        Join,
        Hello,
        GetUsers,
        Left,
        MergeStarted,
        MergeFinished,
        NumMsgTypes
    };

    static constexpr const char* MSG_HEADER = "__OVUM__";
    static constexpr const size_t MSG_HEADER_LEN = 8;
    static constexpr const char* MSG_VERSION = "3.0";
    static constexpr const char* CUSTOM_MSG_TYPE = "message";
    static constexpr const char* MESSAGE_MSG_TYPE = "MESSAGE";
    static constexpr const char* CUSTOM_MSG_GROUP_KEY = "__SESSION_MANAGEMENT__";
    static constexpr const char* CUSTOM_MSG_VERSION = "1.0";
    
    static constexpr const char* MSG_VERSION_KEY = "version";
    static constexpr const char* MSG_USERNAME_KEY = "from_user_name";
    static constexpr const char* MSG_APP_KEY = "app";
    static constexpr const char* MSG_TYPE_KEY = "message_type";
    static constexpr const char* MSG_CONTENT_KEY = "content";


    // Setup a message that's been received
    OmniChannelMessage(OmniClientChannelEvent eventType, char const* fromUserId, struct OmniClientContent* content);
    OmniChannelMessage();
    OmniChannelMessage(const OmniChannelMessage& src);

    void BuildMessage(MessageType messageType, const char* fromUserName, const char* app);
    OmniClientContent GetOmniClientContent();
    ~OmniChannelMessage();
    std::string GetVersion() const;
    std::string GetFromUserId() const;
    std::string GetFromUser() const;
    std::string GetApp() const;
    MessageType GetType() const;
    void SetOmniChannelEventType(OmniClientChannelEvent event);
    OmniClientChannelEvent GetOmniChannelEventType() const;

    static MessageType StringTypeToMessageType(const std::string& msgStr)
    {
        if ("HELLO" == msgStr) return MessageType::Hello;
        else if ("JOIN" == msgStr) return MessageType::Join;
        else if ("GET_USERS" == msgStr) return MessageType::GetUsers;
        else if ("LEFT" == msgStr) return MessageType::Left;
        else if ("MERGE_STARTED" == msgStr) return MessageType::MergeStarted;
        else if ("MERGE_FINISHED" == msgStr) return MessageType::MergeFinished;
        else return MessageType::NumMsgTypes;
    }

    static const char* MessageTypeToStringType(MessageType msgType)
    {
        if (msgType == MessageType::Hello) return "HELLO";
        else if (msgType == MessageType::Join) return "JOIN";
        else if (msgType == MessageType::GetUsers) return "GET_USERS";
        else if (msgType == MessageType::Left) return "LEFT";
        else if (msgType == MessageType::MergeStarted) return "MERGE_STARTED";
        else if (msgType == MessageType::MergeFinished) return "MERGE_FINISHED";
        else return "INVALID";
    }
private:
    const OmniChannelMessageImpl* Pimpl() const { return m_pImpl.get(); }
    OmniChannelMessageImpl* Pimpl() { return m_pImpl.get(); }
    std::unique_ptr<OmniChannelMessageImpl> m_pImpl;
};


/*
 * Omniverse Peer User struct
 *
 * This struct wraps the items that denote a peer user in the live session.
 *
 *  userName: the login name for the connected user
 *  app:      the application used to connect to the live session
 *  userId:   the Nucleus user id, unique to every connection (regardless of the user or application)
*/
struct OmniPeerUser
{
	OmniPeerUser(const std::string& id, const std::string& name, const std::string& a)
		: userId(id)
		, userName(name)
		, app(a)
	{}

	struct Compare{
		bool operator()(const OmniPeerUser& lhs, const OmniPeerUser& rhs) const {
			return (lhs.userName + lhs.app + lhs.userId) < (rhs.userName + rhs.app + rhs.userId);
		}
	};

	std::string userId;
	std::string userName;
	std::string app;
};


/*
 * Omniverse Channel class
 *
 * This class contains methods to access the Omniverse live session channel for communicating
 * user presence, application name, and merge status.
 */
class OmniChannelImpl;
class OmniChannel
{
public:
    typedef void (NotifyCallback)(OmniChannelMessage::MessageType messageType, void* userData, const char* userName, const char* appName);
    using MessageVector = std::vector<OmniChannelMessage>;

    OmniChannel();
    ~OmniChannel();

    void SetChannelUrl(const char* joinChannelUrl);
    void SetAppName(const char* appName);
    void RegisterNotifyCallback(NotifyCallback& cb, void* userData = nullptr);
    void ClearNotifyCallback();
    bool JoinChannel();
    void LeaveChannel();
    bool IsConnected();

    // This method needs to be periodically called by the application to flush messages
    // from the queue and call notify callbacks
    void Update();

    // If an application wants to poll the channel to process and fetch messages
    // (rather than have Update issue callbacks)
    MessageVector GetAllMessages();

    void SendChannelMessage(OmniChannelMessage::MessageType messageType);
    size_t GetUsersCount() const;
    const OmniPeerUser& GetUserAtIndex(size_t index) const;

private:
    const OmniChannelImpl* Pimpl() const { return m_pImpl.get(); }
    OmniChannelImpl* Pimpl() { return m_pImpl.get(); }
    std::unique_ptr<OmniChannelImpl> m_pImpl;
};

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterOp/core/omniUtils/OmniChannel.cpp>
#endif


#endif

#endif