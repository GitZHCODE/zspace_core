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


#include<zspace/zInterface/functionsets/zFnGraph.h>

#include <set>
#include <src/zInterface/objects/zGraphObjectStorage.h>
#include <src/zCore/geometry/detail/zGraphStorage.h>
#include<zspace/zInterface/functionsets/zFnMesh.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnGraph::zFnGraph()
	{
		fnType = zFnType::zGraphFn;
		graphObj = nullptr;
		planarGraph = false;
		graphNormal = zVector(0, 0, 1);
	}

	ZSPACE_INLINE zFnGraph::zFnGraph(zObjectGraph &_graphObj, bool  _planarGraph, zVector _graphNormal)
	{
		fnType = zFnType::zGraphFn;

		setObject(_graphObj);

		planarGraph = _planarGraph;
		graphNormal = _graphNormal;
	}

	ZSPACE_INLINE void zFnGraph::setObject(zObjectGraph &_graphObject)
	{
		graphObj = &_graphObject;
	}

	ZSPACE_INLINE zObjectGraph* zFnGraph::object() const
	{
		return graphObj;
	}

	ZSPACE_INLINE bool zFnGraph::hasObject() const
	{
		return graphObj != nullptr;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnGraph::~zFnGraph() {}

	//---- OVERRIDE METHODS
	
	ZSPACE_INLINE zFnType zFnGraph::getType()
	{
		return zGraphFn;
	}

#if defined ZSPACE_USD_INTEROP

#endif

	ZSPACE_INLINE void zFnGraph::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		coreUtils.getBounds(zGraphObjectStorage::get(*graphObj).vertexPositions, minBB, maxBB);
	}

	ZSPACE_INLINE void zFnGraph::clear()
	{
		zGraphObjectStorage::get(*graphObj).clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnGraph::create(zPointArray(&_positions), zIntArray(&edgeConnects), bool staticGraph,int precision)
	{
		zGraphObjectStorage::get(*graphObj).create(_positions, edgeConnects,staticGraph, precision);

		if (staticGraph) setStaticContainers();
	}

	ZSPACE_INLINE void zFnGraph::create(zPointArray(&_positions), zIntArray(&edgeConnects), zVector &graphNormal, bool staticGraph)
	{

		graphNormal.normalize();

		zVector x(1, 0, 0);
		zVector sortRef = graphNormal ^ x;

		zGraphObjectStorage::get(*graphObj).create(_positions, edgeConnects, graphNormal, sortRef);

		if (staticGraph) setStaticContainers();
	}

	ZSPACE_INLINE void zFnGraph::createFromMesh(zObjectMesh &meshObj, bool excludeBoundary, bool staticGraph)
	{
		zFnMesh fnMesh(meshObj);

		vector<int>edgeConnects;
		vector<zVector> vertexPositions;

		fnMesh.getVertexPositions(vertexPositions, excludeBoundary);
		fnMesh.getEdgeData(edgeConnects, excludeBoundary);

		create(vertexPositions, edgeConnects, staticGraph);

		if (staticGraph) setStaticContainers();
	}

	ZSPACE_INLINE bool zFnGraph::addVertex(zPoint &_pos, bool checkDuplicates, zItGraphVertex &vertex)
	{
		if (checkDuplicates)
		{
			int id;
			bool chk = vertexExists(_pos, vertex);
			if (chk)	return false;

		}

		bool out = zGraphObjectStorage::get(*graphObj).addVertex(_pos);
		vertex = zItGraphVertex(*graphObj, numVertices() - 1);

		return out;
	}

	ZSPACE_INLINE bool zFnGraph::addEdges(int &v1, int &v2, bool checkDuplicates, zItGraphHalfEdge &halfEdge)
	{
		if (v1 < 0 && v1 >= numVertices()) throw std::invalid_argument(" error: index out of bounds");
		if (v2 < 0 && v2 >= numVertices()) throw std::invalid_argument(" error: index out of bounds");

		if (checkDuplicates)
		{
			int id;
			bool chk = halfEdgeExists(v1, v2, id);
			if (chk)
			{
				halfEdge = zItGraphHalfEdge(*graphObj, id);
				return false;
			}
		}

		bool out = zGraphObjectStorage::get(*graphObj).addEdges(v1, v2);

		halfEdge = zItGraphHalfEdge(*graphObj, numHalfEdges() - 2);

		return out;
	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE int zFnGraph::numVertices()
	{
		return zGraphObjectStorage::get(*graphObj).n_v;
	}

	ZSPACE_INLINE int zFnGraph::numEdges()
	{
		return zGraphObjectStorage::get(*graphObj).n_e;
	}

	ZSPACE_INLINE int zFnGraph::numHalfEdges()
	{
		return zGraphObjectStorage::get(*graphObj).n_he;
	}

	ZSPACE_INLINE bool zFnGraph::vertexExists(zPoint pos, zItGraphVertex &outVertex, int precisionfactor)
	{

		int id;
		bool chk = zGraphObjectStorage::get(*graphObj).vertexExists(pos, id, precisionfactor);

		if (chk) outVertex = zItGraphVertex(*graphObj, id);

		return chk;
	}

	ZSPACE_INLINE bool zFnGraph::halfEdgeExists(int v1, int v2, int &outHalfEdgeId)
	{
		return zGraphObjectStorage::get(*graphObj).halfEdgeExists(v1, v2, outHalfEdgeId);
	}

	ZSPACE_INLINE bool zFnGraph::halfEdgeExists(int v1, int v2, zItGraphHalfEdge &outHalfEdge)
	{
		int id;
		bool chk = halfEdgeExists(v1, v2, id);

		if (chk) outHalfEdge = zItGraphHalfEdge(*graphObj, id);

		return chk;
	}


	//--- COMPUTE METHODS 

	ZSPACE_INLINE void zFnGraph::computeEdgeColorfromVertexColor()
	{

		for (zItGraphEdge e(*graphObj); !e.end(); e++)
		{
			if (e.isActive())
			{
				int v0 = e.getHalfEdge(0).getVertex().getId();
				int v1 = e.getHalfEdge(1).getVertex().getId();

				zColor col;
				col.r = (zGraphObjectStorage::get(*graphObj).vertexColors[v0].r + zGraphObjectStorage::get(*graphObj).vertexColors[v1].r) * 0.5;
				col.g = (zGraphObjectStorage::get(*graphObj).vertexColors[v0].g + zGraphObjectStorage::get(*graphObj).vertexColors[v1].g) * 0.5;
				col.b = (zGraphObjectStorage::get(*graphObj).vertexColors[v0].b + zGraphObjectStorage::get(*graphObj).vertexColors[v1].b) * 0.5;
				col.a = (zGraphObjectStorage::get(*graphObj).vertexColors[v0].a + zGraphObjectStorage::get(*graphObj).vertexColors[v1].a) * 0.5;

				if (zGraphObjectStorage::get(*graphObj).edgeColors.size() <= e.getId()) zGraphObjectStorage::get(*graphObj).edgeColors.push_back(col);
				else zGraphObjectStorage::get(*graphObj).edgeColors[e.getId()] = col;


			}


		}

	}

	ZSPACE_INLINE void zFnGraph::computeVertexColorfromEdgeColor()
	{
		for (zItGraphVertex v(*graphObj); !v.end(); v++)
		{
			if (v.isActive())
			{
				vector<int> cEdges;
				v.getConnectedHalfEdges(cEdges);

				zColor col;
				for (int j = 0; j < cEdges.size(); j++)
				{
					col.r += zGraphObjectStorage::get(*graphObj).edgeColors[cEdges[j]].r;
					col.g += zGraphObjectStorage::get(*graphObj).edgeColors[cEdges[j]].g;
					col.b += zGraphObjectStorage::get(*graphObj).edgeColors[cEdges[j]].b;
				}

				col.r /= cEdges.size(); col.g /= cEdges.size(); col.b /= cEdges.size();

				zGraphObjectStorage::get(*graphObj).vertexColors[v.getId()] = col;

			}
		}
	}

	ZSPACE_INLINE void zFnGraph::averageVertices(int numSteps)
	{
		for (int k = 0; k < numSteps; k++)
		{
			vector<zVector> tempVertPos;

			for (zItGraphVertex v(*graphObj); !v.end(); v++)
			{
				tempVertPos.push_back(zGraphObjectStorage::get(*graphObj).vertexPositions[v.getId()]);

				if (v.isActive())
				{
					if (!v.checkValency(1))
					{
						vector<int> cVerts;

						v.getConnectedVertices(cVerts);

						for (int j = 0; j < cVerts.size(); j++)
						{
							zVector p = zGraphObjectStorage::get(*graphObj).vertexPositions[cVerts[j]];
							tempVertPos[v.getId()] += p;
						}

						tempVertPos[v.getId()] /= (cVerts.size() + 1);
					}
				}

			}

			// update position
			for (int i = 0; i < tempVertPos.size(); i++) zGraphObjectStorage::get(*graphObj).vertexPositions[i] = tempVertPos[i];
		}

	}

	ZSPACE_INLINE void zFnGraph::removeInactiveElements(zHEData type)
	{
		if (type == zVertexData || type == zEdgeData || type == zHalfEdgeData) removeInactive(type);
		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	ZSPACE_INLINE void zFnGraph::makeStatic()
	{
		setStaticContainers();
	}

	//--- SET METHODS 

	ZSPACE_INLINE void zFnGraph::setVertexPositions(zPointArray& pos)
	{
		if (pos.size() != zGraphObjectStorage::get(*graphObj).vertexPositions.size()) throw std::invalid_argument("size of position contatiner is not equal to number of graph vertices.");

		for (int i = 0; i < zGraphObjectStorage::get(*graphObj).vertexPositions.size(); i++)
		{
			zGraphObjectStorage::get(*graphObj).vertexPositions[i] = pos[i];
		}
	}

	ZSPACE_INLINE void zFnGraph::setVertexColor(zColor col, bool setEdgeColor)
	{
		zGraphObjectStorage::get(*graphObj).vertexColors.clear();
		zGraphObjectStorage::get(*graphObj).vertexColors.assign(zGraphObjectStorage::get(*graphObj).n_v, col);

		if (setEdgeColor) computeEdgeColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnGraph::setVertexColors(zColorArray& col, bool setEdgeColor)
	{
		if (zGraphObjectStorage::get(*graphObj).vertexColors.size() != zGraphObjectStorage::get(*graphObj).vertices.size())
		{
			zGraphObjectStorage::get(*graphObj).vertexColors.clear();
			for (int i = 0; i < zGraphObjectStorage::get(*graphObj).vertices.size(); i++) zGraphObjectStorage::get(*graphObj).vertexColors.push_back(zColor(1, 0, 0, 1));
		}

		if (col.size() != zGraphObjectStorage::get(*graphObj).vertexColors.size()) throw std::invalid_argument("size of color contatiner is not equal to number of graph vertices.");

		for (int i = 0; i < zGraphObjectStorage::get(*graphObj).vertexColors.size(); i++)
		{
			zGraphObjectStorage::get(*graphObj).vertexColors[i] = col[i];
		}

		if (setEdgeColor) computeEdgeColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnGraph::setEdgeColor(zColor col, bool setVertexColor)
	{

		zGraphObjectStorage::get(*graphObj).edgeColors.clear();
		zGraphObjectStorage::get(*graphObj).edgeColors.assign(zGraphObjectStorage::get(*graphObj).n_e, col);

		if (setVertexColor) computeVertexColorfromEdgeColor();

	}

	ZSPACE_INLINE void zFnGraph::setEdgeColors(zColorArray& col, bool setVertexColor)
	{
		if (col.size() != zGraphObjectStorage::get(*graphObj).edgeColors.size()) throw std::invalid_argument("size of color contatiner is not equal to number of graph half edges.");

		for (int i = 0; i < zGraphObjectStorage::get(*graphObj).edgeColors.size(); i++)
		{
			zGraphObjectStorage::get(*graphObj).edgeColors[i] = col[i];
		}

		if (setVertexColor) computeVertexColorfromEdgeColor();
	}

	ZSPACE_INLINE void zFnGraph::setEdgeWeight(double wt)
	{
		zGraphObjectStorage::get(*graphObj).edgeWeights.clear();
		zGraphObjectStorage::get(*graphObj).edgeWeights.assign(zGraphObjectStorage::get(*graphObj).n_e, wt);

	}

	ZSPACE_INLINE void zFnGraph::setEdgeWeights(zDoubleArray& wt)
	{
		if (wt.size() != zGraphObjectStorage::get(*graphObj).edgeColors.size()) throw std::invalid_argument("size of wt contatiner is not equal to number of mesh half edges.");

		for (int i = 0; i < zGraphObjectStorage::get(*graphObj).edgeWeights.size(); i++)
		{
			zGraphObjectStorage::get(*graphObj).edgeWeights[i] = wt[i];
		}
	}

	//--- GET METHODS 

	ZSPACE_INLINE void zFnGraph::getVertexPositions(zPointArray& pos)
	{
		pos = zGraphObjectStorage::get(*graphObj).vertexPositions;
	}

	ZSPACE_INLINE zPoint* zFnGraph::getRawVertexPositions()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zGraphObjectStorage::get(*graphObj).vertexPositions[0];
	}

	ZSPACE_INLINE void zFnGraph::getVertexColors(zColorArray& col)
	{
		col = zGraphObjectStorage::get(*graphObj).vertexColors;
	}

	ZSPACE_INLINE void zFnGraph::getVertexWeights(zDoubleArray& weights)
	{
		weights = zGraphObjectStorage::get(*graphObj).vertexWeights;
	}

	ZSPACE_INLINE zColor* zFnGraph::getRawVertexColors()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zGraphObjectStorage::get(*graphObj).vertexColors[0];
	}

	ZSPACE_INLINE void zFnGraph::getEdgeColors(zColorArray& col)
	{
		col = zGraphObjectStorage::get(*graphObj).edgeColors;
	}

	ZSPACE_INLINE void zFnGraph::getEdgeWeights(zDoubleArray& weights)
	{
		weights = zGraphObjectStorage::get(*graphObj).edgeWeights;
	}

	ZSPACE_INLINE zColor* zFnGraph::getRawEdgeColors()
	{
		if (numEdges() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zGraphObjectStorage::get(*graphObj).edgeColors[0];
	}

	ZSPACE_INLINE zPoint zFnGraph::getCenter()
	{
		zPoint out;

		for (int i = 0; i < zGraphObjectStorage::get(*graphObj).vertexPositions.size(); i++)
		{
			out += zGraphObjectStorage::get(*graphObj).vertexPositions[i];
		}

		out /= zGraphObjectStorage::get(*graphObj).vertexPositions.size();

		return out;

	}

	ZSPACE_INLINE void zFnGraph::getCenters(zHEData type, zPointArray &centers)
	{
		// graph Edge 
		if (type == zHalfEdgeData)
		{

			centers.clear();

			for (zItGraphHalfEdge he(*graphObj); !he.end(); he++)
			{
				if (he.isActive())
				{
					centers.push_back(he.getCenter());
				}
				else
				{
					centers.push_back(zVector());

				}
			}

		}
		else if (type == zEdgeData)
		{

			centers.clear();

			for (zItGraphEdge e(*graphObj); !e.end(); e++)
			{
				if (e.isActive())
				{
					centers.push_back(e.getCenter());
				}
				else
				{
					centers.push_back(zVector());

				}
			}

		}

		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	ZSPACE_INLINE double zFnGraph::getHalfEdgeLengths(zDoubleArray &halfEdgeLengths)
	{
		double total = 0.0;


		halfEdgeLengths.clear();

		for (zItGraphEdge e(*graphObj); !e.end(); e++)
		{
			if (e.isActive())
			{
				double e_len = e.getLength();

				halfEdgeLengths.push_back(e_len);
				halfEdgeLengths.push_back(e_len);

				total += e_len;
			}
			else
			{
				halfEdgeLengths.push_back(0);
				halfEdgeLengths.push_back(0);
			}
		}

		return total;
	}

	ZSPACE_INLINE double zFnGraph::getEdgeLengths(zDoubleArray &edgeLengths)
	{
		double total = 0.0;


		edgeLengths.clear();

		for (zItGraphEdge e(*graphObj); !e.end(); e++)
		{
			if (e.isActive())
			{
				double e_len = e.getLength();
				edgeLengths.push_back(e_len);
				total += e_len;
			}
			else
			{
				edgeLengths.push_back(0);
			}
		}

		return total;
	}

	ZSPACE_INLINE void zFnGraph::getEdgeData(zIntArray &edgeConnects)
	{
		edgeConnects.clear();

		for (zItGraphEdge e(*graphObj); !e.end(); e++)
		{
			edgeConnects.push_back(e.getHalfEdge(0).getVertex().getId());
			edgeConnects.push_back(e.getHalfEdge(1).getVertex().getId());
		}
	}

	ZSPACE_INLINE zObjectGraph zFnGraph::getDuplicate(bool planarGraph, zVector graphNormal)
	{
		zObjectGraph out;

		if (numVertices() != zGraphObjectStorage::get(*graphObj).vertices.size()) removeInactiveElements(zVertexData);
		if (numEdges() != zGraphObjectStorage::get(*graphObj).edges.size()) removeInactiveElements(zEdgeData);

		vector<zVector> positions;
		vector<int> edgeConnects;


		positions = zGraphObjectStorage::get(*graphObj).vertexPositions;
		getEdgeData(edgeConnects);


		if (planarGraph)
		{
			graphNormal.normalize();

			zVector x(1, 0, 0);
			zVector sortRef = graphNormal ^ x;

			zGraphObjectStorage::get(out).create(positions, edgeConnects, graphNormal, sortRef);
		}
		else zGraphObjectStorage::get(out).create(positions, edgeConnects);

		zGraphObjectStorage::get(out).vertexColors = zGraphObjectStorage::get(*graphObj).vertexColors;
		zGraphObjectStorage::get(out).edgeColors = zGraphObjectStorage::get(*graphObj).edgeColors;

		return out;
	}

	ZSPACE_INLINE void zFnGraph::getGraphMesh(zObjectMesh &out, double width, zVector graphNormal)
	{

		vector<zVector>positions;
		vector<int> polyConnects;
		vector<int> polyCounts;

		if (numVertices() != zGraphObjectStorage::get(*graphObj).vertices.size()) removeInactiveElements(zVertexData);
		if (numEdges() != zGraphObjectStorage::get(*graphObj).edges.size()) removeInactiveElements(zEdgeData);

		positions = zGraphObjectStorage::get(*graphObj).vertexPositions;

		vector<vector<int>> edgeVertices;
		for (zItGraphHalfEdge he(*graphObj); !he.end(); he++)
		{

			int v0 = he.getStartVertex().getId();
			int v1 = he.getVertex().getId();

			vector<int> temp;
			temp.push_back(v0);
			temp.push_back(v1);
			temp.push_back(-1);
			temp.push_back(-1);

			edgeVertices.push_back(temp);
		}

		for (zItGraphVertex v(*graphObj); !v.end(); v++)
		{
			vector<zItGraphHalfEdge> cEdges;
			v.getConnectedHalfEdges(cEdges);

			if (cEdges.size() == 1)
			{

				int currentId = cEdges[0].getId();
				int prevId = cEdges[0].getPrev().getId();

				zVector e_current = cEdges[0].getVector();
				e_current.normalize();

				zVector e_prev = cEdges[0].getPrev().getVector();
				e_prev.normalize();

				zVector n_current = graphNormal ^ e_current;
				n_current.normalize();

				zVector n_prev = graphNormal ^ e_prev;
				n_prev.normalize();


				double w = width * 0.5;

				edgeVertices[currentId][3] = positions.size();
				positions.push_back(v.getPosition() + (n_current * w));

				edgeVertices[prevId][2] = positions.size();
				positions.push_back(v.getPosition() + (n_prev * w));

			}

			else
			{
				for (int j = 0; j < cEdges.size(); j++)
				{
					int currentId = cEdges[j].getId();
					int prevId = cEdges[j].getPrev().getId();


					zVector e_current = cEdges[j].getVector();
					e_current.normalize();

					zVector e_prev = cEdges[j].getPrev().getVector();
					e_prev.normalize();

					zVector n_current = graphNormal ^ e_current;
					n_current.normalize();

					zVector n_prev = graphNormal ^ e_prev;
					n_prev.normalize();

					zVector norm = (n_current + n_prev) * 0.5;
					norm.normalize();


					double w = width * 0.5;

					zVector a0 = cEdges[j].getStartVertex().getPosition() + (n_current * w);
					zVector a1 = cEdges[j].getCenter() + (n_current * w);

					zVector b0 = cEdges[j].getStartVertex().getPosition() + (n_prev * w);
					zVector b1 = cEdges[j].getPrev().getCenter() + (n_prev * w);

					double uA, uB;
					bool intersect = zGraphObjectStorage::get(*graphObj).coreUtils.line_lineClosestPoints(a0, a1, b0, b1, uA, uB);


					edgeVertices[currentId][3] = positions.size();
					edgeVertices[prevId][2] = positions.size();



					if (!intersect) positions.push_back(v.getPosition() + (norm * w));
					else
					{
						if (uA >= uB)
						{
							zVector dir = a1 - a0;
							double len = dir.length();
							dir.normalize();

							if (uA < 0) dir *= -1;
							positions.push_back(a0 + dir * len * uA);
						}
						else
						{
							zVector dir = b1 - b0;
							double len = dir.length();
							dir.normalize();

							if (uB < 0) dir *= -1;

							positions.push_back(b0 + dir * len * uB);
						}
					}
				}
			}

		}

		for (int i = 0; i < edgeVertices.size(); i++)
		{

			for (int j = 0; j < edgeVertices[i].size(); j++)
			{
				polyConnects.push_back(edgeVertices[i][j]);
			}

			polyCounts.push_back(edgeVertices[i].size());

		}




		// mesh
		if (positions.size() > 0)
		{
			zFnMesh fnMesh(out);
			fnMesh.create(positions, polyCounts, polyConnects);
		}



	}

	ZSPACE_INLINE void zFnGraph::getGraphEccentricityCenter(zItGraphVertexArray & outV)
	{
		const int N = numVertices();	// number of nodes in graph
		const int INF = 99999;
		MatrixXi d(N,N);				// distances between nodes
		VectorXi e(N);					// eccentricity of nodes
		set<int> c;						// center of graph
		int rad = INF;					// radius of graph
		int diam = 0;					// diamater of graph

		zIntArray boundaryVerts;
		for (zItGraphVertex v(*graphObj); !v.end(); v++)
			if (v.checkValency(1)) boundaryVerts.push_back(v.getId());
		
		d.setConstant(INF);
		e.setZero();

		for (zItGraphHalfEdge he(*graphObj); !he.end(); he++)
		{
			d(he.getStartVertex().getId(), he.getVertex().getId()) = 1;
			d(he.getStartVertex().getId(), he.getStartVertex().getId()) = 0;
		}

		// Floyd-Warshall's algorithm
		for (int k = 0; k < N; k++)
			for (int j = 0; j < N; j++)
				for (int i = 0; i < N; i++)
					d(i,j) = coreUtils.zMin(d(i, j), d(i, k) + d(k, j));

		// Counting values of eccentricity
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				e(i) = coreUtils.zMax(e(i), d(i,j));

		for (int i = 0; i < N; i++) 
		{
			rad = coreUtils.zMin(rad, e(i));
			diam = coreUtils.zMax(diam, e(i));
		}


		for (int i = 0; i < N; i++) 
			if (e[i] == rad) 		
				c.insert(i);


		zItGraphVertex v(*graphObj);
		int maxScore = 0;

		//printf("\n cen:");
		for (auto id : c)
		{
			zItGraphVertex v1(*graphObj, id);
			outV.push_back(v1);

			double score = 0;

			for (int j = 0; j < boundaryVerts.size(); j++)
				score += d(v1.getId(), boundaryVerts[j]);

			if (score > maxScore)
			{
				maxScore = score;
				v = v1;
			}

			//printf("  %i ", v1.getId());
		}

		
		//outV.push_back(v);
	}

	//---- TOPOLOGY MODIFIER METHODS

	ZSPACE_INLINE zItGraphVertex zFnGraph::splitEdge(zItGraphEdge &edge, double edgeFactor)
	{
		int edgeId = edge.getId();

		zItGraphHalfEdge he = edge.getHalfEdge(0);
		zItGraphHalfEdge heS = edge.getHalfEdge(1);

		zItGraphHalfEdge he_next = he.getNext();
		zItGraphHalfEdge he_prev = he.getPrev();

		zItGraphHalfEdge heS_next = heS.getNext();
		zItGraphHalfEdge heS_prev = heS.getPrev();


		zVector edgeDir = he.getVector();
		double  edgeLength = edgeDir.length();
		edgeDir.normalize();

		zVector newVertPos = he.getStartVertex().getPosition() + edgeDir * edgeFactor * edgeLength;

		int numOriginalVertices = numVertices();

		// check if vertex exists if not add new vertex
		zItGraphVertex newVertex;
		addVertex(newVertPos, false, newVertex);

		if (newVertex.getId() >= numOriginalVertices)
		{
			// remove from halfEdge vertices map
			removeFromHalfEdgesMap(he);

			// add new edges
			int v1 = newVertex.getId();
			int v2 = he.getVertex().getId();

			bool v2_val1 = he.getVertex().checkValency(1);

			zItGraphHalfEdge newHe;
			bool edgesResize = addEdges(v1, v2, false, newHe);

			int newHeId = newHe.getId();

			// recompute iterators if resize is true
			if (edgesResize)
			{
				edge = zItGraphEdge(*graphObj, edgeId);

				he = edge.getHalfEdge(0);
				heS = edge.getHalfEdge(1);

				he_next = he.getNext();
				he_prev = he.getPrev();

				heS_next = heS.getNext();
				heS_prev = heS.getPrev();

				newHe = zItGraphHalfEdge(*graphObj, newHeId);
			}

			zItGraphHalfEdge newHeS = newHe.getSym();

			// update vertex pointers
			newVertex.setHalfEdge(newHe);
			he.getVertex().setHalfEdge(newHeS);

			//// update pointers
			he.setVertex(newVertex);		// current hedge vertex pointer updated to new added vertex

			newHeS.setNext(heS);			// new added symmetry hedge next pointer to point to the symmetry of current hedge
			
			if (!v2_val1) newHeS.setPrev(heS_prev);
			else newHeS.setPrev(newHe);
			
			newHe.setPrev(he);				// new added  hedge prev pointer to point to the current hedge
			if (!v2_val1) newHe.setNext(he_next);

			// update verticesEdge map
			addToHalfEdgesMap(he);
		}

		return newVertex;
	}

	//---- TRANSFORM METHODS OVERRIDES

	ZSPACE_INLINE void zFnGraph::setTransform(zTransform &inTransform, bool decompose, bool updatePositions)
	{
		if (updatePositions)
		{
			zTransformationMatrix to;
			to.setTransform(inTransform, decompose);

			zTransform transMat = graphObj->transformationMatrix.getToMatrix(to);
			transformObject(transMat);

			graphObj->transformationMatrix.setTransform(inTransform);

			// update pivot values of object transformation matrix
			zVector p = graphObj->transformationMatrix.getPivot();
			p = p * transMat;
			setPivot(p);

		}
		else
		{
			graphObj->transformationMatrix.setTransform(inTransform, decompose);

			zVector p = graphObj->transformationMatrix.getO();
			setPivot(p);

		}

	}

	ZSPACE_INLINE void zFnGraph::setScale(zFloat4 &scale)
	{
		// get  inverse pivot translations
		zTransform invScalemat = graphObj->transformationMatrix.asInverseScaleTransformMatrix();

		// set scale values of object transformation matrix
		graphObj->transformationMatrix.setScale(scale);

		// get new scale transformation matrix
		zTransform scaleMat = graphObj->transformationMatrix.asScaleTransformMatrix();

		// compute total transformation
		zTransform transMat = invScalemat * scaleMat;

		// transform object
		transformObject(transMat);
	}

	ZSPACE_INLINE void zFnGraph::setRotation(zFloat4 &rotation, bool appendRotations)
	{
		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = graphObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = graphObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = graphObj->transformationMatrix;
		to.setRotation(rotation, appendRotations);
		zTransform toMat = graphObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set rotation values of object transformation matrix
		graphObj->transformationMatrix.setRotation(rotation, appendRotations);;
	}

	ZSPACE_INLINE void zFnGraph::setTranslation(zVector &translation, bool appendTranslations)
	{
		// get vector as zDouble3
		zFloat4 t;
		translation.getComponents(t);

		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = graphObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = graphObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = graphObj->transformationMatrix;
		to.setTranslation(t, appendTranslations);
		zTransform toMat = graphObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set translation values of object transformation matrix
		graphObj->transformationMatrix.setTranslation(t, appendTranslations);;

		// update pivot values of object transformation matrix
		zVector p = graphObj->transformationMatrix.getPivot();
		p = p * transMat;
		setPivot(p);

	}

	ZSPACE_INLINE void zFnGraph::setPivot(zVector &pivot)
	{
		// get vector as zDouble3
		zFloat4 p;
		pivot.getComponents(p);

		// set pivot values of object transformation matrix
		graphObj->transformationMatrix.setPivot(p);
	}

	ZSPACE_INLINE void zFnGraph::getTransform(zTransform &transform)
	{
		transform = graphObj->transformationMatrix.asMatrix();
	}

	//---- PROTECTED OVERRIDE METHODS

	ZSPACE_INLINE void zFnGraph::transformObject(zTransform &transform)
	{

		if (numVertices() == 0) return;


		zVector* pos = getRawVertexPositions();

		for (int i = 0; i < numVertices(); i++)
		{

			zVector newPos = pos[i] * transform;
			pos[i] = newPos;
		}

	}

	//---- PROTECTED REMOVE INACTIVE METHODS

	ZSPACE_INLINE void zFnGraph::removeInactive(zHEData type)
	{
		//  Vertex
		if (type == zVertexData)
		{
			zItVertex v = zGraphObjectStorage::get(*graphObj).vertices.begin();

			while (v != zGraphObjectStorage::get(*graphObj).vertices.end())
			{
				bool active = v->isActive();

				if (!active)
				{
					zGraphObjectStorage::get(*graphObj).vertices.erase(v++);

					zGraphObjectStorage::get(*graphObj).n_v--;
				}
			}

			zGraphObjectStorage::get(*graphObj).indexElements(zVertexData);

			printf("\n removed inactive vertices. ");

		}

		//  Edge
		else if (type == zEdgeData || type == zHalfEdgeData)
		{

			zItHalfEdge he = zGraphObjectStorage::get(*graphObj).halfEdges.begin();

			while (he != zGraphObjectStorage::get(*graphObj).halfEdges.end())
			{
				bool active = he->isActive();

				if (!active)
				{
					zGraphObjectStorage::get(*graphObj).halfEdges.erase(he++);

					zGraphObjectStorage::get(*graphObj).n_he--;
				}
			}

			zItEdge e = zGraphObjectStorage::get(*graphObj).edges.begin();

			while (e != zGraphObjectStorage::get(*graphObj).edges.end())
			{
				bool active = e->isActive();

				if (!active)
				{
					zGraphObjectStorage::get(*graphObj).edges.erase(e++);

					zGraphObjectStorage::get(*graphObj).n_e--;
				}
			}

			printf("\n removed inactive edges. ");

			zGraphObjectStorage::get(*graphObj).indexElements(zHalfEdgeData);
			zGraphObjectStorage::get(*graphObj).indexElements(zEdgeData);

		}

		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	//---- PROTECTED FACTORY METHODS

	//---- PRIVATE METHODS

	ZSPACE_INLINE void zFnGraph::setStaticContainers()
	{
		zGraphObjectStorage::get(*graphObj).staticGeometry = true;

		vector<vector<int>> edgeVerts;

		for (zItGraphEdge e(*graphObj); !e.end(); e++)
		{
			vector<int> verts;
			e.getVertices(verts);

			edgeVerts.push_back(verts);
		}

		zGraphObjectStorage::get(*graphObj).setStaticEdgeVertices(edgeVerts);
	}

	//---- PRIVATE DEACTIVATE AND REMOVE METHODS

	ZSPACE_INLINE void zFnGraph::addToHalfEdgesMap(zItGraphHalfEdge &he)
	{
		zGraphObjectStorage::get(*graphObj).addToHalfEdgesMap(he.getStartVertex().getId(), he.getVertex().getId(), he.getId());
	}

	ZSPACE_INLINE void zFnGraph::removeFromHalfEdgesMap(zItGraphHalfEdge &he)
	{
		zGraphObjectStorage::get(*graphObj).removeFromHalfEdgesMap(he.getStartVertex().getId(), he.getVertex().getId());
	}
}
