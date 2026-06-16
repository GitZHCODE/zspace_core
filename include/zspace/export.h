#ifndef ZSPACE_EXPORT_H
#define ZSPACE_EXPORT_H

#include <zspace/zCore/base/zInline.h>

#if defined(_MSC_VER)
#define ZSPACE_SUPPRESS_DLL_INTERFACE __pragma(warning(suppress : 4251))
#else
#define ZSPACE_SUPPRESS_DLL_INTERFACE
#endif

#endif
