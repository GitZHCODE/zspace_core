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

#ifdef ZSPACE_INLINE
#undef ZSPACE_INLINE
#endif

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
#  define ZSPACE_INLINE 
#else
#  define ZSPACE_INLINE inline
#endif

#if defined(ZSPACE_DYNAMIC_LIBRARY)
#if defined(_WIN32)
#if defined(ZSPACE_CORE_EXPORTS)
#define ZSPACE_CORE __declspec(dllexport)
#else
#define ZSPACE_CORE __declspec(dllimport)
#endif

#if defined(ZSPACE_INTERFACE_EXPORTS)
#define ZSPACE_API __declspec(dllexport)
#else
#define ZSPACE_API __declspec(dllimport)
#endif

#if defined(ZSPACE_DISPLAY_EXPORTS)
#define ZSPACE_DISPLAY __declspec(dllexport)
#else
#define ZSPACE_DISPLAY __declspec(dllimport)
#endif

#if defined(ZSPACE_INTEROP_EXPORTS)
#define ZSPACE_INTEROP __declspec(dllexport)
#else
#define ZSPACE_INTEROP __declspec(dllimport)
#endif

#if defined(ZSPACE_IO_EXPORTS)
#define ZSPACE_IO __declspec(dllexport)
#else
#define ZSPACE_IO __declspec(dllimport)
#endif

#define ZSPACE_TOOLS
#define ZSPACE_AG
#define ZSPACE_HS
#define ZSPACE_CF
#define ZSPACE_APP
#define ZSPACE_MAYA
#define ZSPACE_RHINO
#define ZSPACE_OMNI
#define ZSPACE_CUDA
#else
#define ZSPACE_CORE
#define ZSPACE_API
#define ZSPACE_DISPLAY
#define ZSPACE_INTEROP
#define ZSPACE_IO
#define ZSPACE_TOOLS
#define ZSPACE_AG
#define ZSPACE_HS
#define ZSPACE_CF
#define ZSPACE_APP
#define ZSPACE_MAYA
#define ZSPACE_RHINO
#define ZSPACE_OMNI
#define ZSPACE_CUDA
#endif
#else
#define ZSPACE_CORE 
#define ZSPACE_API 
#define ZSPACE_DISPLAY
#define ZSPACE_TOOLS
#define ZSPACE_AG 
#define ZSPACE_HS 
#define ZSPACE_CF 
#define ZSPACE_APP  
#define ZSPACE_INTEROP 
#define ZSPACE_IO
#define ZSPACE_MAYA  
#define ZSPACE_RHINO  
#define ZSPACE_OMNI 
#define ZSPACE_CUDA  
#endif

#ifndef __CUDACC__
#define ZSPACE_CUDA_CALLABLE 
#define ZSPACE_CUDA_CALLABLE_HOST 
#define ZSPACE_CUDA_CALLABLE_DEVICE 
#endif

