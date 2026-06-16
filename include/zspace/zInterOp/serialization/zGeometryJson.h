// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms,
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE
//
// This Source Code Form is subject to the terms of the MIT License.

#ifndef ZSPACE_GEOMETRY_JSON_H
#define ZSPACE_GEOMETRY_JSON_H

#pragma once

#include <zspace/zCore/base/zTypeDef.h>

#include <vector>

namespace zSpace
{
	/*! \struct zHalfEdgeJsonData
	*	\brief Transfer data used by graph and mesh JSON serializers.
	*/
	struct zHalfEdgeJsonData
	{
		std::vector<int> vertices;
		std::vector<zIntArray> halfedges;
		std::vector<int> faces;
		std::vector<zDoubleArray> vertexAttributes;
		std::vector<zDoubleArray> halfedgeAttributes;
		std::vector<zDoubleArray> faceAttributes;
		zDoubleArray edgeCreaseData;
		zDoubleArray vertexCreaseData;
		zDoubleArray pleatData;
	};

	// Transitional alias for source compatibility with existing integrations.
	using zUtilsJsonHE = zHalfEdgeJsonData;
}

#endif
