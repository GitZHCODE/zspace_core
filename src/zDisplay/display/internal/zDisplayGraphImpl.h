#ifndef ZSPACE_DISPLAY_GRAPH_IMPL_H
#define ZSPACE_DISPLAY_GRAPH_IMPL_H

#include <zspace/zCore/base/zVector.h>

#include <vector>

namespace zSpace
{
	class zDisplayGraph::Impl
	{
	public:
		bool objectVisible = true;
		bool transformVisible = false;
		bool verticesVisible = false;
		bool edgesVisible = true;
		bool vertexIdsVisible = false;
		bool edgeIdsVisible = false;
		std::vector<zVector> edgeCenters;
	};
}

#endif
