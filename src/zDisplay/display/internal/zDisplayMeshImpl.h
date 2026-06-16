#ifndef ZSPACE_DISPLAY_MESH_IMPL_H
#define ZSPACE_DISPLAY_MESH_IMPL_H

#include <zspace/zCore/base/zVector.h>

#include <vector>

namespace zSpace
{
	class zDisplayMesh::Impl
	{
	public:
		bool objectVisible = true;
		bool transformVisible = false;
		bool verticesVisible = false;
		bool edgesVisible = true;
		bool facesVisible = true;
		bool vertexIdsVisible = false;
		bool edgeIdsVisible = false;
		bool faceIdsVisible = false;
		bool dihedralEdgesVisible = false;
		bool vertexNormalsVisible = false;
		bool faceNormalsVisible = false;
		double dihedralAngleThreshold = 45.0;
		double normalScale = 1.0;
		std::vector<zVector> faceCenters;
		std::vector<zVector> edgeCenters;
		std::vector<double> dihedralAngles;
	};
}

#endif
