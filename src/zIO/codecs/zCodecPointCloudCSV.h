#ifndef ZSPACE_CODEC_POINT_CLOUD_CSV_H
#define ZSPACE_CODEC_POINT_CLOUD_CSV_H

#include <src/zIO/internal/zIOData.h>
#include <zspace/zIO/zIOResult.h>

#include <string>

namespace zSpace::io_detail
{
	zIOResult readPointCloudCSV(const std::string& path, PointCloudData& data);
	zIOResult writePointCloudCSV(const std::string& path, const PointCloudData& data);
}

#endif
