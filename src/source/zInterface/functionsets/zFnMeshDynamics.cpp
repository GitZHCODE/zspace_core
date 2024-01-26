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


#include "zInterface/functionsets/zFnMeshDynamics.h"

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
		particlesObj.clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnMeshDynamics::makeDynamic(bool fixBoundary)
	{
		//fnParticles.clear();
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
				
	}

	ZSPACE_INLINE void zFnMeshDynamics::create(zObjMesh &_meshObj, bool fixBoundary)
	{
		meshObj = &_meshObj;
		makeDynamic(fixBoundary);

	}

	ZSPACE_INLINE int zFnMeshDynamics::numParticles()
	{
		return particlesObj.size();
	}

	//---- SET METHODS 

	ZSPACE_INLINE void zFnMeshDynamics::setFixed(zIntArray& vIDs)
	{
		for (int i = 0; i < vIDs.size(); i++)
		{
			if (vIDs[i] < particlesObj.size())
			{
				zFnParticle fnParticle(particlesObj[vIDs[i]]);
				fnParticle.setFixed(true);
			}
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::setFixed_boundary()
	{
		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			if (v.onBoundary())
			{
				if (v.getId() < particlesObj.size())
				{
					zFnParticle fnParticle(particlesObj[v.getId()]);
					fnParticle.setFixed(true);
				}
			}
			
		}
	}

	//---- FORCE METHODS 

	ZSPACE_INLINE void zFnMeshDynamics::addGravityForce(double strength, zVector& gForce)
	{
		for (auto &oParticle : particlesObj)
		{
			zFnParticle fnParticle(oParticle);
			zVector force = gForce * strength;
			fnParticle.addForce(force);
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addDragForce(double strength, float drag)
	{
		for (auto& oParticle : particlesObj)
		{
			zFnParticle fnParticle(oParticle);
			zVector v = fnParticle.getVelocity();
			zVector pForce = v * drag * -strength;
			fnParticle.addForce(pForce);
		}
	
	}

	ZSPACE_INLINE void zFnMeshDynamics::addSpringForce(double strength, zFloatArray& restLength)
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
			pForce_v1 *= strength;

			zVector pForce_v2 = pForce_v1 * -1;	
			pForce_v2 *= strength;
			
			zFnParticle fnParticle1(particlesObj[eVerts[0]]);
			zFnParticle fnParticle2(particlesObj[eVerts[1]]);


			fnParticle1.addForce(pForce_v1);
			fnParticle2.addForce(pForce_v2);
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addDistanceForce(double strength, int v0, int v1, float& restDistance)
	{
		zFnParticle p0(particlesObj[v0]);
		zFnParticle p1(particlesObj[v1]);

		zPoint* pos_v0 = p0.getRawPosition();
		zPoint* pos_v1 = p1.getRawPosition();
		zPoint pos_mid = (*pos_v0 + *pos_v1) * 0.5;

		zVector vec_v0 = *pos_v0 - *pos_v1;
		zVector vec_v1 = vec_v0 * -1;

		float displacement = restDistance - vec_v0.length();
		displacement /= 2;

		vec_v0.normalize();
		vec_v1.normalize();

		zVector pForce_v0, pForce_v1;

		pForce_v0 = vec_v0 * (displacement * strength);
		pForce_v1 = vec_v1 * (displacement * strength);

		p0.addForce(pForce_v0);
		p1.addForce(pForce_v1);
	}

	ZSPACE_INLINE void zFnMeshDynamics::addLoadForce(double strength, int vId, zVector& vec)
	{
		vec.normalize();
		zVector pForce = vec * strength;
		for (auto& oParticle : particlesObj)
		{
			zFnParticle fnParticle(oParticle);
			zVector force = pForce;
			fnParticle.addForce(force);
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addPlaneForce(double strength, zIntArray vIds, zPoint& targetCenter, zVector& targetNormal)
	{
		zUtilsCore core;

		for (auto& id : vIds)
		{
			zItMeshVertex v(*meshObj, id);

			double dist = core.minDist_Point_Plane(*v.getRawPosition(), targetCenter, targetNormal);
			zVector pForce = targetNormal * dist * -1.0;
			pForce = pForce * strength;

			zFnParticle fnParticle(particlesObj[id]);
			fnParticle.addForce(pForce);
		}
	}


	ZSPACE_INLINE void zFnMeshDynamics::addPlanarityForce(double strength, double tolerance, zPlanarSolverType type,  zDoubleArray& planarityDeviations, zVectorArray& forceDir, bool& exit, zSolverForceConstraints constrainType)
	{
		if (forceDir.size() != numVertices())
		{
			forceDir.clear();
			forceDir.assign(numVertices(), zVector());
		}

		exit = true;

		zUtilsCore core;
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

				if (fVerts.size() == 3) planarityDeviations[fID] = 0;

				if (fVerts.size() != 4) continue;

				bool check = core.line_lineClosestPoints(vPositions[fVerts[0]], vPositions[fVerts[2]], vPositions[fVerts[1]], vPositions[fVerts[3]], uA, uB, pA, pB);

				planarityDeviations[fID] = pA.distanceTo(pB);

				if (planarityDeviations[fID] > tolerance)
				{
					exit = false;

					zVector dir = pB - pA;
					dir.normalize();

					zVector pForceA = dir * planarityDeviations[fID] * 0.5;
					zVector pForceB = dir * planarityDeviations[fID] * -0.5;

					pForceA = pForceA * strength;
					pForceB = pForceB * strength;

					if (constrainType == zConstraintX)  { pForceA.x = 0; pForceB.x = 0; }
					if (constrainType == zConstraintY)  { pForceA.y = 0; pForceB.y = 0; }
					if (constrainType == zConstraintZ)  { pForceA.z = 0; pForceB.z = 0; }
					if (constrainType == zConstraintXY) { pForceA.x = 0; pForceB.x = 0;  pForceA.y = 0; pForceB.y = 0; }
					if (constrainType == zConstraintYZ) { pForceA.z = 0; pForceB.z = 0;  pForceA.y = 0; pForceB.y = 0; }
					if (constrainType == zConstraintZX) { pForceA.x = 0; pForceB.x = 0;  pForceA.z = 0; pForceB.z = 0; }


					zFnParticle fnParticle0(particlesObj[fVerts[0]]);
					zFnParticle fnParticle1(particlesObj[fVerts[1]]);
					zFnParticle fnParticle2(particlesObj[fVerts[2]]);
					zFnParticle fnParticle3(particlesObj[fVerts[3]]);

					fnParticle0.addForce(pForceA);
					fnParticle2.addForce(pForceA);

					fnParticle1.addForce(pForceB);
					fnParticle3.addForce(pForceB);

					forceDir[fVerts[0]] += pForceA;
					forceDir[fVerts[2]] += pForceA;

					forceDir[fVerts[1]] += pForceB;
					forceDir[fVerts[3]] += pForceB;
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
				/*float dev   = f.getVolume(fTris, fCenter,false);
				planarityDeviations[fID] = abs(dev);*/

				planarityDeviations[fID] = -1;

				for (int k = 0; k < fVerts.size(); k++)
				{
					double dist = core.minDist_Point_Plane(vPositions[fVerts[k]], fCenter, fNorm);
					if (dist > planarityDeviations[fID]) planarityDeviations[fID] = dist;

				}
							

				if (abs(planarityDeviations[fID]) > tolerance)
				{
					exit = false;

					for (int k = 0; k < fVerts.size(); k++)
					{
						double dist = core.minDist_Point_Plane(vPositions[fVerts[k]], fCenter, fNorm);
						zVector pForce = fNorm * dist * -1.0;
						pForce = pForce * strength;

						if (constrainType == zConstraintX)  { pForce.x = 0; }
						if (constrainType == zConstraintY)  { pForce.y = 0; }
						if (constrainType == zConstraintZ)  { pForce.z = 0; }
						if (constrainType == zConstraintXY) { pForce.x = 0; pForce.y = 0; }
						if (constrainType == zConstraintYZ) { pForce.z = 0; pForce.y = 0; }
						if (constrainType == zConstraintZX) { pForce.x = 0; pForce.z = 0; }


						zFnParticle fnParticle(particlesObj[fVerts[k]]);
						fnParticle.addForce(pForce);

						/*zVector pForce = fNorm * dev *-1;*/
						forceDir[fVerts[k]] += pForce;
					}

				}
					
				
			}
		}

		for (auto& fDir : forceDir)
		{
			if(fDir.length() > 0) fDir.normalize();
		}

	}

	ZSPACE_INLINE void zFnMeshDynamics::addPlanarityForce_vertexgroups(double strength, double tolerance, vector<zIntArray>& vertexIDs, zPointArray& targetCenters, zVectorArray& targetNormals, zDoubleArray& planarityDeviations, zVectorArray& forceDir, bool& exit)
	{

		if (vertexIDs.size() != targetNormals.size()) return;

		if (forceDir.size() != numVertices())
		{
			forceDir.clear();
			forceDir.assign(numVertices(), zVector());
		}

		if (planarityDeviations.size() != vertexIDs.size())
		{
			planarityDeviations.clear();
			planarityDeviations.assign(vertexIDs.size(), -1);
		}

		exit = true;

		zUtilsCore core;
		zPoint* vPositions = getRawVertexPositions();

		for (int i = 0; i < vertexIDs.size(); i++)
		{
			planarityDeviations[i] = -1;

			for (int k = 0; k < vertexIDs[i].size(); k++)
			{
				double dist = core.minDist_Point_Plane(vPositions[vertexIDs[i][k]], targetCenters[i], targetNormals[i]);
				if (dist > planarityDeviations[i]) planarityDeviations[i] = dist;
			}

			if (planarityDeviations[i] > tolerance)
			{
				exit = false;
				for (int k = 0; k < vertexIDs[i].size(); k++)
				{
					double dist = core.minDist_Point_Plane(vPositions[vertexIDs[i][k]], targetCenters[i], targetNormals[i]);
					if (dist > tolerance)
					{
						zVector pForce = targetNormals[i] * dist * -1.0;

						pForce = pForce * strength;

						zFnParticle fnParticle(particlesObj[vertexIDs[i][k]]);
						fnParticle.addForce(pForce);											

						forceDir[vertexIDs[i][k]] += pForce;						
					}
					
				}
			}
		}

		for (auto& fDir : forceDir)
		{
			if (fDir.length() > 0) fDir.normalize();
		}

	}

	ZSPACE_INLINE void zFnMeshDynamics::addPlanarityForce_targetPlane(double strength, double& tolerance, zPointArray& targetCenters, zVectorArray& targetNormals, zDoubleArray& planarityDeviations, zVectorArray& forceDir, bool& exit)
	{
		if (forceDir.size() != numVertices())
		{
			forceDir.clear();
			forceDir.assign(numVertices(), zVector());
		}

		if (planarityDeviations.size() != numPolygons())
		{
			planarityDeviations.clear();
			planarityDeviations.assign(numPolygons(), -1);
		}

		exit = true;

		zUtilsCore core;
		zPoint* vPositions = getRawVertexPositions();

		

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

					pForce = pForce * strength;

					zFnParticle fnParticle(particlesObj[fVerts[k]]);
					fnParticle.addForce(pForce);

					forceDir[fVerts[k]] += pForce;
				}
			}
				
			
		}

		for (auto& fDir : forceDir)
		{
			if (fDir.length() > 0) fDir.normalize();
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addDevelopabilityForce(double strength, double tolerance, zDoubleArray& vGaussianCurvatures, zVectorArray& forceDir, bool& exit )
	{
		if (forceDir.size() != numVertices())
		{
			forceDir.clear();
			forceDir.assign(numVertices(), zVector());
		}

		exit = true;

		//compute gaussian curvature
		vGaussianCurvatures.clear();
		getGaussianCurvature(vGaussianCurvatures);


		zPoint* vPositions = getRawVertexPositions();
			

		// compute gradient and force
		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			zVector angGrad;
			zVector gForce(0,0,0);


			double angleSum = 0;
			double vGauss = 0;

			int vID = v.getId();

			// boundary vertices
			if (v.onBoundary())
			{
				zItMeshHalfEdgeArray cHEdges;
				v.getConnectedHalfEdges(cHEdges);

				bool addForce = false;

				for (auto& cHe : cHEdges)
				{
					/////  SB CCF 
					if (!cHe.getVertex().onBoundary())
					{
						addForce = true;

					
						int vI = v.getId();
						int vJ = cHe.getVertex().getId();
						int vK = cHe.getNext().getVertex().getId();
						int vL = cHe.getSym().getPrev().getStartVertex().getId();

						vID = vJ;
						
						zVector e11 = vPositions[vI] - vPositions[vJ];
						zVector e12 = vPositions[vK] - vPositions[vJ];

						zVector e21 = vPositions[vI] - vPositions[vJ];
						zVector e22 = vPositions[vL] - vPositions[vJ];

						zVector g1 = (e11 ^ e12) ^ e11;
						g1.normalize();

						zVector g2 = (e21 ^ e22) ^ e21;
						g2.normalize();

						angGrad = g1 + g2;
						angGrad *= -1;
					}
				}

				if (addForce)
				{
					if (angGrad.length() > 0)
					{
						angGrad.normalize();
						gForce = angGrad * (vGaussianCurvatures[vID]) * strength;
						
						zFnParticle fnParticle(particlesObj[v.getId()]);
						fnParticle.addForce(gForce);

						forceDir[v.getId()] = angGrad * coreUtils.zSign(vGaussianCurvatures[v.getId()]);;
					}
					
				}

			}

			// interval vertices
			else
			{
				//if (fnParticles[v.getId()].getFixed()) vGaussianCurvatures[v.getId()] = 0.0;
			
				if (vGaussianCurvatures[v.getId()] > tolerance) exit = false;

				// get 1 Ring neighbours
				zItMeshHalfEdgeArray cHEdges;
				v.getConnectedHalfEdges(cHEdges);

				zItMeshVertexArray nRVerts;
				v.getConnectedVertices(nRVerts);

				// compute angle gradient			
				
				for (int i =0; i< nRVerts.size(); i++ )
				{
					zItMeshVertex Vj = nRVerts[i];

					zItMeshVertex Vnext = nRVerts[(i + 1) % nRVerts.size()];
					zItMeshVertex Vprev = nRVerts[(i - 1 + nRVerts.size()) % nRVerts.size()];

					zVector eFlip = vPositions[v.getId()] - vPositions[Vj.getId()];
					zVector e = vPositions[Vj.getId()] - vPositions[v.getId()];

					//////  Desbrun Cotangent Weights 
					
					/*zVector eNext = vPositions[Vnext.getId()] - vPositions[Vj.getId()];
					zVector ePrev = vPositions[Vprev.getId()] - vPositions[Vj.getId()];
					
					float rho = eFlip.angle(eNext);
					float delta = eFlip.angle(ePrev);

					float cotRHO = cos(rho) / sin(rho);
					float cotDELTA = cos(delta) / sin(delta);
					
					zVector grad = eFlip *((cotRHO + cotDELTA)/ eFlip.length()) ;
					angGrad += grad;*/
					
					
					//////  SB CCF Cotangent Weights
					
					/*zVector ePrev = vPositions[v.getId()] - vPositions[Vprev.getId()];
					zVector ePrev_J = vPositions[Vj.getId()] - vPositions[Vprev.getId()];

					zVector eNext = vPositions[v.getId()] - vPositions[Vnext.getId()];
					zVector eNext_J = vPositions[Vj.getId()] - vPositions[Vnext.getId()];

					float rho = ePrev.angle(ePrev_J);
					float delta = eNext.angle(eNext_J);

					float cotRHO = cos(rho) / sin(rho);
					float cotDELTA = cos(delta) / sin(delta);

					zVector grad = eFlip *((cotRHO + cotDELTA)/ eFlip.length()) ;
					angGrad += grad;*/
					

					//// only Gradient direction
					e.normalize();
					angGrad += e;

				}

				angGrad.normalize();
				gForce = angGrad * vGaussianCurvatures[v.getId()] * strength ;
				
				zFnParticle fnParticle(particlesObj[v.getId()]);
				fnParticle.addForce(gForce);
				
				forceDir[v.getId()] = angGrad * coreUtils.zSign(vGaussianCurvatures[v.getId()]);

			}

			
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
				zFnParticle fnParticle0(particlesObj[faceTris[i][j + 0]]);			
				fnParticle0.addForce(pForce0);

				zVector pForce1 = V1 * strength;
				zFnParticle fnParticle1(particlesObj[faceTris[i][j + 1]]);
				fnParticle1.addForce(pForce1);

				zVector pForce2 = ((V0 * -1) - V1) * strength;
				zFnParticle fnParticle2(particlesObj[faceTris[i][j + 2]]);
				fnParticle2.addForce(pForce2);
			}
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addRigidLineForce(double strength, double tolerance, zIntPairArray& vertexIDs, zDoubleArray& vertexDistances, zDoubleArray& deviations, zVectorArray& forceDir, bool& exit)
	{
		if (vertexDistances.size() != vertexIDs.size())
		{
			printf("\n vertexDistances %i | vertexIDs %i ", vertexDistances.size(), vertexIDs.size());
			throw std::invalid_argument(" error: vertexDistance and VertexIDs to be of the same size");
			
		}

		if (deviations.size() != vertexDistances.size())
		{
			deviations.clear();
			deviations.assign(vertexDistances.size(), -1);
		}

		if (forceDir.size() != numVertices())
		{
			forceDir.clear();
			forceDir.assign(numVertices(), zVector());
		}

		exit = true;

		zPoint* vPositions = getRawVertexPositions();

		for (int i =0; i< vertexDistances.size(); i++)
		{
			zIntPair p = vertexIDs[i];
			int v0 = p.first;
			int v1 = p.second;


			zVector dir_0 = vPositions[v1] - vPositions[v0];
			dir_0.normalize();

			zVector dir_1 = dir_0 * -1.0;

			float dist = vPositions[v1].distanceTo(vPositions[v0]);

			float diff = dist - vertexDistances[i];

			deviations[i] = abs(diff);

			bool print = false;

			if (deviations[i] > tolerance & dist > 0)
			{

				zVector pForceA = (diff < 0)? dir_1 * deviations[i] * 0.5 : dir_0 * deviations[i] * 0.5;
				pForceA *= strength;

				zVector pForceB = (diff < 0) ? dir_0 * deviations[i] * 0.5 : dir_1 * deviations[i] * 0.5;
				pForceB *= strength;

				zFnParticle fnParticle0(particlesObj[v0]);
				fnParticle0.addForce(pForceA);

				zFnParticle fnParticle1(particlesObj[v1]);
				fnParticle1.addForce(pForceB);

				forceDir[v0] += pForceA;
				forceDir[v1] += pForceB;

				exit = false;
			}
			
			

		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addSpringForce(double strength, int eId, float restLength)
	{
		zItMeshEdge e(*meshObj, eId);
		zIntArray vertices;
		e.getVertices(vertices);
		int v0 = vertices[0];
		int v1 = vertices[1];

		zFnParticle p0(particlesObj[v0]);
		zFnParticle p1(particlesObj[v1]);

		zPoint* pos_v0 = p0.getRawPosition();
		zPoint* pos_v1 = p1.getRawPosition();
		zPoint pos_mid = (*pos_v0 + *pos_v1) * 0.5;

		zVector vec_v0 = *pos_v0 - *pos_v1;
		zVector vec_v1 = vec_v0 * -1;

		float displacement = restLength - vec_v0.length();
		displacement /= 2;

		vec_v0.normalize();
		vec_v1.normalize();

		zVector pForce_v0, pForce_v1;

		pForce_v0 = vec_v0 * (displacement * strength);
		pForce_v1 = vec_v1 * (displacement * strength);

		p0.addForce(pForce_v0);
		p1.addForce(pForce_v1);
	}

	ZSPACE_INLINE void zFnMeshDynamics::addAngleForce(double strength, int vId_center, int vId_first, int vId_second, float restAngle, bool moveHinge)
	{
		//zItMeshVertex v(*meshObj, vId_center);
		//zIntArray connectedVertices;
		//v.getConnectedVertices(connectedVertices);

		int v0 = vId_first;
		int v1 = vId_center;
		int v2 = vId_second;

		// Get positions
		zFnParticle p0(particlesObj[v0]);
		zFnParticle p1(particlesObj[v1]);
		zFnParticle p2(particlesObj[v2]);

		zPoint* pos_v0 = p0.getRawPosition();
		zPoint* pos_v1 = p1.getRawPosition();
		zPoint* pos_v2 = p2.getRawPosition();

		// Calculate current vectors
		zVector vec_v1_v0 = *pos_v0 - *pos_v1;
		zVector vec_v1_v2 = *pos_v2 - *pos_v1;

		// Normalize the vectors
		vec_v1_v0.normalize();
		vec_v1_v2.normalize();

		// Calculate current angle
		float currentAngle = vec_v1_v0.angle(vec_v1_v2);// acos(vec_v1_v0 * vec_v1_v2);
		currentAngle *= DEG_TO_RAD;

		// Calculate difference between current and rest angle
		float angleDiff = restAngle - currentAngle;


		zVector force_v1 = (vec_v1_v0 + vec_v1_v2) * angleDiff * strength;

		if (moveHinge)
		{
			p1.addForce(force_v1);
		}

		else
		{
			zVector force_v0 = force_v1 * -1;
			zVector force_v2 = force_v1 * -1;

			p0.addForce(force_v0);
			p1.addForce(force_v1);
			p2.addForce(force_v2);
		}
	}

	ZSPACE_INLINE void zFnMeshDynamics::addFittingForce_cone(double strength, double tolerance, int centerVertexId, bool& exit)
	{

	}

	ZSPACE_INLINE void zFnMeshDynamics::addFittingForce_cylinder(double strength, double tolerance, int centerVertexId, bool& exit)
	{

	}

	//---- UPDATE METHODS 

	ZSPACE_INLINE void zFnMeshDynamics::update(double dT, zIntergrationType type, bool clearForce, bool clearVelocity, bool clearDerivatives)
	{
		for (auto& oParticle : particlesObj)
		{
			zFnParticle fnParticle(oParticle);
			fnParticle.integrateForces(dT, type);
			fnParticle.updateParticle(clearForce, clearVelocity, clearDerivatives);
		}		

		computeMeshNormals();
	}

}