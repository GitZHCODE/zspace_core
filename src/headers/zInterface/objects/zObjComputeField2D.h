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

#ifndef ZSPACE_OBJ_COMP_FIELD2D_H
#define ZSPACE_OBJ_COMP_FIELD2D_H

#pragma once

#include "zInterface/objects/zObj.h"

namespace zSpace
{
	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjComputeField2D
	*	\brief The mesh object class.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/
	
	class ZSPACE_API zObjComputeField2D : public zObj
	{
	protected:
		/*!	\brief container of vertex positions  */
		zPoint* vertexPositions;

		/*!	\brief container of polygons (quads)  */
		int* polygons;

		/*!	\brief container of edges  */
		int* edges;

		/*!	\brief number of vertices */
		int nV;

		/*!	\brief number of edges */
		int nE;

		/*!	\brief number of polygons */
		int nF;

		/*! \brief boolean for displaying the mesh vertices */
		bool displayVertices;

		/*! \brief boolean for displaying the mesh edges */
		bool displayEdges;

		/*! \brief boolean for displaying the mesh faces */
		bool displayFaces;

		/*! \brief boolean for displaying the mesh ids */
		bool displayVertexIds = false;
		bool displayEdgeIds = false;
		bool displayFaceIds = false;

		/*!	\brief stores the start vertex ID in the VBO, when attached to the zBufferObject.	*/
		int VBO_VertexId;

		/*!	\brief stores the start edge ID in the VBO, when attached to the zBufferObject.	*/
		int VBO_EdgeId;

		/*!	\brief stores the start face ID in the VBO, when attached to the zBufferObject.	*/
		int VBO_FaceId;

		/*!	\brief stores the start vertex color ID in the VBO, when attache to the zBufferObject.	*/
		int VBO_VertexColorId;
		
	public:

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------
		
		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjComputeField2D();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjComputeField2D();

		//--------------------------
		//---- SET METHODS
		//--------------------------

		/*! \brief This method sets display vertices, edges and face booleans.
		*
		*	\param		[in]	_displayVerts				- input display vertices booelan.
		*	\param		[in]	_displayEdges				- input display edges booelan.
		*	\param		[in]	_displayFaces				- input display faces booelan.
		*	\since version 0.0.4
		*/
		void setDisplayElements(bool _displayVerts, bool _displayEdges, bool _displayFaces);

		/*! \brief This method sets display vertexIds, edgeIds and faceIds booleans.
		*
		*	\param		[in]	_displayVertIds				- input display vertexIds booelan.
		*	\param		[in]	_displayEdgeIds				- input display edgeIds booelan.
		*	\param		[in]	_displayFaceIds				- input display faceIds booelan.
		*	\since version 0.0.4
		*/
		void setDisplayElementIds(bool _displayVertIds, bool _displayEdgeIds, bool _displayFaceIds);

		/*! \brief This method sets display vertices boolean.
		*
		*	\param		[in]	_displayVerts				- input display vertices booelan.
		*	\since version 0.0.4
		*/
		void setDisplayVertices(bool _displayVerts);

		/*! \brief This method sets display edges boolean.
		*
		*	\param		[in]	_displayEdges				- input display edges booelan.
		*	\since version 0.0.4
		*/
		void setDisplayEdges(bool _displayEdges);

		/*! \brief This method sets display faces boolean.
		*
		*	\param		[in]	_displayFaces				- input display faces booelan.
		*	\since version 0.0.4
		*/
		void setDisplayFaces(bool _displayFaces);

	
		//--------------------------
		//---- GET METHODS
		//--------------------------
		
		/*! \brief This method gets the vertex VBO Index .
		*
		*	\return			int				- vertex VBO Index.
		*	\since version 0.0.2
		*/
		int getVBO_VertexID();

		/*! \brief This method gets the edge VBO Index .
		*
		*	\return			int				- edge VBO Index.
		*	\since version 0.0.2
		*/
		int getVBO_EdgeID();

		/*! \brief This method gets the face VBO Index .
		*
		*	\return			int				- face VBO Index.
		*	\since version 0.0.2
		*/
		int getVBO_FaceID();

		/*! \brief This method gets the vertex color VBO Index .
		*
		*	\return			int				- vertex color VBO Index.
		*	\since version 0.0.2
		*/
		int getVBO_VertexColorID();		

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------
		
		void getBounds(zPoint &minBB, zPoint &maxBB) override;

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		void draw() override;
#endif

		//--------------------------
		//---- DISPLAY BUFFER METHODS
		//--------------------------

		/*! \brief This method appends mesh to the buffer.
		*
		*	\param		[in]	edge_dihedralAngles	- input container of edge dihedral angles.
		*	\param		[in]	DihedralEdges		- true if only edges above the dihedral angle threshold are stored.
		*	\param		[in]	angleThreshold		- angle threshold for the edge dihedral angles.
		*	\since version 0.0.1
		*/
		void appendToBuffer(zDoubleArray edge_dihedralAngles = zDoubleArray(), bool DihedralEdges = false, double angleThreshold = 45);

		protected:

		//--------------------------
		//---- PROTECTED DISPLAY METHODS
		//--------------------------

		/*! \brief This method displays the zMesh.
		*
		*	\since version 0.0.4
		*/
			void drawField2D();

	};
		

}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterface/objects/zObjComputeField2D.cpp>
#endif

#endif