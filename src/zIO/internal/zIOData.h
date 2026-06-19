#ifndef ZSPACE_IO_DATA_H
#define ZSPACE_IO_DATA_H

#include <zspace/zCore/base/zTypeDef.h>

namespace zSpace::io_detail
{
	struct MeshData
	{
		zPointArray positions;
		zIntArray polygonCounts;
		zIntArray polygonConnects;
		zIntArray edgeConnects;
		zVectorArray faceNormals;
		zColorArray vertexColors;
		zColorArray edgeColors;
		zDoubleArray edgeWeights;
		zColorArray faceColors;
		std::vector<zDoubleArray> edgeAttributes;
	};

	struct GraphData
	{
		zPointArray positions;
		zIntArray edgeConnects;
		zColorArray vertexColors;
		zColorArray edgeColors;
	};

	struct PointCloudData
	{
		zPointArray positions;
		zColorArray colors;
	};
}

#endif
