// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//


#if defined ZSPACE_USD_INTEROP

#include "headers/zInterOp/core/zOmniCore.h"

namespace zSpace
{
	UsdStageRefPtr zOmniCore::gStage;
	LiveSessionInfo zOmniCore::gLiveSessionInfo;
	bool zOmniCore::gOmniverseLoggingEnabled = false;
	mutex zOmniCore::gLogMutex;

	GfVec3d zOmniCore::gDefaultRotation(0);
	GfVec3i zOmniCore::gDefaultRotationOrder(0, 1, 2);
	GfVec3d zOmniCore::gDefaultScale(1);
		
	ZSPACE_INLINE void zOmniCore::shutdownOmniverse()
	{
		// Calling this prior to shutdown ensures that all pending live updates complete.
		omniClientLiveWaitForPendingUpdates();

		// The stage is a sophisticated object that needs to be destroyed properly.  
		// Since gStage is a smart pointer we can just reset it
		gStage.Reset();

		// This will prevent "Core::unregister callback called after shutdown"
		omniClientSetLogCallback(nullptr);

		omniClientShutdown();
	}

	ZSPACE_INLINE bool zOmniCore::startOmniverse()
	{
		// Register a function to be called whenever the library wants to print something to a log
		omniClientSetLogCallback(logCallback);

		// The default log level is "Info", set it to "Debug" to see all messages
		omniClientSetLogLevel(eOmniClientLogLevel_Debug);

		// Initialize the library and pass it the version constant defined in OmniClient.h
		// This allows the library to verify it was built with a compatible version. It will
		// return false if there is a version mismatch.
		if (!omniClientInitialize(kOmniClientVersion))
		{
			return false;
		}

		omniClientRegisterConnectionStatusCallback(nullptr, OmniClientConnectionStatusCallbackImpl);

		return true;
	}

	ZSPACE_INLINE bool zOmniCore::isValidOmniURL(const std::string& maybeURL)
	{
		bool isValidURL = false;
		OmniClientUrl* url = omniClientBreakUrl(maybeURL.c_str());
		if (url->host && url->path &&
			(std::string(url->scheme) == std::string("omniverse") ||
				std::string(url->scheme) == std::string("omni")))
		{
			isValidURL = true;
		}
		omniClientFreeUrl(url);

		if(!isValidURL)
		{
			std::cout << "This is not an Omniverse Nucleus URL: " << maybeURL << std::endl;
			std::cout << "Correct Omniverse URL format is: omniverse://server_name/Path/To/Example/Folder" << std::endl;
			std::cout << "Allowing program to continue because file paths may be provided in the form: C:/Path/To/Stage" << std::endl;
		}
		else std::cout << "Connected to Omniverse Nucleus URL: " << maybeURL << std::endl;

		return isValidURL;
	}

	ZSPACE_INLINE void zOmniCore::printConnectedUsername(const std::string& stageUrl)
	{
		// Get the username for the connection
		std::string userName("_none_");
		omniClientWait(omniClientGetServerInfo(stageUrl.c_str(), &userName, [](void* userData, OmniClientResult result, struct OmniClientServerInfo const* info) noexcept
		{
			std::string* userName = static_cast<std::string*>(userData);
			if (userData && userName && info && info->username)
			{
				userName->assign(info->username);
			}
		}));
		{
			std::unique_lock<std::mutex> lk(gLogMutex);
			std::cout << "Connected username: " << userName << std::endl;
		}
	}

	ZSPACE_INLINE string zOmniCore::createOmniverseModel(const std::string& destinationPath, string stageName, bool doLiveEdit)
	{
		std::string stageUrl = destinationPath + "/" + stageName + (doLiveEdit ? ".live" : ".usd");

		// Delete the old version of this file on Omniverse and wait for the operation to complete
		{
			std::unique_lock<std::mutex> lk(gLogMutex);
			std::cout << "Waiting for " << stageUrl << " to delete... " << std::endl;
		}
		omniClientWait(omniClientDelete(stageUrl.c_str(), nullptr, nullptr));
		{
			std::unique_lock<std::mutex> lk(gLogMutex);
			std::cout << "finished" << std::endl;
		}

		// Create this file in Omniverse cleanly
		gStage = UsdStage::CreateNew(stageUrl);
		if (!gStage)
		{
			failNotify("Failure to create model in Omniverse", stageUrl.c_str());
			return std::string();
		}

		{
			std::unique_lock<std::mutex> lk(gLogMutex);
			std::cout << "New stage created: " << stageUrl << std::endl;
		}

		// Keep the model contained inside of "World", only need to do this once per model
		const SdfPath defaultPrimPath = SdfPath::AbsoluteRootPath().AppendChild(_tokens->World);
		UsdGeomXform defaultPrim = UsdGeomXform::Define(gStage, defaultPrimPath);

		// Set the /World prim as the default prim
		gStage->SetDefaultPrim(defaultPrim.GetPrim());

		// Set the default prim as an assembly to support using component references
		pxr::UsdModelAPI(defaultPrim).SetKind(pxr::KindTokens->assembly);

		// Always a good idea to declare your up-ness
		UsdGeomSetStageUpAxis(gStage, UsdGeomTokens->y);

		// For physics its important to set units!
		UsdGeomSetStageMetersPerUnit(gStage, 0.01);

		return stageUrl;
	}


	ZSPACE_INLINE void zOmniCore::OmniClientConnectionStatusCallbackImpl(void* userData, const char* url, OmniClientConnectionStatus status) noexcept
	{
		// Let's just print this regardless
		{
			std::unique_lock<std::mutex> lk(gLogMutex);
			std::cout << "Connection Status: " << omniClientGetConnectionStatusString(status) << " [" << url << "]" << std::endl;
		}
		if (status == eOmniClientConnectionStatus_ConnectError)
		{
			// We shouldn't just exit here - we should clean up a bit, but we're going to do it anyway
			std::cout << "[ERROR] Failed connection, exiting." << std::endl;
			exit(-1);
		}
	
	}

	ZSPACE_INLINE void zOmniCore::logCallback(const char* threadName, const char* component, OmniClientLogLevel level, const char* message) noexcept
	{
		std::unique_lock<std::mutex> lk(gLogMutex);
		if (gOmniverseLoggingEnabled)
		{
			puts(message);
		}
	}

	ZSPACE_INLINE void zOmniCore::failNotify(const char* msg, const char* detail)
	{
		std::unique_lock<std::mutex> lk(gLogMutex);

		fprintf(stderr, "%s\n", msg);
		if (detail != nullptr)
		{
			fprintf(stderr, "%s\n", detail);
		}
	}

	ZSPACE_INLINE void zOmniCore::findOrCreateSession(UsdStageRefPtr rootStage, LiveSessionInfo& liveSessionInfo)
	{
		UsdStageRefPtr liveStage;
		std::string sessionFolderForStage = liveSessionInfo.GetSessionFolderPathForStage();
		std::vector<std::string> sessionList = liveSessionInfo.GetLiveSessionList();

		std::cout << "Select or create a Live Session:" << std::endl;
		for (int i = 0; i < sessionList.size(); i++)
		{
			std::cout << " [" << i << "] " << sessionList[i] << std::endl;
		}
		std::cout << " [n] Create a new session" << std::endl;
		std::cout << " [q] Quit" << std::endl;
		std::cout << "Select a live session to join: " << std::endl;
		char selection;
		std::cin >> selection;

		// If the user picked a session, find the root.live folder
		size_t selectionIdx = size_t(selection) - 0x30;
		if (std::isdigit(selection) && selectionIdx < sessionList.size())
		{
			std::string sessionName = sessionList[selectionIdx];
			liveSessionInfo.SetSessionName(sessionName.c_str());

			// Check that the config file version matches
			LiveSessionConfigFile sessionConfig;
			std::string tomlUrl = liveSessionInfo.GetLiveSessionTomlUrl();
			if (!sessionConfig.IsVersionCompatible(tomlUrl.c_str()))
			{
				std::string actualVersion = sessionConfig.GetSessionConfigValue(tomlUrl.c_str(), LiveSessionConfigFile::Key::Version);
				std::cout << "The session config TOML file version is not compatible, exiting." << std::endl;
				std::cout << "Expected: " << LiveSessionConfigFile::kCurrentVersion << " Actual: " << actualVersion << std::endl;
				exit(1);
			}

			std::string liveSessionUrl = liveSessionInfo.GetLiveSessionUrl();
			liveStage = UsdStage::Open(liveSessionUrl);
		}
		else if ('n' == selection)
		{
			// Get a new session name
			bool validSessionName = false;
			std::string sessionName;
			while (!validSessionName)
			{
				std::cout << "Enter the new session name: ";
				std::cin >> sessionName;

				// Session names must start with an alphabetical character, but may contain alphanumeric, hyphen, or underscore characters.
				if (liveSessionInfo.SetSessionName(sessionName.c_str()))
				{
					validSessionName = true;
				}
				else
				{
					std::cout << "Session names must start with an alphabetical character, but may contain alphanumeric, hyphen, or underscore characters." << std::endl;
				}
			}

			// Make sure that this session doesn't already exist (don't overwrite/stomp it)
			if (liveSessionInfo.DoesSessionExist())
			{
				std::cout << "Session config file already exists: " << liveSessionInfo.GetLiveSessionTomlUrl() << std::endl;
				exit(1);
			}

			// Create the session config file 
			LiveSessionConfigFile::KeyMap keyMap;
			std::string stageUrl = liveSessionInfo.GetStageUrl();
			std::string connectedUserName = getConnectedUsername(stageUrl.c_str());
			keyMap[LiveSessionConfigFile::Key::Admin] = connectedUserName.c_str();
			keyMap[LiveSessionConfigFile::Key::StageUrl] = stageUrl.c_str();
			keyMap[LiveSessionConfigFile::Key::Mode] = "default";
			LiveSessionConfigFile sessionConfig;
			if (!sessionConfig.CreateSessionConfigFile(liveSessionInfo.GetLiveSessionTomlUrl().c_str(), keyMap))
			{
				std::cout << "Unable to create session config file: " << liveSessionInfo.GetLiveSessionTomlUrl() << std::endl;
				exit(1);
			}

			// Create the new root.live file to be the stage's edit target
			std::string liveSessionUrl = liveSessionInfo.GetLiveSessionUrl();
			liveStage = UsdStage::CreateNew(liveSessionUrl);
		}
		else
		{
			std::cout << "Invalid selection, exiting";
			exit(1);
		}


		// Get the live layer from the live stage
		SdfLayerHandle liveLayer = liveStage->GetRootLayer();

		// Construct the layers so that we can join the session
		rootStage->GetSessionLayer()->InsertSubLayerPath(liveLayer->GetIdentifier());
		rootStage->SetEditTarget(UsdEditTarget(liveLayer));
	}

	ZSPACE_INLINE std::string zOmniCore::getConnectedUsername(const char* stageUrl)
	{
		// Get the username for the connection
		std::string userName("_none_");
		omniClientWait(omniClientGetServerInfo(stageUrl, &userName, [](void* userData, OmniClientResult result, struct OmniClientServerInfo const* info) noexcept
		{
			std::string* userName = static_cast<std::string*>(userData);
			if (userData && userName && info && info->username)
			{
				userName->assign(info->username);
			}
		}));

		return userName;
	}

	ZSPACE_INLINE bool zOmniCore::endAndMergeSession(UsdStageRefPtr rootStage, OmniChannel& channel, LiveSessionInfo& liveSessionInfo)
	{
		// Do we have authority (check TOML)?
	// Get the current owner name from the session TOML
		LiveSessionConfigFile sessionConfig;
		std::string sessionAdmin = sessionConfig.GetSessionAdmin(liveSessionInfo.GetLiveSessionTomlUrl().c_str());
		std::string currentUser = getConnectedUsername(liveSessionInfo.GetStageUrl().c_str());
		if (sessionAdmin != currentUser)
		{
			std::cout << "You [" << currentUser << "] are not the session admin [" << sessionAdmin << "].  Stopping merge.";
			return false;
		}

		// Gather the latest changes from the live stage
		omniClientLiveProcess();

		// Send a MERGE_STARTED channel message
		channel.SendChannelMessage(OmniChannelMessage::MessageType::MergeStarted);

		// Create a checkpoint on the live layer (don't force if no changes)
		// Create a checkpoint on the root layer (don't force if no changes)
		std::string comment("Pre-merge for " + liveSessionInfo.GetSessionName() + " session");
		checkpointFile(liveSessionInfo.GetLiveSessionUrl(), comment.c_str(), false);
		checkpointFile(liveSessionInfo.GetStageUrl(), comment.c_str(), false);

		// Check if there are any prims defined in the stage's root layer.
		// Merging to a new layer if this is the case could result in no visible changes due to the
		// root layer taking priority
		bool rootHasSpecs = primUtils::RootLayerHasPrimSpecsDefined(rootStage);

		std::string mergeOption("_");
		while (mergeOption != "n" && mergeOption != "r" && mergeOption != "c")
		{
			if (rootHasSpecs)
			{
				std::cout << "\nWARNING: There are PrimSpecs defined in the root layer.  Changes from the live session could be hidden if they are merged to a new layer." << std::endl;
			}
			std::cout << "Merge to new layer [n], root layer [r], or cancel [c]: ";
			std::cin >> mergeOption;
		}
		if (mergeOption == "n")
		{
			// Inject a new layer in the same folder as the root with the session name into the root stage (rootStageName_sessionName_edits.usd)
			std::string stageName = liveSessionInfo.GetStageFileName();
			std::string stageFolder = liveSessionInfo.GetStageFolderUrl();
			std::string sessionName = liveSessionInfo.GetSessionName();
			std::string newLayerUrl = stageFolder + "/" + stageName + "_" + sessionName + ".usd";
			std::cout << "Merging session changes to " << newLayerUrl << " and inserting as a sublayer in the root layer." << std::endl;
			primUtils::MergeLiveLayerToNewLayer(rootStage->GetEditTarget().GetLayer(), rootStage->GetRootLayer(), newLayerUrl.c_str());
		}
		else if (mergeOption == "r")
		{
			// Merge the live deltas to the root layer
			// This does not clear the source layer --- we'll do that after checkpointing it
			primUtils::MergeLiveLayerToRoot(rootStage->GetEditTarget().GetLayer(), rootStage->GetRootLayer());
		}
		else // the merge was canceled
		{
			return false;
		}

		// Create a checkpoint on the root layer while saving it
		std::string postComment("Post-merge for " + liveSessionInfo.GetSessionName() + " session");
		omniUsdResolverSetCheckpointMessage(postComment.c_str());
		rootStage->GetRootLayer()->Save();
		omniUsdResolverSetCheckpointMessage("");

		// Clear the live layer and process the live changes
		rootStage->GetEditTarget().GetLayer()->Clear();
		omniClientLiveProcess();

		// Remove the .live layer from the session layer
		rootStage->GetSessionLayer()->GetSubLayerPaths().clear();
		rootStage->SetEditTarget(UsdEditTarget(rootStage->GetRootLayer()));

		// Send a MERGE_FINISHED channel message
		channel.SendChannelMessage(OmniChannelMessage::MessageType::MergeFinished);

		return true;
	}

}

#endif