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
		zPointArray positions = zGraphObjectStorage::read(*graphObj).positions;
		coreUtils.getBounds(positions, minBB, maxBB);
	}

	ZSPACE_INLINE void zFnGraph::clear()
	{
		zGraphObjectStorage::edit(*graphObj).clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnGraph::create(zPointArray(&_positions), zIntArray(&edgeConnects), int precision)
	{
		zGraphObjectStorage::set(*graphObj, _positions, edgeConnects);
	}

	ZSPACE_INLINE void zFnGraph::create(zPointArray(&_positions), zIntArray(&edgeConnects), zVector &graphNormal)
	{

		graphNormal.normalize();

		zVector x(1, 0, 0);
		zVector sortRef = graphNormal ^ x;

		zGraphObjectStorage::set(*graphObj, _positions, edgeConnects);
	}

	ZSPACE_INLINE void zFnGraph::createFromMesh(zObjectMesh &meshObj, bool excludeBoundary)
	{
		zFnMesh fnMesh(meshObj);

		vector<int>edgeConnects;
		vector<zVector> vertexPositions;

		fnMesh.getVertexPositions(vertexPositions, excludeBoundary);
		fnMesh.getEdgeData(edgeConnects, excludeBoundary);

		create(vertexPositions, edgeConnects);
	}

	ZSPACE_INLINE bool zFnGraph::addVertex(zPoint &_pos, bool checkDuplicates, zItGraphVertex &vertex)
	{
		if (checkDuplicates)
		{
			int id;
			bool chk = vertexExists(_pos, vertex);
			if (chk)	return false;

		}

		bool out = zGraphObjectStorage::edit(*graphObj).addVertex(_pos);
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

		int edgeId = -1;
		bool out = zGraphObjectStorage::edit(*graphObj).addEdge(v1, v2, false, &edgeId);

		halfEdge = zItGraphHalfEdge(*graphObj, numHalfEdges() - 2);

		return out;
	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE int zFnGraph::numVertices()
	{
		return zGraphObjectStorage::read(*graphObj).numVertices();
	}

	ZSPACE_INLINE int zFnGraph::numEdges()
	{
		return zGraphObjectStorage::read(*graphObj).numEdges();
	}

	ZSPACE_INLINE bool zFnGraph::edgeExists(int v1, int v2, int &outEdgeId)
	{
		return zGraphObjectStorage::read(*graphObj).edgeExists(v1, v2, outEdgeId);
	}

	ZSPACE_INLINE int zFnGraph::numHalfEdges()
	{
		return zGraphObjectStorage::read(*graphObj).numHalfEdges();
	}

	ZSPACE_INLINE bool zFnGraph::vertexExists(zPoint pos, zItGraphVertex &outVertex, int precisionfactor)
	{

		int id;
		bool chk = zGraphObjectStorage::read(*graphObj).vertexExists(pos, id, precisionfactor);

		if (chk) outVertex = zItGraphVertex(*graphObj, id);

		return chk;
	}

	ZSPACE_INLINE bool zFnGraph::halfEdgeExists(int v1, int v2, int &outHalfEdgeId)
	{
		int edgeId = -1;
		if (!zGraphObjectStorage::read(*graphObj).edgeExists(v1, v2, edgeId)) return false;

		const auto& data = zGraphObjectStorage::read(*graphObj);
		const int a = data.edgeVertexIndices[edgeId * 2];
		outHalfEdgeId = (a == v1) ? edgeId * 2 : edgeId * 2 + 1;
		return true;
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
		auto& data = zGraphObjectStorage::edit(*graphObj);
		if (data.edgeColors.size() != data.numEdges()) data.edgeColors.assign(data.numEdges(), zColor(0, 0, 0, 1));
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			int v0 = data.edgeVertexIndices[edgeId * 2];
			int v1 = data.edgeVertexIndices[edgeId * 2 + 1];

			zColor col;
			col.r = (data.vertexColors[v0].r + data.vertexColors[v1].r) * 0.5;
			col.g = (data.vertexColors[v0].g + data.vertexColors[v1].g) * 0.5;
			col.b = (data.vertexColors[v0].b + data.vertexColors[v1].b) * 0.5;
			col.a = (data.vertexColors[v0].a + data.vertexColors[v1].a) * 0.5;
			data.edgeColors[edgeId] = col;
		}

	}

	ZSPACE_INLINE void zFnGraph::computeVertexColorfromEdgeColor()
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		if (data.edgeColors.size() != data.numEdges()) data.edgeColors.assign(data.numEdges(), zColor(0, 0, 0, 1));
		if (data.vertexColors.size() != data.numVertices()) data.vertexColors.assign(data.numVertices(), zColor(0, 0, 0, 1));

		vector<zColor> colorSums(data.numVertices(), zColor(0, 0, 0, 0));
		vector<int> colorCounts(data.numVertices(), 0);

		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			const zColor& edgeColor = data.edgeColors[edgeId];

			colorSums[v0].r += edgeColor.r;
			colorSums[v0].g += edgeColor.g;
			colorSums[v0].b += edgeColor.b;
			colorSums[v0].a += edgeColor.a;
			colorCounts[v0]++;

			colorSums[v1].r += edgeColor.r;
			colorSums[v1].g += edgeColor.g;
			colorSums[v1].b += edgeColor.b;
			colorSums[v1].a += edgeColor.a;
			colorCounts[v1]++;
		}

		for (int vertexId = 0; vertexId < data.numVertices(); ++vertexId)
		{
			if (colorCounts[vertexId] == 0) continue;

			colorSums[vertexId].r /= colorCounts[vertexId];
			colorSums[vertexId].g /= colorCounts[vertexId];
			colorSums[vertexId].b /= colorCounts[vertexId];
			colorSums[vertexId].a /= colorCounts[vertexId];
			data.vertexColors[vertexId] = colorSums[vertexId];
		}
	}

	ZSPACE_INLINE void zFnGraph::averageVertices(int numSteps)
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		vector<zIntArray> adjacency(data.numVertices());

		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];

			adjacency[v0].push_back(v1);
			adjacency[v1].push_back(v0);
		}

		for (int k = 0; k < numSteps; k++)
		{
			zPointArray tempVertPos = data.positions;

			for (int vertexId = 0; vertexId < data.numVertices(); ++vertexId)
			{
				if (adjacency[vertexId].size() == 1) continue;

				zPoint avg = data.positions[vertexId];
				for (int j = 0; j < adjacency[vertexId].size(); j++)
				{
					avg += data.positions[adjacency[vertexId][j]];
				}

				avg /= (adjacency[vertexId].size() + 1);
				tempVertPos[vertexId] = avg;
			}

			data.positions = tempVertPos;
		}

	}

	ZSPACE_INLINE void zFnGraph::removeInactiveElements(zHEData type)
	{
		if (type == zVertexData || type == zEdgeData || type == zHalfEdgeData) removeInactive(type);
		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	//--- SET METHODS 

	ZSPACE_INLINE void zFnGraph::setVertexPositions(zPointArray& pos)
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		if (pos.size() != data.positions.size()) throw std::invalid_argument("size of position contatiner is not equal to number of graph vertices.");

		for (int i = 0; i < data.positions.size(); i++)
		{
			data.positions[i] = pos[i];
		}
	}

	ZSPACE_INLINE void zFnGraph::setVertexColor(zColor col, bool setEdgeColor)
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		data.vertexColors.clear();
		data.vertexColors.assign(data.numVertices(), col);

		if (setEdgeColor) computeEdgeColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnGraph::setVertexColors(zColorArray& col, bool setEdgeColor)
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		if (data.vertexColors.size() != data.positions.size())
		{
			data.vertexColors.clear();
			for (int i = 0; i < data.positions.size(); i++) data.vertexColors.push_back(zColor(1, 0, 0, 1));
		}

		if (col.size() != data.vertexColors.size()) throw std::invalid_argument("size of color contatiner is not equal to number of graph vertices.");

		for (int i = 0; i < data.vertexColors.size(); i++)
		{
			data.vertexColors[i] = col[i];
		}

		if (setEdgeColor) computeEdgeColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnGraph::setEdgeColor(zColor col, bool setVertexColor)
	{

		auto& data = zGraphObjectStorage::edit(*graphObj);
		data.edgeColors.clear();
		data.edgeColors.assign(data.numEdges(), col);

		if (setVertexColor) computeVertexColorfromEdgeColor();

	}

	ZSPACE_INLINE void zFnGraph::setEdgeColors(zColorArray& col, bool setVertexColor)
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		if (col.size() != data.edgeColors.size()) throw std::invalid_argument("size of color contatiner is not equal to number of graph edges.");

		for (int i = 0; i < data.edgeColors.size(); i++)
		{
			data.edgeColors[i] = col[i];
		}

		if (setVertexColor) computeVertexColorfromEdgeColor();
	}

	ZSPACE_INLINE void zFnGraph::setEdgeWeight(double wt)
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		data.edgeWeights.clear();
		data.edgeWeights.assign(data.numEdges(), wt);

	}

	ZSPACE_INLINE void zFnGraph::setEdgeWeights(zDoubleArray& wt)
	{
		auto& data = zGraphObjectStorage::edit(*graphObj);
		if (wt.size() != data.edgeWeights.size()) throw std::invalid_argument("size of wt contatiner is not equal to number of graph edges.");

		for (int i = 0; i < data.edgeWeights.size(); i++)
		{
			data.edgeWeights[i] = wt[i];
		}
	}

	//--- GET METHODS 

	ZSPACE_INLINE void zFnGraph::getVertexPositions(zPointArray& pos)
	{
		pos = zGraphObjectStorage::read(*graphObj).positions;
	}

	ZSPACE_INLINE zPoint* zFnGraph::getRawVertexPositions()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zGraphObjectStorage::edit(*graphObj).positions[0];
	}

	ZSPACE_INLINE void zFnGraph::getVertexColors(zColorArray& col)
	{
		col = zGraphObjectStorage::read(*graphObj).vertexColors;
	}

	ZSPACE_INLINE void zFnGraph::getVertexWeights(zDoubleArray& weights)
	{
		weights = zGraphObjectStorage::read(*graphObj).vertexWeights;
	}

	ZSPACE_INLINE zColor* zFnGraph::getRawVertexColors()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zGraphObjectStorage::edit(*graphObj).vertexColors[0];
	}

	ZSPACE_INLINE void zFnGraph::getEdgeColors(zColorArray& col)
	{
		col = zGraphObjectStorage::read(*graphObj).edgeColors;
	}

	ZSPACE_INLINE void zFnGraph::getEdgeWeights(zDoubleArray& weights)
	{
		weights = zGraphObjectStorage::read(*graphObj).edgeWeights;
	}

	ZSPACE_INLINE zColor* zFnGraph::getRawEdgeColors()
	{
		if (numEdges() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zGraphObjectStorage::edit(*graphObj).edgeColors[0];
	}

	ZSPACE_INLINE zPoint zFnGraph::getCenter()
	{
		zPoint out;
		const auto& data = zGraphObjectStorage::read(*graphObj);

		for (int i = 0; i < data.positions.size(); i++)
		{
			out += data.positions[i];
		}

		out /= data.positions.size();

		return out;

	}

	ZSPACE_INLINE void zFnGraph::getCenters(zHEData type, zPointArray &centers)
	{
		// graph Edge 
		if (type == zHalfEdgeData)
		{

			centers.clear();
			const auto& data = zGraphObjectStorage::read(*graphObj);

			for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
			{
				const int v0 = data.edgeVertexIndices[edgeId * 2];
				const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
				zPoint p0 = data.positions[v0];
				zPoint p1 = data.positions[v1];
				zPoint center = (p0 + p1) * 0.5;

				centers.push_back(center);
				centers.push_back(center);
			}

		}
		else if (type == zEdgeData)
		{

			centers.clear();
			const auto& data = zGraphObjectStorage::read(*graphObj);
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			zPoint p0 = data.positions[v0];
			zPoint p1 = data.positions[v1];
			centers.push_back((p0 + p1) * 0.5);
		}

		}

		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	ZSPACE_INLINE double zFnGraph::getHalfEdgeLengths(zDoubleArray &halfEdgeLengths)
	{
		double total = 0.0;


		halfEdgeLengths.clear();

		const auto& data = zGraphObjectStorage::read(*graphObj);
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			zPoint p0 = data.positions[v0];
			zPoint p1 = data.positions[v1];
			double e_len = (p1 - p0).length();

			halfEdgeLengths.push_back(e_len);
			halfEdgeLengths.push_back(e_len);

			total += e_len;
		}

		return total;
	}

	ZSPACE_INLINE double zFnGraph::getEdgeLengths(zDoubleArray &edgeLengths)
	{
		double total = 0.0;


		edgeLengths.clear();

		const auto& data = zGraphObjectStorage::read(*graphObj);
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			zPoint p0 = data.positions[v0];
			zPoint p1 = data.positions[v1];
			double e_len = (p1 - p0).length();
			edgeLengths.push_back(e_len);
			total += e_len;
		}

		return total;
	}

	ZSPACE_INLINE void zFnGraph::getEdgeData(zIntArray &edgeConnects)
	{
		edgeConnects = zGraphObjectStorage::read(*graphObj).edgeVertexIndices;
	}

	ZSPACE_INLINE zObjectGraph zFnGraph::getDuplicate(bool planarGraph, zVector graphNormal)
	{
		zObjectGraph out;

		vector<zVector> positions;
		vector<int> edgeConnects;

		const auto& data = zGraphObjectStorage::read(*graphObj);
		positions = data.positions;
		getEdgeData(edgeConnects);

		zGraphObjectStorage::set(out, positions, edgeConnects);

		auto& outData = zGraphObjectStorage::edit(out);
		outData.vertexColors = data.vertexColors;
		outData.edgeColors = data.edgeColors;
		outData.vertexWeights = data.vertexWeights;
		outData.edgeWeights = data.edgeWeights;

		return out;
	}

	ZSPACE_INLINE void zFnGraph::getGraphMesh(zObjectMesh &out, double width, zVector graphNormal)
	{

		vector<zVector>positions;
		vector<int> polyConnects;
		vector<int> polyCounts;

		positions = zGraphObjectStorage::read(*graphObj).positions;

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
		const auto& data = zGraphObjectStorage::read(*graphObj);
		const int N = data.numVertices();	// number of nodes in graph
		outV.clear();
		if (N == 0) return;

		const int INF = 99999;
		MatrixXi d(N,N);				// distances between nodes
		VectorXi e(N);					// eccentricity of nodes
		set<int> c;						// center of graph
		int rad = INF;					// radius of graph
		int diam = 0;					// diamater of graph

		vector<zIntArray> adjacency(N);
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];

			adjacency[v0].push_back(v1);
			adjacency[v1].push_back(v0);
		}

		zIntArray boundaryVerts;
		for (int vertexId = 0; vertexId < N; vertexId++)
			if (adjacency[vertexId].size() == 1) boundaryVerts.push_back(vertexId);
		
		d.setConstant(INF);
		e.setZero();

		for (int i = 0; i < N; i++) d(i, i) = 0;

		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];

			d(v0, v1) = 1;
			d(v1, v0) = 1;
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


		auto& data = zGraphObjectStorage::edit(*graphObj);

		for (int i = 0; i < data.positions.size(); i++)
		{

			zVector newPos = data.positions[i] * transform;
			data.positions[i] = newPos;
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
