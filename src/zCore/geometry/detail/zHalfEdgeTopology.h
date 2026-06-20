// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms,
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE
//
// This Source Code Form is subject to the terms of the MIT License.

#ifndef ZSPACE_DETAIL_HALF_EDGE_TOPOLOGY_H
#define ZSPACE_DETAIL_HALF_EDGE_TOPOLOGY_H

#pragma once

#include <zspace/zCore/base/zVector.h>
#include <zspace/zCore/base/zColor.h>
#include <zspace/zCore/base/zTypeDef.h>
#include <zspace/zCore/utilities/zUtilsCore.h>
#include <zspace/zCore/geometry/zHEGeomTypes.h>

namespace zSpace
{
	namespace detail
	{
		/*! \class zHalfEdgeTopology
		*	\brief Internal shared half-edge storage used by graph and mesh geometry.
		*
		*	This type is not part of the public object API. Client code should access geometry
		*	through zObjectMesh/zFnMesh, zObjectGraph/zFnGraph, and their iterators.
		*/
		class ZSPACE_CORE zHalfEdgeTopology
		{
		public:
			zUtilsCore coreUtils;

			int n_v;
			int n_e;
			int n_he;

			zVertexArray vertices;
			zHalfEdgeArray halfEdges;
			zEdgeArray edges;

			zPointArray vertexPositions;
			unordered_map<string, int> existingHalfEdges;
			unordered_map<string, int> positionVertex;

			zColorArray vertexColors;
			zColorArray edgeColors;
			zDoubleArray vertexWeights;
			zDoubleArray edgeWeights;

			vector<zVertexHandle> vHandles;
			vector<zEdgeHandle> eHandles;
			vector<zHalfEdgeHandle> heHandles;

			bool staticGeometry = false;
			vector<zIntArray> edgeVertices;

			zHalfEdgeTopology();
			~zHalfEdgeTopology();

			void setNumVertices(int _n_v, bool setMax = true);
			void setNumEdges(int _n_e, bool setMax = true);

			bool vertexExists(zPoint pos, int& outVertexId, int precisionfactor = 6);
			void addToPositionMap(zPoint& pos, int index, int precisionfactor = 6);
			void removeFromPositionMap(zPoint& pos, int precisionfactor = 6);
			void addToHalfEdgesMap(int v1, int v2, int index);
			void removeFromHalfEdgesMap(int v1, int v2);
			bool halfEdgeExists(int v1, int v2, int& outEdgeId);
		};
	}
}

#if defined(ZSPACE_STATIC_LIBRARY) || defined(ZSPACE_DYNAMIC_LIBRARY)
#else
#include <src/zCore/geometry/zHalfEdgeTopology.cpp>
#endif

#endif
