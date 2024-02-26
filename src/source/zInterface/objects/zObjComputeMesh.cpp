// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2022 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Heba Eiz <heba.eiz@zaha-hadid.com> , Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//


#include "zInterface/objects/zObjComputeMesh.h"

namespace zSpace
{
	//---- CONSTRUCTOR
	ZSPACE_INLINE zObjComputeMesh::zObjComputeMesh() {}

	//---- DESTRUCTOR
	ZSPACE_INLINE zObjComputeMesh::~zObjComputeMesh() {

		//deleting all pointers 
		if (polygons && nF>0)
		{
			for (int i = 0; i < nF; i++)
			{
				delete polygons[i];
				polygons[i] = NULL;
			}
			delete polygons;
			polygons = NULL;
		}
		
		if (triangles && nF>0)
		{
			for (int i = 0; i < nF; i++)
			{
				delete triangles[i];
				triangles[i] = NULL;
			}
			delete triangles;
			triangles = NULL;
		}
		
		if (edges && nE >0)
		{
			for (int i = 0; i < nE; i++)
			{
				delete edges[i];
				edges[i] = NULL;
			}
			delete edges;
			edges = NULL;
		}

		if (cVertices && nV > 0)
		{
			for (int i = 0; i < nV; i++)
			{
				delete cVertices;
				cVertices[i] = NULL;
			}
			delete cVertices;
			cVertices = NULL;
		}

		if (polyCounts)
		{
			delete polyCounts;
			polyCounts = NULL;
		}

		if (triCounts)
		{
			delete triCounts;
			triCounts = NULL;
		}
		if (cVerticesCounts)
		{
			delete cVerticesCounts;
			cVerticesCounts = NULL;
		}
		if (vertexPositions)
		{
			delete vertexPositions;
			vertexPositions = NULL;
		}

		if (o_Particles)
		{
			delete o_Particles;
			o_Particles = NULL;
		}
		/*if(fnParticles)
		{
			delete fnParticles;
			fnParticles = NULL;
		}*/
		if (vFixed)
		{
			delete vFixed;
			vFixed = NULL;
		}
	}


	//---- OVERRIDE METHODS
	//ZSPACE_INLINE void zObjComputeMesh::draw() {}

	ZSPACE_INLINE void zObjComputeMesh::getBounds(zPoint& minBB, zPoint& maxBB) {}

}
