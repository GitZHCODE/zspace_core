// Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.
//
#pragma once

#ifdef __cplusplus
#    define OMNIUSDRESOLVER_EXPORT_C extern "C"
#elif !defined(OMNIUSDRESOLVER_EXPORTS)
#    define OMNIUSDRESOLVER_EXPORT_C extern
#else
#    define OMNIUSDRESOLVER_EXPORT_C
#endif

#if defined(_WIN32)
#    define OMNIUSDRESOLVER_ABI __cdecl
#else
#    define OMNIUSDRESOLVER_ABI
#endif

#ifdef OMNIUSDRESOLVER_EXPORTS
#    if defined(_WIN32)
#        define OMNIUSDRESOLVER_EXPORT_CPP __declspec(dllexport)
#    else
#        define OMNIUSDRESOLVER_EXPORT_CPP __attribute__((visibility("default")))
#    endif
#    define OMNIUSDRESOLVER_EXPORT(ReturnType)                                                                         \
        OMNIUSDRESOLVER_EXPORT_C OMNIUSDRESOLVER_EXPORT_CPP ReturnType OMNIUSDRESOLVER_ABI
#else
#    if defined(_WIN32)
#        define OMNIUSDRESOLVER_EXPORT_CPP __declspec(dllimport)
#    else
#        define OMNIUSDRESOLVER_EXPORT_CPP
#    endif
#    define OMNIUSDRESOLVER_EXPORT(ReturnType)                                                                         \
        OMNIUSDRESOLVER_EXPORT_C OMNIUSDRESOLVER_EXPORT_CPP ReturnType OMNIUSDRESOLVER_ABI
#endif

#ifdef __cplusplus
#    if __cplusplus >= 201703L
#        define OMNIUSDRESOLVER_NOEXCEPT noexcept
#        define OMNIUSDRESOLVER_CALLBACK_NOEXCEPT noexcept
#    elif __cplusplus >= 201103L || _MSC_VER >= 1900
#        define OMNIUSDRESOLVER_NOEXCEPT noexcept
#        define OMNIUSDRESOLVER_CALLBACK_NOEXCEPT
#    else
#        define OMNIUSDRESOLVER_NOEXCEPT throw()
#        define OMNIUSDRESOLVER_CALLBACK_NOEXCEPT
#    endif
#else
#    define OMNIUSDRESOLVER_NOEXCEPT
#    define OMNIUSDRESOLVER_CALLBACK_NOEXCEPT
#endif
