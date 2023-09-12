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

//#include "OmniChannel.h"
#include <headers/zInterOp/core/omniUtils/OmniChannel.h>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#undef snprintf
#include <nlohmann/json.hpp>
#include <OmniClient.h>


// OmniChannelMessage PIMPL
class OmniChannelMessageImpl
{
public:

    // Setup a message that's been received
    OmniChannelMessageImpl(OmniClientChannelEvent eventType, char const* fromUserId, struct OmniClientContent* content)
        : mEventType(eventType)
    {
        if (fromUserId)
        {
            mFromUserId = fromUserId;
        }
        if (mEventType == eOmniClientChannelEvent_Message &&
            content && content->buffer && content->size > OmniChannelMessage::MSG_HEADER_LEN)
        {
            // Check the first 8 bytes is "__OVUM__"
            std::string bufferStr((char*)content->buffer, content->size);
            if (OmniChannelMessage::MSG_HEADER == bufferStr.substr(0, OmniChannelMessage::MSG_HEADER_LEN))
            {
                mJson = nlohmann::json::parse(bufferStr.substr(OmniChannelMessage::MSG_HEADER_LEN));
                //std::cout << "Json string: " << mJson << std::endl;
            }

            //std::cout << GetType() << std::endl;
        }
    }

    OmniChannelMessageImpl() {}
    ~OmniChannelMessageImpl() = default;

    // Build a message for sending
    void BuildMessage(OmniChannelMessage::MessageType messageType, const char* fromUserName, const char* app)
    {
        mJson[OmniChannelMessage::MSG_VERSION_KEY] = OmniChannelMessage::MSG_VERSION;
        mJson[OmniChannelMessage::MSG_USERNAME_KEY] = fromUserName;
        mJson[OmniChannelMessage::MSG_APP_KEY] = app;
        nlohmann::json emptyContent;
        //mJson["content"] = emptyContent;

        if (messageType == OmniChannelMessage::MessageType::MergeStarted || messageType == OmniChannelMessage::MessageType::MergeFinished)
        {
            mJson[OmniChannelMessage::MSG_TYPE_KEY] = OmniChannelMessage::MESSAGE_MSG_TYPE;
            nlohmann::json sessionContentMsg = {
                {OmniChannelMessage::CUSTOM_MSG_GROUP_KEY, {
                        {OmniChannelMessage::MSG_VERSION_KEY, OmniChannelMessage::CUSTOM_MSG_VERSION}, // Currently not sent by Kit
                        {OmniChannelMessage::CUSTOM_MSG_TYPE,  OmniChannelMessage::MessageTypeToStringType(messageType)}
                    },
                }
            };
            mJson[OmniChannelMessage::MSG_CONTENT_KEY] = sessionContentMsg;
        }
        else
        {
            mJson[OmniChannelMessage::MSG_TYPE_KEY] = OmniChannelMessage::MessageTypeToStringType(messageType);
        }
        //std::cout << "Build message: " << mJson << std::endl;
    }

    OmniClientContent GetOmniClientContent()
    {
        std::string msg = OmniChannelMessage::MSG_HEADER + mJson.dump();
        mContent = omniClientAllocContent(msg.length());
        std::memcpy(mContent.buffer, msg.c_str(), msg.length());
        return mContent;
    }

    std::string GetVersion() const
    {
        if (mJson.find(OmniChannelMessage::MSG_VERSION_KEY) == mJson.end())
        {
            return "";
        }
        else
        {
            return mJson[OmniChannelMessage::MSG_VERSION_KEY].get<std::string>();
        }
    }

    std::string GetFromUserId() const
    {
        return mFromUserId;
    }

    std::string GetFromUser() const
    {
        return mJson[OmniChannelMessage::MSG_USERNAME_KEY].get<std::string>();
    }

    std::string GetApp() const
    {
        return mJson[OmniChannelMessage::MSG_APP_KEY].get<std::string>();
    }

    OmniChannelMessage::MessageType GetType() const
    {
        if (OmniChannelMessage::MESSAGE_MSG_TYPE == mJson[OmniChannelMessage::MSG_TYPE_KEY])
        {
            nlohmann::json customContent = mJson[OmniChannelMessage::MSG_CONTENT_KEY];
            nlohmann::json sessionMsg = customContent[OmniChannelMessage::CUSTOM_MSG_GROUP_KEY];
            // There's currently no version check
            //if ("1.0" == sessionMsg["version"])
            {
                return OmniChannelMessage::StringTypeToMessageType(sessionMsg[OmniChannelMessage::CUSTOM_MSG_TYPE].get<std::string>());
            }
        }
        return OmniChannelMessage::StringTypeToMessageType(mJson[OmniChannelMessage::MSG_TYPE_KEY].get<std::string>());
    }

    void SetOmniChannelEventType(OmniClientChannelEvent event)
    {
        mEventType = event;
    }

    OmniClientChannelEvent GetOmniChannelEventType() const
    {
        return mEventType;
    }

    OmniClientChannelEvent mEventType;
    std::string mFromUserId;
    struct OmniClientContent mContent;
    nlohmann::json mJson;
};




// OmniChannelMessage Interface
OmniChannelMessage::OmniChannelMessage()
    : m_pImpl(new OmniChannelMessageImpl())
{}

// Setup a message that's been received
OmniChannelMessage::OmniChannelMessage(OmniClientChannelEvent eventType, char const* fromUserId, struct OmniClientContent* content)
    : m_pImpl(new OmniChannelMessageImpl(eventType, fromUserId, content))
{}

OmniChannelMessage::OmniChannelMessage(const OmniChannelMessage& src)
    : m_pImpl(new OmniChannelMessageImpl(*src.m_pImpl))
{}

OmniChannelMessage::~OmniChannelMessage()
{}

// Build a message for sending
void OmniChannelMessage::BuildMessage(MessageType messageType, const char* fromUserName, const char* app)
{
    Pimpl()->BuildMessage(messageType, fromUserName, app);
}

OmniClientContent OmniChannelMessage::GetOmniClientContent()
{
    return Pimpl()->GetOmniClientContent();
}

std::string OmniChannelMessage::GetVersion() const
{
    return Pimpl()->GetVersion();
}

std::string OmniChannelMessage::GetFromUserId() const
{
    return Pimpl()->GetFromUserId();
}

std::string OmniChannelMessage::GetFromUser() const
{
    return Pimpl()->GetFromUser();
}

std::string OmniChannelMessage::GetApp() const
{
    return Pimpl()->GetApp();
}

OmniChannelMessage::MessageType OmniChannelMessage::GetType() const
{
    return Pimpl()->GetType();
}

void OmniChannelMessage::SetOmniChannelEventType(OmniClientChannelEvent event)
{
    Pimpl()->SetOmniChannelEventType(event);
}

OmniClientChannelEvent OmniChannelMessage::GetOmniChannelEventType() const
{
    return Pimpl()->GetOmniChannelEventType();
}




// OmniChannel PIMPL
class OmniChannelImpl
{
public:
    OmniChannelImpl() {}

    ~OmniChannelImpl() = default;

    void SetChannelUrl(const char* joinChannelUrl)
    {
        mChannelUrl.assign(joinChannelUrl);
    }

    void SetAppName(const char* appName)
    {
        mAppName.assign(appName);
    }

    void RegisterNotifyCallback(OmniChannel::NotifyCallback& cb, void* userData = nullptr)
    {
        mNotifyCallback = &cb;
        mNotifyCallbackUserData = userData;
    }

    void ClearNotifyCallback()
    {
        mNotifyCallback = nullptr;
        mNotifyCallbackUserData = nullptr;
    }

    bool JoinChannel()
    {
        auto ChannelMessageCallback = [](void* userData, OmniClientResult result, OmniClientChannelEvent eventType, char const* fromUser, struct OmniClientContent* content) noexcept
        {
            OmniChannelImpl* omniChannelContext = static_cast<OmniChannelImpl*>(userData);
            if (!omniChannelContext)
            {
                std::cerr << "omniClientJoinChannel callback context invalid" << std::endl;
                return;
            }

            if (result != eOmniClientResult_Ok)
            {
                return;
            }

            {
                std::unique_lock<std::mutex> lk(omniChannelContext->mMutex);
                omniChannelContext->mMessageQueue.push(OmniChannelMessage(eventType, fromUser, content));
            }
        };

        mRequestId = omniClientJoinChannel(mChannelUrl.c_str(), this, ChannelMessageCallback);

        // Synchronously wait to join the message channel
        omniClientWait(mRequestId);

        // Get the username for the connection
        omniClientWait(omniClientGetServerInfo(mChannelUrl.c_str(), &mUserName, [](void* userData, OmniClientResult result, struct OmniClientServerInfo const* info) noexcept
            {
                std::string* userName = static_cast<std::string*>(userData);
                if (userData && userName && info && info->username)
                {
                    userName->assign(info->username);
                }
            }));

        // Send when joining a channel, don't send when just peering in and getting users
        SendChannelMessage(OmniChannelMessage::MessageType::Join);
        return true;
    }

    void LeaveChannel()
    {
        // Clear the peer users
        {
            std::unique_lock<std::mutex> lk(mMutex);
            mPeerUsers.clear();
        }

        SendChannelMessage(OmniChannelMessage::MessageType::Left);
        omniClientStop(mRequestId);
        mRequestId = kInvalidRequestId;
    }

    bool IsConnected()
    {
        return (mRequestId != kInvalidRequestId);
    }

    OmniChannel::MessageVector IterateMessages(bool bReturnCopies, bool bCallNotifyCallbacks, bool bSendReplies)
    {
        bool bSendHello = false;
        OmniChannel::MessageVector messages;
        while (!mMessageQueue.empty())
        {
            std::unique_lock<std::mutex> lk(mMutex);
            const OmniChannelMessage& msg = mMessageQueue.front();

            std::string eventTypeStr;
            if (msg.GetOmniChannelEventType() == eOmniClientChannelEvent_Message)
            {
                // NOTE!  This check is done here, but ultimately care should be taken by apps to
                // respond appropriately to an invalid message version.  It's possible that new message
                // versions could cause problems with this app, and maybe this client should disconnect.
                if (msg.GetVersion() != OmniChannelMessage::MSG_VERSION)
                {
                    std::cout << "Message version sent from user " << msg.GetFromUserId() << " does not match expected version: " <<
                        OmniChannelMessage::MSG_VERSION <<
                        ", username: " << msg.GetFromUser() <<
                        ", app: " << msg.GetApp() <<
                        ", message type: " << OmniChannelMessage::MessageTypeToStringType(msg.GetType()) << std::endl;
                }

                // This is optional, if the client is polling rather than using "Update"
                if (bReturnCopies)
                {
                    messages.push_back(msg);
                }

                if (OmniChannelMessage::MessageType::GetUsers == msg.GetType() ||
                    OmniChannelMessage::MessageType::Join == msg.GetType())
                {
                    bSendHello = true;
                }

                // Add a new peer user to the set
                if (OmniChannelMessage::MessageType::Hello == msg.GetType() ||
                    OmniChannelMessage::MessageType::Join == msg.GetType())
                {
                    mPeerUsers.insert(OmniPeerUser(msg.GetFromUserId(), msg.GetFromUser(), msg.GetApp()));
                }
                else if (OmniChannelMessage::MessageType::Left == msg.GetType())
                {
                    mPeerUsers.erase(OmniPeerUser(msg.GetFromUserId(), msg.GetFromUser(), msg.GetApp()));
                }

                // This is optional, if the client would prefer the async-callback style
                if (mNotifyCallback && bCallNotifyCallbacks)
                {
                    mNotifyCallback(msg.GetType(), mNotifyCallbackUserData, msg.GetFromUser().c_str(), msg.GetApp().c_str());
                }
            }
            mMessageQueue.pop();
        }
        // Send this msg while NOT holding the message queue lock
        // This is optional
        if (bSendHello && bSendReplies)
        {
            SendChannelMessage(OmniChannelMessage::MessageType::Hello);
        }

        return messages;
    }

    void Update()
    {
        IterateMessages(
            false,  /* return message copies */
            true,   /* call notify callbacks */
            true);  /* send GET_USER replies */
    }

    OmniChannel::MessageVector GetAllMessages()
    {
        return IterateMessages(
            true,   /* return message copies */
            false,  /* call notify callbacks */
            true);  /* send GET_USER replies */
    }

    void SendChannelMessage(OmniChannelMessage::MessageType messageType)
    {
        OmniChannelMessage msg;
        msg.BuildMessage(messageType, mUserName.c_str(), mAppName.c_str());
        OmniClientContent content = msg.GetOmniClientContent();
        omniClientWait(omniClientSendMessage(mRequestId, &content, nullptr, [](void* userData, OmniClientResult result) noexcept {}));
    }

    size_t GetUsersCount() const
    {
        return mPeerUsers.size();
    }

    const OmniPeerUser& GetUserAtIndex(size_t index) const
    {
        std::set<OmniPeerUser, OmniPeerUser::Compare>::iterator it = mPeerUsers.begin();
        std::advance(it, index);
        return *it;
    }

    std::string mChannelUrl;
    OmniClientRequestId mRequestId = kInvalidRequestId;
    std::queue<OmniChannelMessage> mMessageQueue;
    std::mutex mMutex;
    std::string mUserName;
    std::string mAppName;
    std::set<OmniPeerUser, OmniPeerUser::Compare> mPeerUsers;
    OmniChannel::NotifyCallback* mNotifyCallback = nullptr;
    void* mNotifyCallbackUserData = nullptr;
};



// OmniChannel interface impl

OmniChannel::OmniChannel() : m_pImpl(new OmniChannelImpl())
{}

OmniChannel::~OmniChannel()
{
    if (IsConnected())
    {
        LeaveChannel();
    }
}

void OmniChannel::SetChannelUrl(const char* joinChannelUrl)
{
    Pimpl()->SetChannelUrl(joinChannelUrl);
}

void OmniChannel::SetAppName(const char* appName)
{
    Pimpl()->SetAppName(appName);
}

void OmniChannel::RegisterNotifyCallback(NotifyCallback& cb, void* userData)
{
    Pimpl()->RegisterNotifyCallback(cb, userData);
}

void OmniChannel::ClearNotifyCallback()
{
    Pimpl()->ClearNotifyCallback();
}

bool OmniChannel::JoinChannel()
{
    return Pimpl()->JoinChannel();
}

void OmniChannel::LeaveChannel()
{
    Pimpl()->LeaveChannel();
}

bool OmniChannel::IsConnected()
{
    return Pimpl()->IsConnected();
}

void OmniChannel::Update()
{
    Pimpl()->Update();
}

OmniChannel::MessageVector OmniChannel::GetAllMessages()
{
    return Pimpl()->GetAllMessages();
}

void OmniChannel::SendChannelMessage(OmniChannelMessage::MessageType messageType)
{
    Pimpl()->SendChannelMessage(messageType);
}

size_t OmniChannel::GetUsersCount() const
{
    return Pimpl()->GetUsersCount();
}

const OmniPeerUser& OmniChannel::GetUserAtIndex(size_t index) const
{
    return Pimpl()->GetUserAtIndex(index);
}


