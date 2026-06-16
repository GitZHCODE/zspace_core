#ifndef ZSPACE_CODEC_JSON_H
#define ZSPACE_CODEC_JSON_H

#include <src/zIO/internal/zIOData.h>
#include <zspace/zIO/zIOResult.h>

#include <string>

namespace zSpace::io_detail
{
	zIOResult readMeshJSON(const std::string& path, MeshData& data);
	zIOResult writeMeshJSON(const std::string& path, const MeshData& data);

	zIOResult readGraphJSON(const std::string& path, GraphData& data);
	zIOResult writeGraphJSON(const std::string& path, const GraphData& data);
}

#endif
