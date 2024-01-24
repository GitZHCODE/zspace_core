// Copyright (c) 2022-2023, NVIDIA CORPORATION. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto. Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.
//
// clang-format off
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "Defines.h"

/**
 * Sets the message that will be used by all atomic checkpoints created when saving USD files in Nucleus. By default
 * the message will be an empty string (""). The provided message will apply all to all 'Save' and 'Export' calls made
 * after the call to this function.
 * 
 * @param message Checkpoint message.
 */
OMNIUSDRESOLVER_EXPORT(void)
    omniUsdResolverSetCheckpointMessage(const char* message)
    OMNIUSDRESOLVER_NOEXCEPT;

/**
 * Return a human readable string of the version of this USD resolver
 */
OMNIUSDRESOLVER_EXPORT(const char*)
    omniUsdResolverGetVersionString()
    OMNIUSDRESOLVER_NOEXCEPT;

enum OmniUsdResolverEvent
{
    eOmniUsdResolverEvent_Resolving,
    eOmniUsdResolverEvent_Reading,
    eOmniUsdResolverEvent_Writing,

    Count_eOmniUsdResolverEvent
};

enum OmniUsdResolverEventState
{
    /// Started always comes first
    eOmniUsdResolverEventState_Started,

    /// The operation completed successfully
    eOmniUsdResolverEventState_Success,

    /// An error occured while performing the operation
    eOmniUsdResolverEventState_Failure,

    Count_eOmniUsdResolverEventState
};

/**
* Called when the resolver is operating on a file.
*/
typedef void(OMNIUSDRESOLVER_ABI* OmniUsdResolverEventCallback)
    (void * userData, const char* identifier, OmniUsdResolverEvent eventType, OmniUsdResolverEventState eventState, uint64_t fileSize)
    OMNIUSDRESOLVER_CALLBACK_NOEXCEPT;

/**
* Register a function that will be called any time something interesting happens
*
* Returns a handle that can be passed to omniUsdResolverUnregisterCallback
*/
OMNIUSDRESOLVER_EXPORT(uint32_t)
    omniUsdResolverRegisterEventCallback(void * userData, OmniUsdResolverEventCallback callback)
    OMNIUSDRESOLVER_NOEXCEPT;

/**
* Unregister a previously registered callback
*/
OMNIUSDRESOLVER_EXPORT(void)
    omniUsdResolverUnregisterCallback(uint32_t handle)
    OMNIUSDRESOLVER_NOEXCEPT;

OMNIUSDRESOLVER_EXPORT(void) omniUsdResolverSetCheckpointMessage(const char* message) OMNIUSDRESOLVER_NOEXCEPT;

/**
* Set a list of built-in MDLs.
* 
* Resolving an identifier in this list will return immediately rather than performing a full resolution.
*/
OMNIUSDRESOLVER_EXPORT(void)
    omniUsdResolverSetMdlBuiltins(char const** builtins, size_t numBuiltins)
    OMNIUSDRESOLVER_NOEXCEPT;

