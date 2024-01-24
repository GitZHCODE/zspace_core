// Copyright (c) 2020-2022, NVIDIA CORPORATION. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto. Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.
//
#pragma once

#ifdef __cplusplus
#    define OMNICLIENT_EXPORT_C extern "C"
#elif !defined(OMNICLIENT_EXPORTS)
#    define OMNICLIENT_EXPORT_C extern
#else
#    define OMNICLIENT_EXPORT_C
#endif

#if defined(_WIN32)
#    define OMNICLIENT_ABI __cdecl
#else
#    define OMNICLIENT_ABI
#endif

#ifdef OMNICLIENT_EXPORTS
#    if defined(_WIN32)
#        define OMNICLIENT_EXPORT_CPP __declspec(dllexport)
#    elif defined(__linux__) || defined(__APPLE__)
#        define OMNICLIENT_EXPORT_CPP __attribute__((visibility("default")))
#    else
static_assert(false, "unsupported platform!");
#    endif
#    define OMNICLIENT_EXPORT(ReturnType) OMNICLIENT_EXPORT_C OMNICLIENT_EXPORT_CPP ReturnType OMNICLIENT_ABI
#else
#    if defined(_WIN32)
#        define OMNICLIENT_EXPORT_CPP __declspec(dllimport)
#    else
#        define OMNICLIENT_EXPORT_CPP
#    endif
#    define OMNICLIENT_EXPORT(ReturnType) OMNICLIENT_EXPORT_C OMNICLIENT_EXPORT_CPP ReturnType OMNICLIENT_ABI
#endif

#ifdef __cplusplus
#    define OMNICLIENT_DEFAULT(Val) = (Val)
#    if __cplusplus >= 201703L
#        define OMNICLIENT_NOEXCEPT noexcept
#        define OMNICLIENT_CALLBACK_NOEXCEPT noexcept
#    elif __cplusplus >= 201103L || _MSC_VER >= 1900
#        define OMNICLIENT_NOEXCEPT noexcept
#        define OMNICLIENT_CALLBACK_NOEXCEPT
#    else
#        define OMNICLIENT_NOEXCEPT throw()
#        define OMNICLIENT_CALLBACK_NOEXCEPT
#    endif
#else
#    define OMNICLIENT_DEFAULT(Val)
#    define OMNICLIENT_NOEXCEPT
#    define OMNICLIENT_CALLBACK_NOEXCEPT
#endif

#if !defined(OMNICLIENT_EXPORTS) && defined(__cplusplus) && __cplusplus >= 201402L
#    define OMNICLIENT_DEPRECATED(x) [[deprecated(x)]]
#else
#    define OMNICLIENT_DEPRECATED(x)
#endif
