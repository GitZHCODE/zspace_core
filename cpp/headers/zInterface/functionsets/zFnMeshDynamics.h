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
		/*!	\brief container of particle function set  */
		vector<zFnParticle> fnParticles;	

		/*!	\brief container of  particle objects  */
		vector<zObjParticle> particlesObj;
		
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
		//---- FORCE METHODS 
		//--------------------------

		/*! \brief This method adds the gravitational force to the input mesh.
		*
		*	\param	[in]	inMesh					- input compute mesh object.
		*	\param	[in]	gForce					- gravitational force vector.
		*	\since version 0.0.4
		*/
		void addGravityForce( zVector& gForce);

		/*! \brief This method adds the drag force to the input mesh.
		*
		*	\param	[in]	inMesh					- input compute mesh object.
		*	\param	[in]	drag					- drag constant.
		*	\since version 0.0.4
		*/
		void addDragForce(float drag);

		/*! \brief This method adds the drag force to the input mesh.
		*
		*	\param	[in]	inMesh					- input compute mesh object.
		*	\param	[in]	restLength				- input container of restlengths per edge.
		*	\param	[in]	springConstant			- input spring constant.
		*	\since version 0.0.4
		*/
		void addSpringForce(zFloatArray& restLength, double strength = 1);

		/*! \brief This method adds the smoothness force to the mesh.
		*	\details based on https://github.com/Dan-Piker/K2Goals/blob/master/TangentialSmooth.cs
		*	\param	[in]	strength			- input strength of the force.
		*	\since version 0.0.4
		*/
		void addSmoothnessForce(double strength = 1);

		/*! \brief This method adds the planarisation forces to the input mesh.
		*
		*	\param	[in]	type					- input planarisation type - zQuadPlanar or zVolumePlanar.
		* 	\param	[in]	tolerance				- input tolerance value below which the force isnt applied.
		*  	\param	[out]	planarityDeviations		- output container of planarity deviations per face.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*	\since version 0.0.4
		*/
		void addPlanarityForce( zPlanarSolverType type, double& tolerance, zDoubleArray& planarityDeviations, bool& exit);

		/*! \brief This method adds the planarisation forces to the input mesh.
		*
		*	\param	[in]	type					- input planarisation type - zQuadPlanar or zVolumePlanar.
		*  	\param	[in]	targetCenters			- container of target origin per face. Used only for zVolumePlanar planarisation type.
		*  	\param	[in]	targetNormals			- container of target normals per face. Used only for zVolumePlanar planarisation type.
		* 	\param	[in]	tolerance				- input tolerance value below which the force isnt applied.
		*  	\param	[out]	planarityDeviations		- output container of planarity deviations per face.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*	\since version 0.0.4
		*/
		void addPlanarityForce_targetPlane(zPlanarSolverType type, zPointArray& targetCenters, zVectorArray& targetNormals, double& tolerance, zDoubleArray& planarityDeviations, bool& exit);

		/*! \brief This method adds the planarisation forces to the input mesh.
		*	\details based on Desburn et.al(2002) http://www.geometry.caltech.edu/pubs/DMA02.pdf
		*	\param	[in]	inMesh					- input compute mesh object.
		*	\param	[in]	type					- input planarisation type - zQuadPlanar or zVolumePlanar.
		* 	\param	[in]	tolerance				- input tolerance value belwo which the force isnt applied.
		*  	\param	[out]	exit					- output boolean true if all the planarity deviations are below tolerance.
		*  	\param	[out]	planarityDeviations		- output container of planarity deviations per face.
		*  	\param	[in]	targetCenters			- container of target origin per face. Used only for zVolumePlanar planarisation type.
		*  	\param	[in]	targetNormals			- container of target normals per face. Used only for zVolumePlanar planarisation type.
		*	\since version 0.0.4
		*/
		void addGaussianForce( double& tolerance, zDoubleArray& vGaussianCurvatures, bool& exit);


		/*! \brief This method adds the minimize area forces (minimal surface) to the input mesh.
		*	\details based on http://courses.cms.caltech.edu/cs177/hmw/Hmw2.pdf , https://github.com/Dan-Piker/K2Goals/blob/master/SoapFilm.cs
		*	\param	[in]	inMesh					- input compute mesh object.
		*	\param	[in]	type					- input planarisation type - zQuadPlanar or zVolumePlanar.
		* 	\param	[in]	strength				- input strength of the force.
		*	\since version 0.0.4
		*/
		void addMinimizeAreaForce(double strength = 1);

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