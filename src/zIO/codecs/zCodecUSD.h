#ifndef ZSPACE_CODEC_USD_H
#define ZSPACE_CODEC_USD_H

#include <zspace/zIO/zIOResult.h>

#include <src/zIO/internal/zIOData.h>

#include <string>

namespace zSpace::io_detail
{
	zIOResult readMeshUSD(const std::string& path, MeshData& data);
	zIOResult writeMeshUSD(const std::string& path, const MeshData& data);
}

#endif
