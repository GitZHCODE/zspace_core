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


#include<headers/zInterface/functionsets/zFnMeshDynamics.h>

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnMeshDynamics::zFnMeshDynamics()
	{
		fnType = zFnType::zMeshDynamicsFn;
		meshObj = nullptr;
	}

	ZSPACE_INLINE zFnMeshDynamics::zFnMeshDynamics(zObjMesh &_meshObj)
	{
		fnType = zFnType::zMeshDynamicsFn;
		meshObj = &_meshObj;


	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnMeshDynamics::~zFnMeshDynamics() {}

	//---- OVERRIDE METHODS
	
	ZSPACE_INLINE zFnType zFnMeshDynamics::getType()
	{
		return zFnType::zMeshDynamicsFn;
	}

	ZSPACE_INLINE void zFnMeshDynamics::from(string path, zFileTpye type)
	{
		if (type == zOBJ) fromOBJ(path);
		else if (type == zJSON) fromJSON(path);

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnMeshDynamics::to(string path, zFileTpye type)
	{
		if (type == zOBJ) toOBJ(path);
		else if (type == zJSON) toJSON(path);

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnMeshDynamics::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		meshObj->getBounds(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnMeshDynamics::clear()
	{

		zFnMesh::clear();

		fnParticles.clear();
		particlesObj.clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnMeshDynamics::makeDynamic(bool fixBoundary)
	{
		fnParticles.clear();
		particlesObj.clear();

		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			bool fixed = false;

			if (fixBoundary) fixed = (v.onBoundary());

			zObjParticle p;
			p.particle = zParticle(*v.getRawPosition(), fixed);
			particlesObj.push_back(p);

			if (!fixed) setVertexColor(zColor(0, 0, 1, 1));
		}

		for (int i = 0; i < particlesObj.size(); i++)
		{
			fnParticles.push_back(zFnParticle(particlesObj[i]));
		}

	}

	ZSPACE_INLINE void zFnMeshDynamics::create(zObjMesh &_meshObj, bool fixBoundary)
	{
		meshObj = &_meshObj;
		makeDynamic(fixBoundary);

	}

	//---- FORCE METHODS 


	ZSPACE_INLINE void zFnMeshDynamics::addGravityForce(zVector& gForce)
	{
		for (auto &fnP : fnParticles)
		{
			fnP.addForce(gForce);
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addDragForce(float drag)
	{
		for (auto& fnP : fnParticles)
		{
			zVector v = fnP.getVelocity();
			zVector pForce = v * drag * -1;
			fnP.addForce(pForce);
		}
	
	}

	ZSPACE_INLINE void zFnMeshDynamics::addSpringForce(zFloatArray& restLength, double strength)
	{
		zPoint* vPositions = getRawVertexPositions();

		for (zItMeshEdge e(*meshObj); !e.end(); e++)
		{
			zIntArray eVerts;
			e.getVertices(eVerts);

			zVector eVec = vPositions[eVerts[1]] - vPositions[eVerts[0]];
			float eLen = e.getLength();
			eVec.normalize();

			float restLen = restLength[e.getId()];

			float val = strength * (eLen - restLen);
			zVector pForce_v1 = eVec * (val * 0.5);

			zVector pForce_v2 = pForce_v1 * -1;

			fnParticles[eVerts[0]].addForce(pForce_v1);
			fnParticles[eVerts[1]].addForce(pForce_v2);
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addPlanarityForce(zPlanarSolverType type, double& tolerance, zDoubleArray& planarityDeviations, bool& exit)
	{

		printf("\n numP %i %i ", particlesObj.size(), fnParticles.size());

		zUtilsCore core;
		exit = true;

		zPoint* vPositions = getRawVertexPositions();

		if (planarityDeviations.size() != numPolygons())
		{
			planarityDeviations.clear();
			planarityDeviations.assign(numPolygons(), -1);
		}

		if (type == zQuadPlanar)
		{
			for (zItMeshFace f(*meshObj); !f.end(); f++)
			{
				double uA, uB;
				zPoint pA, pB;

				int fID = f.getId();				

				zIntArray fVerts;
				f.getVertices(fVerts);

				bool check = core.line_lineClosestPoints(vPositions[fVerts[0]], vPositions[fVerts[2]], vPositions[fVerts[1]], vPositions[fVerts[3]], uA, uB, pA, pB);

				planarityDeviations[fID] = pA.distanceTo(pB);

				if (planarityDeviations[fID] > tolerance)
				{
					exit = false;

					zVector dir = pB - pA;
					dir.normalize();

					zVector pForceA = dir * planarityDeviations[fID] * 0.5;
					zVector pForceB = dir * planarityDeviations[fID] * -0.5;

					fnParticles[fVerts[0]].addForce(pForceA);
					fnParticles[fVerts[2]].addForce(pForceA);

					fnParticles[fVerts[1]].addForce(pForceB);
					fnParticles[fVerts[3]].addForce(pForceB);
				}

			}
		}

		else if (type == zVolumePlanar)
		{
			for (zItMeshFace f(*meshObj); !f.end(); f++)
			{
				int fID = f.getId();

				zIntArray fVerts;
				f.getVertices(fVerts);

				zIntArray fTris;
				zPoint fCenter = f.getCenter();
				zVector fNorm = f.getNormal();
				//planarityDeviations[fID] = f.getVolume(fTris, fCenter,false);

				planarityDeviations[fID] = -1;

				for (int k = 0; k < fVerts.size(); k++)
				{
					double dist = core.minDist_Point_Plane(vPositions[fVerts[k]], fCenter, fNorm);
					if (dist > planarityDeviations[fID]) planarityDeviations[fID] = dist;

				}

			

				if (planarityDeviations[fID] > tolerance)
				{
					exit = false;

					for (int k = 0; k < fVerts.size(); k++)
					{
						double dist = core.minDist_Point_Plane(vPositions[fVerts[k]], fCenter, fNorm);
						zVector pForce = fNorm * dist * -1.0;
						fnParticles[fVerts[k]].addForce(pForce);

					}
				}
			}
		}

	}

	ZSPACE_INLINE void zFnMeshDynamics::addPlanarityForce_targetPlane(zPlanarSolverType type, zPointArray& targetCenters, zVectorArray& targetNormals, double& tolerance, zDoubleArray& planarityDeviations, bool& exit)
	{
		zUtilsCore core;
		zPoint* vPositions = getRawVertexPositions();

		exit = true;

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			int fID = f.getId();

			zIntArray fVerts;
			f.getVertices(fVerts);

			planarityDeviations[fID] = -1;

			for (int k = 0; k < fVerts.size(); k++)
			{
				double dist = core.minDist_Point_Plane(vPositions[fVerts[k]], targetCenters[fID], targetNormals[fID]);
				if (dist > planarityDeviations[fID]) planarityDeviations[fID] = dist;

			}


			if (planarityDeviations[fID] > tolerance)
			{
				exit = false;

				for (int k = 0; k < fVerts.size(); k++)
				{
					double dist = core.minDist_Point_Plane(vPositions[fVerts[k]], targetCenters[fID], targetNormals[fID]);
					zVector pForce = targetNormals[fID] * dist * -1.0;
					fnParticles[fVerts[k]].addForce(pForce);

				}
			}
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addGaussianForce(double& tolerance, zDoubleArray& vGaussianCurvatures, bool& exit)
	{

		//compute gaussian curvature
		zDoubleArray vGauss;
		getGaussianCurvature(vGauss);

		zPoint* vPositions = getRawVertexPositions();

		// compute gradient and force
		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			zVector angGrad;
			zVector gForce;

			int vID = v.getId();

			// boundary vertices
			if (v.onBoundary())
			{

			}

			// interval vertices
			else
			{
				// get 1 Ring neighbours
				zItMeshHalfEdgeArray cHEdges;
				v.getConnectedHalfEdges(cHEdges);

				zItMeshVertexArray nRVerts;

				for (auto& cHe : cHEdges)
				{
					zItMeshHalfEdge he = cHe;

					do
					{
						zItMeshVertex v = he.getVertex();

						bool chkRepeat = false;
						for (int i = 0; i < nRVerts.size(); i++)
						{
							if (nRVerts[i].getId() == v.getId())
							{
								chkRepeat = true;
								break;
							}
						}

						if (!chkRepeat) nRVerts.push_back(v);
						he = he.getNext();

					} while (he != cHe);
				}

				// compute angle gradient
				
				
				for (int i =0; i< nRVerts.size(); i++ )
				{
					zItMeshVertex Vj = nRVerts[i];

					zItMeshVertex Vnext = nRVerts[(i + 1) % nRVerts.size()];
					zItMeshVertex Vprev = nRVerts[(i - 1) % nRVerts.size()];

					zVector eFlip = vPositions[v.getId()] - vPositions[Vj.getId()];
					zVector eNext = vPositions[Vnext.getId()] - vPositions[Vj.getId()];
					zVector ePrev = vPositions[Vprev.getId()] - vPositions[Vj.getId()];

					//float rho = eFlip.angle(eNext);
					//float delta = eFlip.angle(ePrev);

					zVector e = vPositions[Vj.getId()] - vPositions[v.getId()];

					zVector grad = e *((eFlip.cotan(eNext) + eFlip.cotan(ePrev))/ e.length()) ;

					angGrad += e;
				}

				angGrad.normalize();

			}

			gForce = angGrad * vGauss[vID];
			fnParticles[v.getId()].addForce(gForce);
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addMinimizeAreaForce(double strength)
	{
		int currentIndex = 0;

		zInt2DArray faceTris;
		getMeshTriangles(faceTris);

		zPoint* vPositions = getRawVertexPositions();


		for (int i = 0; i < numPolygons(); i++)
		{
			for (int j = 0; j < faceTris[i].size(); j += 3)
			{
				zPoint PA = vPositions[faceTris[i][j + 0]];
				zPoint PB = vPositions[faceTris[i][j + 1]];
				zPoint PC = vPositions[faceTris[i][j + 2]];

				zVector AB = PB - PA;
				zVector BC = PC - PB;
				zVector CA = PA - PC;

				zVector Normal = AB ^ BC;
				Normal.normalize();

				zVector V0 = (BC ^ Normal) * 0.5;
				zVector V1 = (CA ^ Normal) * 0.5;

				zVector pForce0 = V0 * strength;
				fnParticles[faceTris[i][j + 0]].addForce(pForce0);

				zVector pForce1 = V1 * strength;
				fnParticles[faceTris[i][j + 1]].addForce(pForce1);

				zVector pForce2 = ((V0 * -1) - V1) * strength;
				fnParticles[faceTris[i][j + 2]].addForce(pForce2);
			}
		}
	}


	//---- UPDATE METHODS 

	ZSPACE_INLINE void zFnMeshDynamics::update(double dT, zIntergrationType type, bool clearForce, bool clearVelocity, bool clearDerivatives)
	{
		for (int i = 0; i < fnParticles.size(); i++)
		{
			fnParticles[i].integrateForces(dT, type);
			fnParticles[i].updateParticle(clearForce, clearVelocity, clearDerivatives);
		}

		computeMeshNormals();
	}

}