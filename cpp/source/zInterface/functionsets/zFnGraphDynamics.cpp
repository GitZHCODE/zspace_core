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

	ZSPACE_INLINE void zFnGraphDynamics::from(string path, zFileTpye type, bool staticGeom)
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

	ZSPACE_INLINE void zFnGraphDynamics::to(string path, zFileTpye type)
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

			if (!fixed) setVertexColor(zColor(0, 0, 1, 1));
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