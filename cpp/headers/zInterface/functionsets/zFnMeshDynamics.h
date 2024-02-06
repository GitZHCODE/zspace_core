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

#ifndef ZSPACE_FN_MESH_DYNAMICS_H
#define ZSPACE_FN_MESH_DYNAMICS_H

#pragma once

#include <headers/zInterface/functionsets/zFnMesh.h>
#include <headers/zInterface/functionsets/zFnParticle.h>


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

	/*! \class zFnMeshDynamics
	*	\brief A mesh function set for dynamics.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zFnMeshDynamics : public zFnMesh
	{
	protected:
		//--------------------------
		//---- PROTECTED ATTRIBUTES
		//--------------------------
		///*!	\brief container of particle function set  */
		//vector<zFnParticle> fnParticles;	
			
		
	public:	

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zFnMeshDynamics();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_meshObj			- input mesh object.
		*	\since version 0.0.2
		*/
		zFnMeshDynamics(zObjMesh &_meshObj);

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zFnMeshDynamics();

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------
		
		zFnType getType() override;

		void from(string path, zFileTpye type);

		void to(string path, zFileTpye type);

		void getBounds(zPoint &minBB, zPoint &maxBB) override;

		void clear() override;
		
		//--------------------------
		//---- CREATE METHODS
		//--------------------------
				

		/*! \brief This method creates the particles object from the mesh object already attached to zFnMeshDynamics.
		*
		*	\param		[in]	fixBoundary			- true if the boundary vertices are to be fixed.
		*	\since version 0.0.2
		*/
		void makeDynamic(bool fixBoundary = false);

		/*! \brief This method creates a particle system from the input mesh object.
		*
		*	\param		[in]	_meshObj			- input mesh object.
		*	\param		[in]	fixBoundary			- true if the boundary vertices are to be fixed.
		*	\since version 0.0.2
		*/
		void create(zObjMesh &_meshObj, bool fixBoundary = false);


		//--------------------------
		//--- TOPOLOGY QUERY METHODS 
		//--------------------------

		/*! \brief This method returns the number of particles in the mesh.
		*	\return				int - number of particles.
		*	\since version 0.0.4
		*/
		int numParticles();

		//--------------------------
		//---- SET METHODS 
		//--------------------------

		/*! \brief This method fix the input vertices.
		*
		*	\param		[in]	vIDs			- container of vertices which are to be fixed.
		*	\since version 0.0.4
		*/
		void setFixed(zIntArray &vIDs);

		/*! \brief This method fix the boundary vertices.
		*
		*	\since version 0.0.4
		*/
		void setFixed_boundary();
		
		//--------------------------
		//---- FORCE METHODS 
		//--------------------------

		/*! \brief This method adds the gravitational force to the input mesh.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	gForce					- gravitational force vector.
		*	\since version 0.0.4
		*/
		void addGravityForce(double strength, zVector& gForce);

		/*! \brief This method adds the drag force to the input mesh.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	drag					- drag constant.
		*	\since version 0.0.4
		*/
		void addDragForce(double strength, float drag);

		/*! \brief This method adds the drag force to the input mesh.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	restLength				- input container of restlengths per edge.
		*  	\param	[out]	constrainType			- input constraints for the force directions.
		*	\since version 0.0.4
		*/
		void addSpringForce(double strength, zFloatArray& restLength, zSolverForceConstraints constrainType = zConstraintFree);

		/*! \brief This method adds the drag force to snap two vertices to rest distance.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	v0						- input first vertex id.
		*	\param	[in]	v1						- input second vertex id.
		*	\param	[in]	restDistance			- input target distance.
		*	\since version 0.0.4
		*/
		void addDistanceForce(double strength, int v0, int v1, float& restDistance);

		/*! \brief This method adds the smoothness force to the mesh.
		*	\details based on https://github.com/Dan-Piker/K2Goals/blob/master/TangentialSmooth.cs
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\since version 0.0.4
		*/
		void addSmoothnessForce(double strength);

		/*! \brief This method force the input vertices to a target plane.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	vIds					- input planarisation type - zQuadPlanar or zVolumePlanar.
		*  	\param	[out]	targetCenter			- input target plane center.
		*  	\param	[out]	targetNormal			- input target plane normal.
		*	\since version 0.0.4
		*/
		void addPlaneForce(double strength, zIntArray vIds, zPoint& targetCenter, zVector& targetNormal);

		/*! \brief This method adds the planarisation forces to the input mesh.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		* 	\param	[in]	tolerance				- input tolerance value below which the force isnt applied.
		*	\param	[in]	type					- input planarisation type - zQuadPlanar or zVolumePlanar.
		*  	\param	[out]	planarityDeviations		- output container of planarity deviations per face.
		*  	\param	[out]	forceDir				- output container of planarity force direction per vertex.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*  	\param	[out]	constrainType			- input constraints for the force directions.
		*	\since version 0.0.4
		*/
		void addPlanarityForce(double strength, double tolerance, zPlanarSolverType type, zDoubleArray& planarityDeviations, zVectorArray& forceDir, bool& exit, zSolverForceConstraints constrainType = zConstraintFree);

		/*! \brief This method adds the forces to the keep the input vertex groups to the target planes specified by target centers and normals.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		* 	\param	[in]	tolerance				- input tolerance value below which the force isnt applied.
		*	\param	[in]	type					- input planarisation type - zQuadPlanar or zVolumePlanar.
		*  	\param	[out]	planarityDeviations		- output container of planarity deviations per face.
		*  	\param	[out]	forceDir				- output container of planarity force direction per vertex.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*	\since version 0.0.4
		*/
		void addPlanarityForce_vertexgroups(double strength, double tolerance, vector<zIntArray> &vertexIDs, zPointArray& targetCenters, zVectorArray& targetNormals, zDoubleArray& planarityDeviations, zVectorArray& forceDir, bool& exit);

		/*! \brief This method adds the planarisation forces to the input mesh.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*  	\param	[in]	targetCenters			- container of target origin per face. Used only for zVolumePlanar planarisation type.
		*  	\param	[in]	targetNormals			- container of target normals per face. Used only for zVolumePlanar planarisation type.
		* 	\param	[in]	tolerance				- input tolerance value below which the force isnt applied.
		*  	\param	[out]	planarityDeviations		- output container of planarity deviations per face.
		*  	\param	[out]	forceDir				- output container of planarity force direction per vertex.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*  	\param	[out]	constrainType			- input constraints for the force directions.
		*	\since version 0.0.4
		*/
		void addPlanarityForce_targetPlane(double strength, double tolerance, zPointArray& targetCenters, zVectorArray& targetNormals,  zDoubleArray& planarityDeviations, zVectorArray& forceDir, bool& exit, zSolverForceConstraints constrainType = zConstraintFree);

		/*! \brief This method adds the developability forces to the input mesh.
		*	\details based on Desburn et.al(2002) http://www.geometry.caltech.edu/pubs/DMA02.pdf
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		* 	\param	[in]	tolerance				- input tolerance value belwo which the force isnt applied.
		*  	\param	[out]	planarityDeviations		- output container of planarity deviations per face.
		*  	\param	[out]	forceDir				- output container of planarity force direction per vertex.
		*  	\param	[out]	exit					- output boolean true if all the vertex gaussian deviations are below tolerance.
		*	\since version 0.0.4
		*/
		void addDevelopabilityForce(double strength, double tolerance, zDoubleArray& vGaussianCurvatures, zVectorArray& forceDir, bool& exit);


		/*! \brief This method adds the minimize area forces (minimal surface) to the input mesh.
		*	\details based on http://courses.cms.caltech.edu/cs177/hmw/Hmw2.pdf , https://github.com/Dan-Piker/K2Goals/blob/master/SoapFilm.cs
		*	\param	[in]	inMesh					- input compute mesh object.
		*	\param	[in]	type					- input planarisation type - zQuadPlanar or zVolumePlanar.
		* 	\param	[in]	strength				- input strength of the force.
		*	\since version 0.0.4
		*/
		void addMinimizeAreaForce(double strength);

		/*! \brief This method adds the forces to keep the input vertex pairs aligned to each other.
		*	\details based on http://courses.cms.caltech.edu/cs177/hmw/Hmw2.pdf , https://github.com/Dan-Piker/K2Goals/blob/master/SoapFilm.cs
		* 	\param	[in]	strength				- input strength of the force.
		*  	\param	[in]	vertexDistances			- input distance to be maintained between vertex pairs of the force.
		*	\param	[in]	vertexIDs				- input container of vertex pairs to be kept aligned.
		*  	\param	[out]	deviations				- output container of deviations per vertex.
		*  	\param	[out]	forceDir				- output container of planarity force direction per vertex.
		*  	\param	[out]	exit					- output boolean true if all the vertex gaussian deviations are below tolerance.
		*	\since version 0.0.4
		*/
		void addRigidLineForce(double strength, double tolerance, zIntPairArray& vertexIDs,zDoubleArray &vertexDistances, zDoubleArray& deviations, zVectorArray& forceDir, bool& exit);

		/*! \brief This method adds the drag force to the input edge.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	eId						- id of edge.
		*	\param	[in]	restLength				- input value of restlength on edge.
		*	\since version 0.0.4
		*/
		void addSpringForce(double strength, int eId, float restLength);


		/*! \brief This method adds the drag force to the input mesh.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	vId						- input vertex id.
		*	\param	[in]	vec						- input load direction.
		*	\since version 0.0.4
		*/
		void addLoadForce(double strength, int vId, zVector& vec);

		/*! \brief This method adds the drag force to the input vertex to a given vector.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	vId						- id of vertex to drag.
		*	\param	[in]	origin					- input position of align vector start.
		*	\param	[in]	alignVector				- input align vector.
		*	\since version 0.0.4
		*/
		void addVectorForce(double strength, int vId, zPoint& origin, zVector& alignVector);

		/*! \brief This method adds the drag force to the input two edges to a given angle.
		*
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		*	\param	[in]	vId						- id of vertex shared by two edges.
		*	\param	[in]	restAngle				- input value of restAngle between two edges.
		*	\param	[in]	moveHinge				- if true add force to the particle at hinge/ false add force to the end particles.
		*	\since version 0.0.4
		*/
		void addAngleForce(double strength, int vId, int vId_first, int vId_second, float restAngle, bool moveHinge = true);

		/*! \brief This method adds the forces to keep the input vertex pairs aligned to each other.
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		* 	\param	[in]	tolerance				- input tolerance value below which the force isnt applied.
		* 	\param	[in]	centerVertexId			- input panel center vertex which shared by four panel faces.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*	\NOT IMPLEMENT YET
		*/
		void addFittingForce_cone(double strength, double tolerance, int centerVertexId, bool& exit);

		/*! \brief This method adds the forces to keep the input vertex pairs aligned to each other.
		*	\param	[in]	strength				- input strength of the force. Typically between 0 and 1.
		* 	\param	[in]	tolerance				- input tolerance value below which the force isnt applied.
		* 	\param	[in]	centerVertexId			- input panel center vertex which shared by four panel faces.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*	\NOT IMPLEMENT YET
		*/
		void addFittingForce_cylinder(double strength, double tolerance, int centerVertexId, bool& exit);

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

	};
}



#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterface/functionsets/zFnMeshDynamics.cpp>
#endif

#endif