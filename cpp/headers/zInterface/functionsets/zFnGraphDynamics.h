// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//

#ifndef ZSPACE_FN_GRAPH_DYNAMICS_H
#define ZSPACE_FN_GRAPH_DYNAMICS_H

#pragma once

#include <headers/zInterface/functionsets/zFnParticle.h>
#include <headers/zInterface/functionsets/zFnGraph.h>

namespace zSpace
{
	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zFuntionSets
	*	\brief The function set classes of the library.
	*  @{
	*/

	/*! \class zFnGraphDynamics
	*	\brief A graph function set for dynamics.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zFnGraphDynamics : public zFnGraph
	{
	private:
		//--------------------------
		//---- PROTECTED ATTRIBUTES
		//--------------------------

		/*!	\brief container of particle function set  */
		//vector<zFnParticle> fnParticles;

		

	public:

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zFnGraphDynamics();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_graphObj			- input graph object.
		*	\since version 0.0.2
		*/
		zFnGraphDynamics(zObjGraph &_graphObj);

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zFnGraphDynamics();

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------
		
		zFnType getType() override;

		void from(std::string path, zFileTpye type, bool staticGeom = false);

		void to(std::string path, zFileTpye type);

		void getBounds(zPoint &minBB, zPoint &maxBB) override;

		void clear() override;

		//--------------------------
		//---- SET METHODS 
		//--------------------------

		/*! \brief This method fix the input vertices.
		*
		*	\param		[in]	vIDs			- container of vertices which are to be fixed.
		*	\since version 0.0.4
		*/
		void setFixed(zIntArray& vIDs);

		//--------------------------
		//---- CREATE METHODS
		//--------------------------

		/*! \brief This method creates the particles object from the graph object already attached to zFnGraphDynamics.
		*
		*	\param		[in]	fixBoundary			- true if the boundary vertices are to be fixed.
		*	\since version 0.0.2
		*/
		void makeDynamic(bool fixBoundary = false);

		/*! \brief This method creates a particle system from the input mesh object.
		*
		*	\param		[in]	_graphObj			- input graph object.
		*	\param		[in]	fixBoundary			- true if the boundary vertices are to be fixed.
		*	\since version 0.0.2
		*/
		void create(zObjGraph &_graphObj, bool fixBoundary = false);		

		//--------------------------
		//---- FORCE METHODS 
		//--------------------------

		/*! \brief This method adds the gravitational force to the input graph.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	gForce					- gravitational force vector.
		*	\since version 0.0.4
		*/
		void addGravityForce(double strength, zVector& gForce);

		/*! \brief This method adds the drag force to the input graph.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	drag					- drag constant.
		*	\since version 0.0.4
		*/
		void addDragForce(double strength, float drag);

		/*! \brief This method adds the drag force to the input graph.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	restLength				- input container of restlengths per edge.
		*	\since version 0.0.4
		*/
		void addSpringForce(double strength, zFloatArray& restLength);

		/*! \brief This method adds the drag force to snap two vertices to rest distance.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	v0						- input first vertex id.
		*	\param	[in]	v1						- input second vertex id.
		*	\param	[in]	restDistance			- input target distance.
		*	\since version 0.0.4
		*/
		void addDistanceForce(double strength, int v0, int v1, float& restDistance);

		/*! \brief This method adds the drag force to the input mesh.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	vId						- input vertex id.
		*	\param	[in]	vec						- input load direction.
		*	\since version 0.0.4
		*/
		void addLoadForce(double strength, int vId, zVector& vec);

		/*! \brief This method adds the drag force to the input edge.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	eId						- id of edge.
		*	\param	[in]	restLength				- input value of restlength on edge.
		*	\since version 0.0.4
		*/
		void addSpringForce(double strength, int eId, float restLength);

		/*! \brief This method adds the drag force to all edges.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	restLength				- input value of restlengths on edges.
		*	\param	[in]	eIds					- ids of edges.
		*	\since version 0.0.4
		*/
		void addSpringForces(double strength, float& restLengths, const zIntArray& eIds = zIntArray());

		/*! \brief This method adds the drag force to the input two edges to a given angle.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	vId						- id of vertex shared by two edges.
		*	\param	[in]	restAngle				- input value of restAngle between two edges.
		*	\param	[in]	moveHinge				- if true add force to the particle at hinge/ false add force to the end particles.
		*	\since version 0.0.4
		*/
		void addAngleForce(double strength, int vId, float restAngle, bool moveHinge = true);

		/*! \brief This method adds the drag force to the input two edges to a given angle.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	restAngle				- input value of restAngle between two edges.
		*	\param	[in]	moveHinge				- if true add force to the particle at hinge/ false add force to the end particles.
		*	\param	[in]	vIds					- id of vertices shared by two edges.
		*	\since version 0.0.4
		*/
		void addAngleForces(double strength, float& restAngle, bool& moveHinge, const zIntArray& vIds = zIntArray());

		/*! \brief This method adds the drag force to the input vertex to a given vector.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	vId						- id of vertex to drag.
		*	\param	[in]	origin					- input position of align vector start.
		*	\param	[in]	alignVector				- input align vector.
		*	\since version 0.0.4
		*/
		void addVectorForce(double strength, int vId, zPoint& origin, zVector& alignVector);

		//--------------------------
		//---- UPDATE METHODS 
		//--------------------------

		/*! \brief This method updates the position and velocity of all the  particle.
		*
		*	\param		[in]	dT					- timestep.
		*	\param		[in]	type				- integration type - zEuler or zRK4.
		*	\param		[in]	clearForce			- clears the force if true.
		*	\param		[in]	clearVelocity		- clears the velocity if true.
		*	\param		[in]	clearDerivatives	- clears the derivatives if true.
		*	\since version 0.0.2
		*/
		void update(double dT, zIntergrationType type = zEuler, bool clearForce = true, bool clearVelocity = false, bool clearDerivatives = false);

	private:

		//--------------------------
		//---- PRIVATE METHODS
		//--------------------------

		/*! \brief This method sets the edge vertex position conatiners for static graphs.
		*
		*	\since version 0.0.2
		*/
		void setStaticContainers();
				
	};

	

}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterface/functionsets/zFnGraphDynamics.cpp>
#endif

#endif
