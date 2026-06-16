#ifndef ZSPACE_CODEC_OBJ_H
#define ZSPACE_CODEC_OBJ_H

#include <src/zIO/internal/zIOData.h>
#include <zspace/zIO/zIOResult.h>

#include <string>

namespace zSpace::io_detail
{
	zIOResult readOBJ(const std::string& path, MeshData& data);
	zIOResult writeOBJ(const std::string& path, const MeshData& data);
}

#endif
