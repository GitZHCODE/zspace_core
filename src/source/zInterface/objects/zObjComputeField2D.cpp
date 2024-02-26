// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>, Leo Bieling <leo.bieling@zaha-hadid.com>
//


#include "zInterface/objects/zObjComputeField2D.h"

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjComputeField2D::zObjComputeField2D()
	{

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		//displayUtils = nullptr;
#endif

		displayVertices = false;
		displayEdges = true;
		displayFaces = true;

		/*displayDihedralEdges = false;
		displayVertexNormals = false;
		displayFaceNormals = false;

		dihedralAngleThreshold = 45;

		normalScale = 1.0;*/
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjComputeField2D::~zObjComputeField2D() {}

	//---- SET METHODS

	ZSPACE_INLINE void zObjComputeField2D::setDisplayElements(bool _displayVerts, bool _displayEdges, bool _displayFaces)
	{
		displayVertices = _displayVerts;
		displayEdges = _displayEdges;
		displayFaces = _displayFaces;
	}

	ZSPACE_INLINE void zObjComputeField2D::setDisplayElementIds(bool _displayVertIds, bool _displayEdgeIds, bool _displayFaceIds)
	{
		displayVertexIds = _displayVertIds;
		displayEdgeIds = _displayEdgeIds;
		displayFaceIds = _displayFaceIds;
	}

	ZSPACE_INLINE void zObjComputeField2D::setDisplayVertices(bool _displayVerts)
	{
		displayVertices = _displayVerts;
	}

	ZSPACE_INLINE void zObjComputeField2D::setDisplayEdges(bool _displayEdges)
	{
		displayEdges = _displayEdges;
	}

	ZSPACE_INLINE void zObjComputeField2D::setDisplayFaces(bool _displayFaces)
	{
		displayFaces = _displayFaces;
	}


	//---- GET METHODS

	ZSPACE_INLINE int zObjComputeField2D::getVBO_VertexID()
	{
		return VBO_VertexId;
	}

	ZSPACE_INLINE int zObjComputeField2D::getVBO_EdgeID()
	{
		return VBO_EdgeId;
	}

	ZSPACE_INLINE int zObjComputeField2D::getVBO_FaceID()
	{
		return VBO_FaceId;
	}

	ZSPACE_INLINE int zObjComputeField2D::getVBO_VertexColorID()
	{
		return VBO_VertexColorId;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zObjComputeField2D::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		coreUtils.getBounds(vertexPositions, nV, minBB, maxBB);
	}

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) || defined (ZSPACE_RHINO_INTEROP)
	// Do Nothing
#else
	   
	//ZSPACE_INLINE void zObjComputeField2D::draw()
	//{	
	//	if (displayObject)
	//	{
	//		drawField2D();			
	//	}

	//	if (displayObjectTransform)
	//	{
	//		displayUtils->drawTransform(transformationMatrix);
	//	}

	//}

	//---- DISPLAY BUFFER METHODS

	ZSPACE_INLINE void zObjComputeField2D::appendToBuffer(zDoubleArray edge_dihedralAngles, bool DihedralEdges, double angleThreshold)
	{
		displayObject = displayEdges = displayVertices = displayFaces = false;

		// Edge Indicies
		

		// Face Indicies

		
		// Vertex Attributes

	

	}

	//---- PROTECTED DISPLAY METHODS

	ZSPACE_INLINE void zObjComputeField2D::drawField2D()
	{

		// draw vertex
		if (displayVertices)
		{
			//displayUtils->drawVertices(mesh.vHandles, &mesh.vertexPositions[0], &mesh.vertexColors[0], &mesh.vertexWeights[0]);			
		}

		// draw vertex ID
		if (displayVertexIds)
		{
			zColor col(0.8, 0, 0, 1);
			//displayUtils->drawVertexIds(mesh.n_v, &mesh.vertexPositions[0], col);
		}

		// draw edges
		if (displayEdges)
		{
			/*if (mesh.staticGeometry)
			{
				displayUtils->drawEdges(mesh.eHandles, mesh.edgeVertices, &mesh.vertexPositions[0], &mesh.edgeColors[0], &mesh.edgeWeights[0]);
			}

			else
			{
				vector<zIntArray> edgeVertices;
				edgeVertices.assign(mesh.edges.size(), zIntArray(2) = { -1,-1 });

				for (auto &e : mesh.edges)
				{

					if (mesh.eHandles[e.getId()].id != -1)
					{
						zIntArray eVerts;

						edgeVertices[e.getId()][0] = e.getHalfEdge(0)->getVertex()->getId();
						edgeVertices[e.getId()][1] = e.getHalfEdge(1)->getVertex()->getId();
					}
				}

				displayUtils->drawEdges(mesh.eHandles, edgeVertices, &mesh.vertexPositions[0], &mesh.edgeColors[0], &mesh.edgeWeights[0]);
			}*/
		}

		// draw edges ID
		if (displayEdgeIds)
		{
			/*if (edgeCenters.size() != mesh.n_e) throw std::invalid_argument(" error: edge centers are not computed.");

			zColor col(0, 0.8, 0, 1);
			displayUtils->drawEdgeIds(mesh.n_e, &edgeCenters[0], col);*/
		}

		// draw polygon
		if (displayFaces)
		{
			/*if (mesh.staticGeometry)
			{
				displayUtils->drawFaces(mesh.fHandles, mesh.faceVertices, &mesh.vertexPositions[0], &mesh.faceColors[0]);
			}
			else
			{
				vector<zIntArray> faceVertices;

				for (int i = 0; i < mesh.n_f; i++)
				{
					zIntArray faceVerts;
					if (mesh.fHandles[i].id != -1)
					{
						mesh.getFaceVertices(i, faceVerts);
					}

					faceVertices.push_back(faceVerts);

				}

				displayUtils->drawFaces(mesh.fHandles, faceVertices, &mesh.vertexPositions[0], &mesh.faceColors[0]);

			}*/
		}

		// draw polygon ID
		if (displayFaceIds)
		{
			/*if (faceCenters.size() != mesh.n_f) throw std::invalid_argument(" error: face centers are not computed.");

			zColor col(0, 0, 0.8, 1);
			displayUtils->drawFaceIds(mesh.n_f, &faceCenters[0], col);*/
		}
	}


#endif // !ZSPACE_UNREAL_INTEROP

}