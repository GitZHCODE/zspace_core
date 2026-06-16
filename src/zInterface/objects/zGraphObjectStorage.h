#ifndef ZSPACE_GRAPH_OBJECT_STORAGE_H
#define ZSPACE_GRAPH_OBJECT_STORAGE_H

#pragma once

#include <src/zCore/geometry/detail/zGraphStorage.h>
#include <zspace/zInterface/objects/zObjectGraph.h>

namespace zSpace
{
	class zObjectGraph::Impl
	{
	public:
		zGraph graph;
	};

	class zGraphObjectStorage
	{
	public:
		static zGraph& get(zObjectGraph& object) { return object.impl->graph; }
		static const zGraph& get(const zObjectGraph& object) { return object.impl->graph; }
	};
}

#endif
