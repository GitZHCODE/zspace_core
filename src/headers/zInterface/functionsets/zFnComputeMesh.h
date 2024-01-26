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

#ifndef ZSPACE_FN_COMPUTEMESH_H
#define ZSPACE_FN_COMPUTEMESH_H


#pragma once
#include <vector>
#include <string>
using namespace std;

#include "zInterface/functionsets/zFn.h"
#include "zInterface/functionsets/zFnParticle.h"

#include "zInterface/objects/zObjComputeMesh.h"

namespace  zSpace
{

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zFuntionSets
	*	\brief The function set classes of the library.
	*  @{
	*/

	/*! \class zFnComputeMesh
	*	\brief The mesh compute funstion set class.
	*	\since version 0.0.2
	*/

	/** @}*/
	class ZSPACE_API zFnComputeMesh : public zFn
	{
	protected:

		/*! \brief Pointer to a zObjComputeMesh object*/
		zObjComputeMesh* compMeshObj;

	public:
		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zFnComputeMesh();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_meshObj			- input mesh object.
		*	\since version 0.0.2
		*/
		zFnComputeMesh(zObjComputeMesh& _compMeshObj);

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zFnComputeMesh();

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------

		zFnType getType() override;

		void from(string path, zFileTpye type, bool staticGeom = false) override;

		void to(string path, zFileTpye type) override;

		void getBounds(zPoint& minBB, zPoint& maxBB) override;
		
		void clear() override;		
		

		/*! \brief This method creates a mesh from the input containers.
		*
		*	\param	[in]	_vertexPositions		- input container of vertex positions. Collapsed 1D array of size numVerts * 3.
		*	\param	[in]	_polyCounts				- input container of number of vertices per polygon of the mesh.
		*	\param	[in]	_polyConnects			- input container of polygon connectivity. Collapsed 1D array of size numFaces * (numVerts per face).
		* 	\param	[in]	numVerts				- input number of vertices in the mesh.
		*  	\param	[in]	numFaces				- input number of faces/polygons in the mesh.
		*	\since version 0.0.2
		*/
		bool create(double* _vertexPositions, int* _polyCounts, int* _polyConnects, int* _triCounts, int* _triConnects, int numVerts, int numFaces);
			
		MatrixXd setMatrixV();

		MatrixXi setMatrixFTris();

		/*! \brief This method set the connected vertices of each each vertex.
		*
		*  	\param	[out]	vertexIndices			- Vertices connected to each vertex collapsed as 1D array.
		*  	\param	[out]	cvCounts				- Count of connected vertices per vertex.
		*	\since version 0.0.4
		*/
		void setCVertices(int* _vertexIndices, int* _cvCounts);

		void computeFaceVolumes(zPointArray& fCenters, zDoubleArray& fVolumes);

		void computeFaceNormals(zVectorArray& fNormals);

		void computeQuadPlanarityDeviation(zDoubleArray& fDeviations);

		void computeBounds(zDomainVector& bounds);

	

		//--------------------------
		//---- TRANSFORM METHODS OVERRIDES
		//--------------------------

		void setTransform(zTransform& inTransform, bool decompose = true, bool updatePositions = true) override;

		void setScale(zFloat4& scale) override;

		void setRotation(zFloat4& rotation, bool appendRotations = false) override;

		void setTranslation(zVector& translation, bool appendTranslations = false) override;

		void setPivot(zVector& pivot) override;

		void getTransform(zTransform& transform) override;

	protected:

		//--------------------------
		//---- TRANSFORM  METHODS
		//--------------------------
		void transformObject(zTransform& transform) override;

		void setPolygon(int* _polyConnects);

		void setEdges();

		void setParticles();

		void setTriangles(int* _triConnects);

	};
}


namespace zSpace
{
	extern "C" {
		void extCreateComputeMesh(double* _vertexPositions, int* _polyCounts, int* _polyConnects, int* _triCounts, int* _triConnects, int numVerts, int numFaces, zObjComputeMesh* outComputeMesh);

		void extDeleteCreateComputeMesh(zObjComputeMesh* computeMesh);
	}

	
}









#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterface/functionsets/zFnComputeMesh.cpp>
#endif

#endif