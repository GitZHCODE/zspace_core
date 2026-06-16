#ifndef ZSPACE_CODEC_GRAPH_TXT_H
#define ZSPACE_CODEC_GRAPH_TXT_H

#include <src/zIO/internal/zIOData.h>
#include <zspace/zIO/zIOResult.h>

#include <string>

namespace zSpace::io_detail
{
	zIOResult readGraphTXT(const std::string& path, GraphData& data);
	zIOResult writeGraphTXT(const std::string& path, const GraphData& data);
}

#endif
