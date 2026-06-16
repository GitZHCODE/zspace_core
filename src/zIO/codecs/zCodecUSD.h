#ifndef ZSPACE_CODEC_USD_H
#define ZSPACE_CODEC_USD_H

#include <zspace/zIO/zIOResult.h>

namespace zSpace::io_detail
{
	inline zIOResult usdUnavailable()
	{
		return zIOResult::error("USD support is not enabled. Load or link the optional zSpace_IO_USD module.");
	}
}

#endif
