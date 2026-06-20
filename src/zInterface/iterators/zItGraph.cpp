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


#include<zspace/zInterface/iterators/zItGraph.h>
#include <src/zInterface/objects/zGraphObjectStorage.h>
#include <src/zCore/geometry/detail/zGraphStorage.h>

//---- ZIT_GRAPH_VERTEX ------------------------------------------------------------------------------

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zItGraphVertex::zItGraphVertex()
	{
		graphObj = nullptr;
		index = 0;
	}

	ZSPACE_INLINE zItGraphVertex::zItGraphVertex(zObjectGraph &_graphObj)
	{
		graphObj = &_graphObj;
		index = 0;
	}

	ZSPACE_INLINE zItGraphVertex::zItGraphVertex(zObjectGraph &_graphObj, int _index)
	{
		graphObj = &_graphObj;
		if (_index < 0 || _index >= zGraphObjectStorage::read(*graphObj).numVertices()) throw std::invalid_argument(" error: index out of bounds");
		index = _index;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zItGraphVertex::begin()
	{
		index = 0;
	}

	ZSPACE_INLINE void zItGraphVertex::operator++(int)
	{
		index++;
	}

	ZSPACE_INLINE void zItGraphVertex::operator--(int)
	{
		index--;
	}

	ZSPACE_INLINE bool zItGraphVertex::end()
	{
		return index >= zGraphObjectStorage::read(*graphObj).numVertices();
	}

	ZSPACE_INLINE void zItGraphVertex::reset()
	{
		index = 0;
	}

	ZSPACE_INLINE int zItGraphVertex::size()
	{

		return zGraphObjectStorage::read(*graphObj).numVertices();
	}

	ZSPACE_INLINE void zItGraphVertex::deactivate()
	{
		zGraph& topology = zGraphObjectStorage::get(*graphObj);
		topology.vHandles[index] = zVertexHandle();
		topology.vertices[index].reset();
	}

	//---- TOPOLOGY QUERY METHODS

	ZSPACE_INLINE void zItGraphVertex::getConnectedHalfEdges(zItGraphHalfEdgeArray& halfedges)
	{
		if (!getHalfEdge().isActive()) return;

		zItGraphHalfEdge start = getHalfEdge();
		zItGraphHalfEdge e = getHalfEdge();

		bool exit = false;

		do
		{
			halfedges.push_back(e);
			e = e.getPrev().getSym();

		} while (e != start);
	}

	ZSPACE_INLINE void zItGraphVertex::getConnectedHalfEdges(zIntArray& halfedgeIndicies)
	{
		if (!getHalfEdge().isActive()) return;


		zItGraphHalfEdge start = getHalfEdge();
		zItGraphHalfEdge e = getHalfEdge();

		bool exit = false;

		do
		{
			halfedgeIndicies.push_back(e.getId());
			e = e.getPrev().getSym();

		} while (e != start);
	}

	ZSPACE_INLINE void zItGraphVertex::getConnectedEdges(zItGraphEdgeArray& edges)
	{
		zItGraphHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);

		for (auto &he : cHEdges)
		{
			edges.push_back(he.getEdge());
		}
	}

	ZSPACE_INLINE void zItGraphVertex::getConnectedEdges(zIntArray& edgeIndicies)
	{
		zItGraphHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);

		for (auto &he : cHEdges)
		{
			edgeIndicies.push_back(he.getEdge().getId());
		}
	}

	ZSPACE_INLINE void zItGraphVertex::getConnectedVertices(zItGraphVertexArray& verticies)
	{
		zItGraphHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);


		for (auto &he : cHEdges)
		{
			verticies.push_back(he.getVertex());
		}
	}

	ZSPACE_INLINE void zItGraphVertex::getConnectedVertices(zIntArray& vertexIndicies)
	{
		zItGraphHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);


		for (auto &he : cHEdges)
		{
			vertexIndicies.push_back(he.getVertex().getId());
		}
	}

	ZSPACE_INLINE int zItGraphVertex::getValence()
	{
		int out;

		zIntArray cHEdges;
		getConnectedHalfEdges(cHEdges);

		out = cHEdges.size();

		return out;
	}

	ZSPACE_INLINE void zItGraphVertex::getBSF(zItGraphVertexArray& bsf, zIntPairArray& vertexPairs)
	{
		int verticesVisitedCounter = 0;

		zBoolArray vertsVisited;
		vertsVisited.assign(zGraphObjectStorage::read(*graphObj).numVertices(), false);

		zItGraphVertexArray currentVertex = { zItGraphVertex(*graphObj, getId()) };
		bool exit = false;

		bsf.clear();
		bsf.push_back(currentVertex[0]);

		vertexPairs.clear();

		do
		{
			zItGraphVertexArray  temp;
			temp.clear();

			for (auto currentV : currentVertex)
			{
				if (vertsVisited[currentV.getId()]) continue;

				zItGraphVertexArray cVerts;
				currentV.getConnectedVertices(cVerts);

				for (auto v : cVerts)
				{
					bool checkRepeat = false;

					if (!vertsVisited[v.getId()])
					{						
						for (auto tmpV : temp)
						{
							if (tmpV.getId() == v.getId())
							{
								checkRepeat = true;
								break;
							}
						}

						if (!checkRepeat)
						{
							// check for repeat in bsf
							for (auto tmpV : bsf)
							{
								if (tmpV.getId() == v.getId())
								{
									checkRepeat = true;
									break;
								}
							}
						}

						if (!checkRepeat)
						{
							temp.push_back(v);

							vertexPairs.push_back(zIntPair(currentV.getId(), v.getId()));
						}
					}				
				}
				vertsVisited[currentV.getId()] = true;
				verticesVisitedCounter++;
			}

			currentVertex.clear();
			if (temp.size() == 0) exit = true;

			currentVertex = temp;

			for (auto v : temp) bsf.push_back(v);
			
		} while (verticesVisitedCounter != zGraphObjectStorage::read(*graphObj).numVertices() && !exit);
	}

	ZSPACE_INLINE void zItGraphVertex::getBSF(zIntArray& bsf, zIntPairArray& vertexPairs)
	{
		zItGraphVertexArray itBSF;
		getBSF(itBSF, vertexPairs);

		for (auto it : itBSF)
			bsf.push_back(it.getId());		
	}

	ZSPACE_INLINE bool zItGraphVertex::checkValency(int valence)
	{
		bool out = false;
		out = (getValence() == valence) ? true : false;

		return out;
	}

	//---- GET METHODS

	ZSPACE_INLINE int zItGraphVertex::getId()
	{
		return index;
	}

	ZSPACE_INLINE zItGraphHalfEdge zItGraphVertex::getHalfEdge()
	{
		return zItGraphHalfEdge(*graphObj, zGraphObjectStorage::get(*graphObj).vertices[index].getHalfEdge());
	}

	ZSPACE_INLINE zItVertex zItGraphVertex::getRawIter()
	{
		iter = zGraphObjectStorage::get(*graphObj).vertices.begin();
		advance(iter, index);
		return iter;
	}

	ZSPACE_INLINE zPoint zItGraphVertex::getPosition()
	{
		return zGraphObjectStorage::read(*graphObj).positions[index];
	}

	ZSPACE_INLINE zPoint* zItGraphVertex::getRawPosition()
	{
		return &zGraphObjectStorage::edit(*graphObj).positions[index];
	}

	ZSPACE_INLINE zColor zItGraphVertex::getColor()
	{
		return zGraphObjectStorage::read(*graphObj).vertexColors[index];
	}

	ZSPACE_INLINE zColor* zItGraphVertex::getRawColor()
	{
		return &zGraphObjectStorage::edit(*graphObj).vertexColors[index];
	}

	//---- SET METHODS

	ZSPACE_INLINE void zItGraphVertex::setId(int _id)
	{
		index = _id;
	}

	ZSPACE_INLINE void zItGraphVertex::setHalfEdge(zItGraphHalfEdge &he)
	{
		//iter->setHalfEdge(he.getId());

		int id = getId();
		int heId = he.getId();

		zGraph& topology = zGraphObjectStorage::get(*graphObj);
		topology.vertices[id].setHalfEdge(heId);
		topology.vHandles[id].he = heId;
	}

	ZSPACE_INLINE void zItGraphVertex::setPosition(zVector &pos)
	{
		zGraphObjectStorage::edit(*graphObj).positions[index] = pos;
	}

	ZSPACE_INLINE void zItGraphVertex::setColor(zColor col)
	{
		zGraphObjectStorage::edit(*graphObj).vertexColors[index] = col;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE bool zItGraphVertex::isActive()
	{
		return graphObj && index >= 0 && index < zGraphObjectStorage::read(*graphObj).numVertices();
	}

	//---- OPERATOR METHODS

	ZSPACE_INLINE bool zItGraphVertex::operator==(zItGraphVertex &other)
	{
		return (getId() == other.getId());
	}

	ZSPACE_INLINE bool zItGraphVertex::operator!=(zItGraphVertex &other)
	{
		return (getId() != other.getId());
	}


}

//---- ZIT_GRAPH_EDGE ------------------------------------------------------------------------------

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zItGraphEdge::zItGraphEdge()
	{
		graphObj = nullptr;
		index = 0;
	}

	ZSPACE_INLINE zItGraphEdge::zItGraphEdge(zObjectGraph &_graphObj)
	{
		graphObj = &_graphObj;
		index = 0;
	}

	ZSPACE_INLINE zItGraphEdge::zItGraphEdge(zObjectGraph &_graphObj, int _index)
	{
		graphObj = &_graphObj;
		if (_index < 0 || _index >= zGraphObjectStorage::read(*graphObj).numEdges()) throw std::invalid_argument(" error: index out of bounds");
		index = _index;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zItGraphEdge::begin()
	{
		index = 0;
	}

	ZSPACE_INLINE void zItGraphEdge::operator++(int)
	{
		index++;
	}

	ZSPACE_INLINE void zItGraphEdge::operator--(int)
	{
		index--;
	}

	ZSPACE_INLINE bool zItGraphEdge::end()
	{
		return index >= zGraphObjectStorage::read(*graphObj).numEdges();
	}

	ZSPACE_INLINE void zItGraphEdge::reset()
	{
		index = 0;
	}

	ZSPACE_INLINE int zItGraphEdge::size()
	{

		return zGraphObjectStorage::read(*graphObj).numEdges();
	}

	ZSPACE_INLINE void zItGraphEdge::deactivate()
	{
		zGraph& topology = zGraphObjectStorage::get(*graphObj);
		topology.eHandles[index] = zEdgeHandle();
		topology.edges[index].reset();
	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE void zItGraphEdge::getVertices(zItGraphVertexArray &verticies)
	{
		zIntArray vertexIndicies;
		getVertices(vertexIndicies);
		verticies.push_back(zItGraphVertex(*graphObj, vertexIndicies[0]));
		verticies.push_back(zItGraphVertex(*graphObj, vertexIndicies[1]));
	}

	ZSPACE_INLINE void zItGraphEdge::getVertices(zIntArray &vertexIndicies)
	{
		const auto& data = zGraphObjectStorage::read(*graphObj);
		vertexIndicies.push_back(data.edgeVertexIndices[index * 2]);
		vertexIndicies.push_back(data.edgeVertexIndices[index * 2 + 1]);
	}

	ZSPACE_INLINE void zItGraphEdge::getVertexPositions(vector<zVector> &vertPositions)
	{
		zIntArray eVerts;

		getVertices(eVerts);

		for (int i = 0; i < eVerts.size(); i++)
		{
			vertPositions.push_back(zGraphObjectStorage::read(*graphObj).positions[eVerts[i]]);
		}
	}

	ZSPACE_INLINE zVector zItGraphEdge::getCenter()
	{
		zIntArray eVerts;
		getVertices(eVerts);

		zPoint p0 = zGraphObjectStorage::read(*graphObj).positions[eVerts[0]];
		zPoint p1 = zGraphObjectStorage::read(*graphObj).positions[eVerts[1]];
		return (p0 + p1) * 0.5;
	}

	ZSPACE_INLINE zVector zItGraphEdge::getVector()
	{

		zIntArray eVerts;
		getVertices(eVerts);
		int v1 = eVerts[0];
		int v2 = eVerts[1];

		zPoint p1 = zGraphObjectStorage::read(*graphObj).positions[v1];
		zPoint p2 = zGraphObjectStorage::read(*graphObj).positions[v2];
		zVector out = p1 - p2;

		return out;
	}

	ZSPACE_INLINE double zItGraphEdge::getLength()
	{
		return getVector().length();
	}

	//---- GET METHODS

	ZSPACE_INLINE int zItGraphEdge::getId()
	{
		return index;
	}

	ZSPACE_INLINE zItGraphHalfEdge zItGraphEdge::getHalfEdge(int _index)
	{
		return zItGraphHalfEdge(*graphObj, zGraphObjectStorage::get(*graphObj).edges[index].getHalfEdge(_index));
	}

	ZSPACE_INLINE zItEdge  zItGraphEdge::getRawIter()
	{
		iter = zGraphObjectStorage::get(*graphObj).edges.begin();
		advance(iter, index);
		return iter;
	}

	ZSPACE_INLINE zColor zItGraphEdge::getColor()
	{
		return zGraphObjectStorage::read(*graphObj).edgeColors[index];
	}

	ZSPACE_INLINE zColor* zItGraphEdge::getRawColor()
	{
		return &zGraphObjectStorage::edit(*graphObj).edgeColors[index];
	}

	//---- SET METHODS

	ZSPACE_INLINE void zItGraphEdge::setId(int _id)
	{
		index = _id;
	}

	ZSPACE_INLINE void zItGraphEdge::setHalfEdge(zItGraphHalfEdge &he, int _index)
	{
		//iter->setHalfEdge(he.getId(), _index);


		int id = getId();
		int heId = he.getId();
		zGraph& topology = zGraphObjectStorage::get(*graphObj);
		topology.edges[id].setHalfEdge(heId, _index);
		if (_index == 0) topology.eHandles[id].he0 = heId;
		if (_index == 1) topology.eHandles[id].he1 = heId;
	}

	ZSPACE_INLINE void zItGraphEdge::setColor(zColor col)
	{
		zGraphObjectStorage::edit(*graphObj).edgeColors[index] = col;
	}

	ZSPACE_INLINE void zItGraphEdge::setWeight(double wt)
	{
		zGraphObjectStorage::edit(*graphObj).edgeWeights[index] = wt;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE bool zItGraphEdge::isActive()
	{
		return graphObj && index >= 0 && index < zGraphObjectStorage::read(*graphObj).numEdges();
	}

	//---- OPERATOR METHODS

	ZSPACE_INLINE bool zItGraphEdge::operator==(zItGraphEdge &other)
	{
		return (getId() == other.getId());
	}

	ZSPACE_INLINE bool zItGraphEdge::operator!=(zItGraphEdge &other)
	{
		return (getId() != other.getId());
	}

}


//---- ZIT_GRAPH_HALFEDGE ------------------------------------------------------------------------------

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zItGraphHalfEdge::zItGraphHalfEdge()
	{
		graphObj = nullptr;
	}

	ZSPACE_INLINE zItGraphHalfEdge::zItGraphHalfEdge(zObjectGraph &_graphObj)
	{
		graphObj = &_graphObj;

		iter = zGraphObjectStorage::get(*graphObj).halfEdges.begin();
	}

	ZSPACE_INLINE zItGraphHalfEdge::zItGraphHalfEdge(zObjectGraph &_graphObj, int _index)
	{
		graphObj = &_graphObj;

		iter = zGraphObjectStorage::get(*graphObj).halfEdges.begin();

		if (_index < 0 && _index >= zGraphObjectStorage::get(*graphObj).halfEdges.size()) throw std::invalid_argument(" error: index out of bounds");
		advance(iter, _index);
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zItGraphHalfEdge::begin()
	{
		iter = zGraphObjectStorage::get(*graphObj).halfEdges.begin();
	}

	ZSPACE_INLINE void zItGraphHalfEdge::operator++(int)
	{
		iter++;
	}

	ZSPACE_INLINE void zItGraphHalfEdge::operator--(int)
	{
		iter--;
	}

	ZSPACE_INLINE bool zItGraphHalfEdge::end()
	{
		return (iter == zGraphObjectStorage::get(*graphObj).halfEdges.end()) ? true : false;
	}

	ZSPACE_INLINE void zItGraphHalfEdge::reset()
	{
		iter = zGraphObjectStorage::get(*graphObj).halfEdges.begin();
	}

	ZSPACE_INLINE int zItGraphHalfEdge::size()
	{

		return zGraphObjectStorage::get(*graphObj).halfEdges.size();
	}

	ZSPACE_INLINE void zItGraphHalfEdge::deactivate()
	{
		zGraphObjectStorage::get(*graphObj).heHandles[iter->getId()] = zHalfEdgeHandle();
		iter->reset();
	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE zItGraphVertex zItGraphHalfEdge::getStartVertex()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");

		return getSym().getVertex();
	}

	ZSPACE_INLINE void zItGraphHalfEdge::getVertices(zItGraphVertexArray &verticies)
	{
		verticies.push_back(getVertex());
		verticies.push_back(getSym().getVertex());
	}

	ZSPACE_INLINE void zItGraphHalfEdge::getVertices(zIntArray &vertexIndicies)
	{
		vertexIndicies.push_back(getVertex().getId());
		vertexIndicies.push_back(getSym().getVertex().getId());
	}

	ZSPACE_INLINE void zItGraphHalfEdge::getVertexPositions(vector<zVector> &vertPositions)
	{
		zIntArray eVerts;

		getVertices(eVerts);

		for (int i = 0; i < eVerts.size(); i++)
		{
			vertPositions.push_back(zGraphObjectStorage::get(*graphObj).vertexPositions[eVerts[i]]);
		}
	}

	ZSPACE_INLINE void zItGraphHalfEdge::getConnectedHalfEdges(zItGraphHalfEdgeArray& edgeIndicies)
	{
		zItGraphVertex v1 = getVertex();
		zItGraphHalfEdgeArray connectedEdgestoVert0;
		v1.getConnectedHalfEdges(connectedEdgestoVert0);

		zItGraphVertex v2 = getSym().getVertex();
		zItGraphHalfEdgeArray connectedEdgestoVert1;
		v2.getConnectedHalfEdges(connectedEdgestoVert1);

		for (int i = 0; i < connectedEdgestoVert0.size(); i++)
		{
			if (connectedEdgestoVert0[i].getId() != getId()) edgeIndicies.push_back(connectedEdgestoVert0[i]);
		}


		for (int i = 0; i < connectedEdgestoVert1.size(); i++)
		{
			if (connectedEdgestoVert1[i].getId() != getId()) edgeIndicies.push_back(connectedEdgestoVert1[i]);
		}
	}

	ZSPACE_INLINE void zItGraphHalfEdge::getConnectedHalfEdges(zIntArray& edgeIndicies)
	{
		zItGraphVertex v1 = getVertex();
		zIntArray connectedEdgestoVert0;
		v1.getConnectedHalfEdges(connectedEdgestoVert0);

		zItGraphVertex v2 = getSym().getVertex();
		zIntArray connectedEdgestoVert1;
		v2.getConnectedHalfEdges(connectedEdgestoVert1);

		for (int i = 0; i < connectedEdgestoVert0.size(); i++)
		{
			if (connectedEdgestoVert0[i] != getId()) edgeIndicies.push_back(connectedEdgestoVert0[i]);
		}


		for (int i = 0; i < connectedEdgestoVert1.size(); i++)
		{
			if (connectedEdgestoVert1[i] != getId()) edgeIndicies.push_back(connectedEdgestoVert1[i]);
		}
	}

	ZSPACE_INLINE bool zItGraphHalfEdge::onBoundary()
	{
		return (iter->getFace() == -1) ? true : false;
	}

	ZSPACE_INLINE zVector zItGraphHalfEdge::getCenter()
	{
		zIntArray eVerts;
		getVertices(eVerts);

		return (zGraphObjectStorage::get(*graphObj).vertexPositions[eVerts[0]] + zGraphObjectStorage::get(*graphObj).vertexPositions[eVerts[1]]) * 0.5;
	}

	ZSPACE_INLINE zVector zItGraphHalfEdge::getVector()
	{

		int v1 = getVertex().getId();
		int v2 = getSym().getVertex().getId();

		zVector out = zGraphObjectStorage::get(*graphObj).vertexPositions[v1] - (zGraphObjectStorage::get(*graphObj).vertexPositions[v2]);

		return out;
	}

	ZSPACE_INLINE double zItGraphHalfEdge::getLength()
	{
		return getVector().length();
	}

	//---- GET METHODS

	ZSPACE_INLINE int zItGraphHalfEdge::getId()
	{
		return iter->getId();
	}

	ZSPACE_INLINE zItGraphHalfEdge zItGraphHalfEdge::getSym()
	{
		return zItGraphHalfEdge(*graphObj, iter->getSym());
	}

	ZSPACE_INLINE zItGraphHalfEdge zItGraphHalfEdge::getNext()
	{
		return zItGraphHalfEdge(*graphObj, iter->getNext());
	}

	ZSPACE_INLINE zItGraphHalfEdge zItGraphHalfEdge::getPrev()
	{
		return zItGraphHalfEdge(*graphObj, iter->getPrev());
	}

	ZSPACE_INLINE zItGraphVertex zItGraphHalfEdge::getVertex()
	{
		return zItGraphVertex(*graphObj, iter->getVertex());
	}

	ZSPACE_INLINE zItGraphEdge zItGraphHalfEdge::getEdge()
	{
		return zItGraphEdge(*graphObj, iter->getEdge());
	}

	ZSPACE_INLINE zItHalfEdge  zItGraphHalfEdge::getRawIter()
	{
		return iter;
	}

	ZSPACE_INLINE zColor zItGraphHalfEdge::getColor()
	{
		return zGraphObjectStorage::get(*graphObj).edgeColors[iter->getEdge()];
	}

	ZSPACE_INLINE zColor* zItGraphHalfEdge::getRawColor()
	{
		return &zGraphObjectStorage::get(*graphObj).edgeColors[iter->getEdge()];
	}

	//---- SET METHODS

	ZSPACE_INLINE void zItGraphHalfEdge::setId(int _id)
	{
		iter->setId(_id);
	}

	ZSPACE_INLINE void zItGraphHalfEdge::setSym(zItGraphHalfEdge &he)
	{
		iter->setSym(he.getId());
		he.iter->setSym(getId());
	}

	ZSPACE_INLINE void zItGraphHalfEdge::setNext(zItGraphHalfEdge &he)
	{
		//iter->setNext(he.getId());

		int id = getId();
		int nextId = he.getId();

		iter->setNext(nextId);
		//he.setPrev(*this);
		he.iter->setPrev(id);

		zGraphObjectStorage::get(*graphObj).heHandles[id].n = nextId;
		zGraphObjectStorage::get(*graphObj).heHandles[nextId].p = id;
	}

	ZSPACE_INLINE void zItGraphHalfEdge::setPrev(zItGraphHalfEdge &he)
	{
		//iter->setPrev(&zGraphObjectStorage::get(*graphObj).halfEdges[he.getId()]);
		int id = getId();
		int prevId = he.getId();

		iter->setPrev(prevId);
		//he.setNext(*this);
		he.iter->setNext(id);

		zGraphObjectStorage::get(*graphObj).heHandles[id].p = prevId;
		zGraphObjectStorage::get(*graphObj).heHandles[prevId].n = id;
	}

	ZSPACE_INLINE void zItGraphHalfEdge::setVertex(zItGraphVertex &v)
	{
		//iter->setVertex(&zGraphObjectStorage::get(*graphObj).vertices[v.getId()]);

		iter->setVertex(v.getId());

		int id = getId();
		int vId = v.getId();

		zGraphObjectStorage::get(*graphObj).heHandles[id].v = vId;
	}

	ZSPACE_INLINE void zItGraphHalfEdge::setEdge(zItGraphEdge &e)
	{
		//iter->setEdge(&zGraphObjectStorage::get(*graphObj).edges[e.getId()]);

		iter->setEdge(e.getId());

		int id = getId();
		int eId = e.getId();

		zGraphObjectStorage::get(*graphObj).heHandles[id].e = eId;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE bool zItGraphHalfEdge::isActive()
	{
		return iter->isActive();
	}

	//---- OPERATOR METHODS

	ZSPACE_INLINE bool zItGraphHalfEdge::operator==(zItGraphHalfEdge &other)
	{
		return (getId() == other.getId());
	}

	ZSPACE_INLINE bool zItGraphHalfEdge::operator!=(zItGraphHalfEdge &other)
	{
		return (getId() != other.getId());
	}

}
