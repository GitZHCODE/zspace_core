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


#include<headers/zInterface/functionsets/zFnGraphDynamics.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnGraphDynamics::zFnGraphDynamics()
	{
		fnType = zFnType::zGraphDynamicsFn;
		graphObj = nullptr;

	}

	ZSPACE_INLINE zFnGraphDynamics::zFnGraphDynamics(zObjGraph &_graphObj)
	{
		graphObj = &_graphObj;
		fnType = zFnType::zGraphDynamicsFn;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnGraphDynamics::~zFnGraphDynamics() {}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE zFnType zFnGraphDynamics::getType()
	{
		return zFnType::zGraphDynamicsFn;
	}

	ZSPACE_INLINE void zFnGraphDynamics::from(std::string path, zFileTpye type, bool staticGeom)
	{
		if (type == zTXT)
		{
			fromTXT(path);
			setStaticContainers();
		}
		else if (type == zMAYATXT)
		{
			fromMAYATXT(path);
			setStaticContainers();
		}
		else if (type == zJSON)
		{
			fromJSON(path);
			setStaticContainers();
		}

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnGraphDynamics::to(std::string path, zFileTpye type)
	{
		if (type == zTXT) toTXT(path);
		else if (type == zJSON) toJSON(path);

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnGraphDynamics::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		graphObj->getBounds(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnGraphDynamics::clear()
	{
		zFnGraph::clear();

		//fnParticles.clear();
		particlesObj.clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnGraphDynamics::makeDynamic(bool fixBoundary)
	{
		//fnParticles.clear();

		for (zItGraphVertex v(*graphObj); !v.end(); v++)
		{
			bool fixed = false;

			if (fixBoundary) fixed = (v.checkValency(1));

			zObjParticle p;
			p.particle = zParticle(*v.getRawPosition(), fixed);
			particlesObj.push_back(p);

			//if (!fixed) setVertexColor(zColor(0, 0, 1, 1));
		}

		/*for (int i = 0; i < particlesObj.size(); i++)
		{
			fnParticles.push_back(zFnParticle(particlesObj[i]));
		}*/
	}

	ZSPACE_INLINE void zFnGraphDynamics::create(zObjGraph &_graphObj, bool fixBoundary)
	{
		graphObj = &_graphObj;

		makeDynamic(fixBoundary);

	}

	//---- SET METHODS 

	ZSPACE_INLINE void zFnGraphDynamics::setFixed(zIntArray& vIDs)
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

	//---- FORCE METHODS 

	ZSPACE_INLINE void zFnGraphDynamics::addGravityForce(double strength, zVector& gForce)
	{
		for (auto& oParticle : particlesObj)
		{
			zFnParticle fnParticle(oParticle);
			zVector force = gForce * strength;
			fnParticle.addForce(force);
		}
	}

	ZSPACE_INLINE void zFnGraphDynamics::addDragForce(double strength, float drag)
	{
		for (auto& oParticle : particlesObj)
		{
			zFnParticle fnParticle(oParticle);
			zVector v = fnParticle.getVelocity();
			zVector pForce = v * drag * -strength;
			fnParticle.addForce(pForce);
		}

	}

	ZSPACE_INLINE void zFnGraphDynamics::addSpringForce(double strength, zFloatArray& restLength)
	{
		zPoint* vPositions = getRawVertexPositions();

		for (zItGraphEdge e(*graphObj); !e.end(); e++)
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

	ZSPACE_INLINE void zFnGraphDynamics::addLoadForce(double strength, int vId, zVector& vec)
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

	ZSPACE_INLINE void zFnGraphDynamics::addSpringForce(double strength, int eId, float restLength)
	{
		zItGraphEdge e(*graphObj, eId);
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

	ZSPACE_INLINE void zFnGraphDynamics::addDistanceForce(double strength, int v0, int v1, float& restDistance)
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

	ZSPACE_INLINE void zFnGraphDynamics::addSpringForces(double strength, float& restLength, const zIntArray& eIds)
	{
		if (eIds.size() == 0)
		{
			//apply force to all edges
			for (int i = 0; i < numEdges(); i++)
			{
				addSpringForce(strength, i, restLength);
			}
		}
		else
		{
			//apply force to specific edges
			for (int edgeId : eIds)
			{
				addSpringForce(strength, edgeId, restLength);
			}
		}
	}

	ZSPACE_INLINE void zFnGraphDynamics::addAngleForce(double strength, int vId, float restAngle, bool moveHinge)
	{
		zItGraphVertex v(*graphObj, vId);
		zIntArray connectedVertices;
		v.getConnectedVertices(connectedVertices);

		int v0 = connectedVertices[0];
		int v1 = v.getId();
		int v2 = connectedVertices[1];

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

	ZSPACE_INLINE void zFnGraphDynamics::addAngleForces(double strength, float& restAngle, bool& moveHinge, const zIntArray& vIds)
	{
		if (vIds.size() == 0)
		{
			//apply force to all edges
			for (int i = 0; i < numEdges(); i++)
			{
				addAngleForce(strength, i, restAngle, moveHinge);
			}
		}
		else
		{
			//apply force to specific edges
			for (int vId : vIds)
			{
				addAngleForce(strength, vId, moveHinge);
			}
		}
	}

	ZSPACE_INLINE void zFnGraphDynamics::addVectorForce(double strength, int vId, zPoint& origin, zVector& alignVector)
	{
		zItGraphVertex v(*graphObj, vId);
		zVector currentVec = *v.getRawPosition() - origin;

		alignVector.normalize();

		//double dot = currentVec * alignVector;
		//alignVector *= currentVec.length() * dot;

		alignVector *= currentVec.length();

		zPoint target = origin + alignVector;
		zVector targetVec = target - *v.getRawPosition();

		zVector pForce = targetVec * strength;

		zFnParticle p0(particlesObj[vId]);
		p0.addForce(pForce);
	}

	//---- UPDATE METHODS 

	ZSPACE_INLINE void zFnGraphDynamics::update(double dT, zIntergrationType type, bool clearForce , bool clearVelocity, bool clearDerivatives)
	{
		for (auto& oParticle : particlesObj)
		{
			zFnParticle fnParticle(oParticle);
			fnParticle.integrateForces(dT, type);
			fnParticle.updateParticle(clearForce, clearVelocity, clearDerivatives);
		}
	}

	//---- PRIVATE METHODS

	ZSPACE_INLINE void zFnGraphDynamics::setStaticContainers()
	{
		graphObj->graph.staticGeometry = true;

		vector<vector<int>> edgeVerts;

		for (zItGraphEdge e(*graphObj); !e.end(); e++)
		{
			vector<int> verts;
			e.getVertices(verts);

			edgeVerts.push_back(verts);
		}

		graphObj->graph.setStaticEdgeVertices(edgeVerts);
	}
}