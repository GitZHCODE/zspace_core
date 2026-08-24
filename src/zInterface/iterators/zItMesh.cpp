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


#include<zspace/zInterface/iterators/zItMesh.h>
#include <src/zInterface/objects/zMeshObjectStorage.h>
#include <src/zCore/geometry/detail/zMeshStorage.h>
#include <unordered_set>

//---- ZIT_MESH_VERTEX ------------------------------------------------------------------------------

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zItMeshVertex::zItMeshVertex()
	{
		meshObj = nullptr;
		index = 0;
	}

	ZSPACE_INLINE zItMeshVertex::zItMeshVertex(zObjectMesh &_meshObj)
	{
		meshObj = &_meshObj;
		index = 0;
	}

	ZSPACE_INLINE zItMeshVertex::zItMeshVertex(zObjectMesh &_meshObj, int _index)
	{
		meshObj = &_meshObj;
		if (_index < 0 || _index >= zMeshObjectStorage::read(*meshObj).numVertices()) throw std::invalid_argument(" error: index out of bounds");
		index = _index;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zItMeshVertex::begin()
	{
		index = 0;
	}

	ZSPACE_INLINE void zItMeshVertex::operator++(int)
	{
		index++;
	}

	ZSPACE_INLINE void zItMeshVertex::operator--(int)
	{
		index--;
	}

	ZSPACE_INLINE bool zItMeshVertex::end()
	{
		return !meshObj || index >= zMeshObjectStorage::read(*meshObj).numVertices();
	}

	ZSPACE_INLINE void zItMeshVertex::reset()
	{
		index = 0;
	}

	ZSPACE_INLINE int zItMeshVertex::size()
	{
		if (!meshObj) return 0;
		return zMeshObjectStorage::read(*meshObj).numVertices();
	}

	ZSPACE_INLINE void zItMeshVertex::deactivate()
	{
		throw std::logic_error("Mesh topology editing is not supported by face-list iterators.");
	}

	//---- TOPOLOGY QUERY METHODS

	ZSPACE_INLINE void zItMeshVertex::getConnectedHalfEdges(zItMeshHalfEdgeArray& halfedges)
	{
		if (zMeshObjectStorage::get(*meshObj).vertices[index].getHalfEdge() == -1) return;

		if (!getHalfEdge().isActive()) return;

		zItMeshHalfEdge start = getHalfEdge();
		zItMeshHalfEdge e = getHalfEdge();

		bool exit = false;

		do
		{
			halfedges.push_back(e);
			e = e.getPrev().getSym();

		} while (e != start);
	}

	ZSPACE_INLINE void zItMeshVertex::getConnectedHalfEdges(zIntArray& halfedgeIndicies)
	{
		if (zMeshObjectStorage::get(*meshObj).vertices[index].getHalfEdge() == -1) return;

		if (!getHalfEdge().isActive()) return;


		zItMeshHalfEdge start = getHalfEdge();
		zItMeshHalfEdge e = getHalfEdge();

		bool exit = false;

		do
		{
			halfedgeIndicies.push_back(e.getId());
			e = e.getPrev().getSym();

		} while (e != start);
	}

	ZSPACE_INLINE void zItMeshVertex::getConnectedEdges(zItMeshEdgeArray& edges)
	{
		zItMeshHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);

		for (auto &he : cHEdges)
		{
			edges.push_back(he.getEdge());
		}
	}

	ZSPACE_INLINE void zItMeshVertex::getConnectedEdges(zIntArray& edgeIndicies)
	{
		zItMeshHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);

		for (auto &he : cHEdges)
		{
			edgeIndicies.push_back(he.getEdge().getId());
		}
	}

	ZSPACE_INLINE void zItMeshVertex::getConnectedVertices(zItMeshVertexArray& verticies)
	{
		zItMeshHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);


		for (auto &he : cHEdges)
		{
			verticies.push_back(he.getVertex());
		}
	}

	ZSPACE_INLINE void zItMeshVertex::getConnectedVertices(zIntArray& vertexIndicies)
	{
		zItMeshHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);


		for (auto &he : cHEdges)
		{
			vertexIndicies.push_back(he.getVertex().getId());
		}
	}

	ZSPACE_INLINE void zItMeshVertex::getConnectedFaces(zItMeshFaceArray& faces)
	{
		zItMeshHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);


		for (auto &he : cHEdges)
		{
			if (!he.onBoundary()) faces.push_back(he.getFace());
		}
	}

	ZSPACE_INLINE void zItMeshVertex::getConnectedFaces(zIntArray& faceIndicies)
	{
		zItMeshHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);


		for (auto &he : cHEdges)
		{
			if (!he.onBoundary()) faceIndicies.push_back(he.getFace().getId());
		}
	}

	ZSPACE_INLINE bool zItMeshVertex::onBoundary()
	{
		bool out = false;

		zItMeshHalfEdgeArray cHEdges;
		getConnectedHalfEdges(cHEdges);

		for (auto &he : cHEdges)
		{
			if (he.onBoundary())
			{
				out = true;
				break;
			}

		}

		return out;
	}

	ZSPACE_INLINE int zItMeshVertex::getValence()
	{
		int out;

		zIntArray cHEdges;
		getConnectedHalfEdges(cHEdges);

		out = cHEdges.size();

		return out;
	}

	ZSPACE_INLINE bool zItMeshVertex::checkValency(int valence)
	{
		bool out = false;
		out = (getValence() == valence) ? true : false;

		return out;
	}

	ZSPACE_INLINE zCurvature zItMeshVertex::getPrincipalCurvature()
	{
		double angleSum = 0;
		double cotangentSum = 0;
		double areaSum = 0;
		double areaSumMixed = 0;
		double edgeLengthSquare = 0;
		float gaussianCurv = 0;
		float gaussianAngle = 0;

		zCurvature curv;
		curv.k1 = 0;
		curv.k2 = 0;

		zVector meanCurvNormal;


		if (!onBoundary())
		{
			zItMeshVertexArray cVerts;
			getConnectedVertices(cVerts);

			zVector pt = getPosition();

			float multFactor = 0.125;

			int i = 0;
			for (auto v : cVerts)
			{
				int next = (i + 1) % cVerts.size();
				int prev = (i + cVerts.size() - 1) % cVerts.size();

				zVector pt1 = v.getPosition();
				zVector pt2 = cVerts[next].getPosition();
				zVector pt3 = cVerts[prev].getPosition();

				zVector p01 = pt - pt1;
				zVector p02 = pt - pt2;
				zVector p10 = pt1 - pt;
				zVector p20 = pt2 - pt;
				zVector p12 = pt1 - pt2;
				zVector p21 = pt2 - pt1;
				zVector p31 = pt3 - pt1;

				zVector cr = (p10) ^ (p20);

				float ang = (p10).angle(p20);
				angleSum += ang;
				cotangentSum += (((p20)*(p10)) / cr.length());


				float e_Length = (pt1 - pt2).length();

				edgeLengthSquare += (e_Length * e_Length);

				zVector cr_alpha = (p01) ^ (p21);
				zVector cr_beta = (p01) ^ (p31);

				float coTan_alpha = (((p01)*(p21)) / cr_alpha.length());
				float coTan_beta = (((p01)*(p31)) / cr_beta.length());

				zVector cross = (pt1 - pt) ^ (pt2 - pt);
				float area = 0.5 * cross.length();
				areaSum += area;

				// check if triangle is obtuse
				if ((p10).angle(p20) <= 90 && (p01).angle(p21) <= 90 && (p12).angle(p02) <= 90)
				{
					areaSumMixed += (coTan_alpha + coTan_beta) * edgeLengthSquare * 0.125;
				}
				else
				{

					double triArea = (((p10) ^ (p20)).length()) / 2;

					if ((ang) <= 90) areaSumMixed += triArea * 0.25;
					else areaSumMixed += triArea * 0.5;

				}

				meanCurvNormal += ((pt - pt1)*(coTan_alpha + coTan_beta));

				i++;
			}

			meanCurvNormal /= (2 * areaSumMixed);

			// using https://arxiv.org/abs/0804.1046
			gaussianCurv = (360 - angleSum) / ((0.5 * areaSum) - (multFactor * cotangentSum * edgeLengthSquare));
			
			////// Based on Discrete Differential-Geometry Operators for Triangulated 2-Manifolds
			//https://link.springer.com/chapter/10.1007/978-3-662-05105-4_2
			//gaussianCurv = (360 - angleSum) / areaSumMixed;

			//printf("\n %1.6f ", gaussianCurv);

			double meanCurv = (meanCurvNormal.length() / 2);
			//if (meanCurv <0.001) meanCurv = 0;

			double deltaX = (meanCurv*meanCurv) - gaussianCurv;
			if (deltaX < 0) deltaX = 0;


			curv.k1 = meanCurv + sqrt(deltaX);
			curv.k2 = meanCurv - sqrt(deltaX);


		}

		return curv;
	}

	ZSPACE_INLINE double zItMeshVertex::getGaussianCurvature()
	{
		double out = -1;

		double angleSum = 0;
		double cotangentSum = 0;
		double areaSum = 0;
		double areaSumMixed = 0;
		double edgeLengthSquare = 0;
		float gaussianCurv = 0;
		float gaussianAngle = 0;
		

		zVector meanCurvNormal;


		if (!onBoundary())
		{
			zItMeshVertexArray cVerts;
			getConnectedVertices(cVerts);

			zVector pt = getPosition();

			float multFactor = 0.125;

			int i = 0;
			for (auto v : cVerts)
			{
				int next = (i + 1) % cVerts.size();
				int prev = (i + cVerts.size() - 1) % cVerts.size();

				zVector pt1 = v.getPosition();
				zVector pt2 = cVerts[next].getPosition();
				zVector pt3 = cVerts[prev].getPosition();

				//if (onBoundary() && cVerts[next].onBoundary() && cVerts[prev].onBoundary()) continue;

				zVector p01 = pt - pt1;
				zVector p02 = pt - pt2;
				zVector p10 = pt1 - pt;
				zVector p20 = pt2 - pt;
				zVector p12 = pt1 - pt2;
				zVector p21 = pt2 - pt1;
				zVector p31 = pt3 - pt1;

				zVector cr = (p10) ^ (p20);

				float ang = (p10).angle(p20);
				angleSum += ang;
				cotangentSum += (((p20) * (p10)) / cr.length());


				float e_Length = (pt1 - pt2).length();

				edgeLengthSquare += (e_Length * e_Length);

				zVector cr_alpha = (p01) ^ (p21);
				zVector cr_beta = (p01) ^ (p31);

				float coTan_alpha = (((p01) * (p21)) / cr_alpha.length());
				float coTan_beta = (((p01) * (p31)) / cr_beta.length());

				zVector cross = (pt1 - pt) ^ (pt2 - pt);
				float area = 0.5 * cross.length();
				areaSum += area;

				// check if triangle is obtuse
				if ((p10).angle(p20) <= 90 && (p01).angle(p21) <= 90 && (p12).angle(p02) <= 90)
				{
					areaSumMixed += (coTan_alpha + coTan_beta) * edgeLengthSquare * 0.125;
				}
				else
				{

					double triArea = (((p10) ^ (p20)).length()) / 2;

					if ((ang) <= 90) areaSumMixed += triArea * 0.25;
					else areaSumMixed += triArea * 0.5;

				}

				meanCurvNormal += ((pt - pt1) * (coTan_alpha + coTan_beta));

				i++;
			}

			meanCurvNormal /= (2 * areaSumMixed);

			gaussianCurv = ((360 - angleSum) * DEG_TO_RAD);
			//gaussianCurv /= getArea();

			/// 
			// using https://arxiv.org/abs/0804.1046
			//gaussianCurv = (gaussianCurv) / ((0.5 * areaSum) - (multFactor * cotangentSum * edgeLengthSquare));

			////// Based on Discrete Differential-Geometry Operators for Triangulated 2-Manifolds
			//https://link.springer.com/chapter/10.1007/978-3-662-05105-4_2
			//gaussianCurv = gaussianCurv / areaSumMixed;

		}

		return gaussianCurv;	
	}

	ZSPACE_INLINE zVector zItMeshVertex::getGaussianGradient()
	{
		zVector out;

		double g0 = getGaussianCurvature();

		zVector temp = getPosition();

		zVector dx = zVector(EPS, 0, 0);
		//setPosition(temp - dx);
		//double gx0 = getGaussianCurvature();
		setPosition(temp + dx);
		double gx1 = getGaussianCurvature();

		zVector dy = zVector(0, EPS, 0);
		//setPosition(temp - dy);
		//double gy0 = getGaussianCurvature();
		setPosition(temp + dy);
		double gy1 = getGaussianCurvature();

		zVector dz = zVector(0, 0, EPS);
		//setPosition(temp - dz);
		//double gz0 = getGaussianCurvature();
		setPosition(temp + dz);
		double gz1 = getGaussianCurvature();

		setPosition(temp);

		zVector grad = zVector(g0 - gx1, g0 - gy1, g0 - gz1);
		grad.normalize();

	/*	double lengthSum;

		zItMeshVertexArray cVerts;
		getConnectedVertices(cVerts);

		for (int i = 0; i < cVerts.size(); i++)
		{
			double edgeLength = getPosition().distanceTo(cVerts[i].getPosition());
			lengthSum += edgeLength;
		}

		double avg = lengthSum / cVerts.size();

		double mag = g0 * avg;
	*/
		
		
		///out = grad * mag;

		return grad;
		
	}

	ZSPACE_INLINE double zItMeshVertex::getArea()
	{
		vector<zVector> cFCenters, cECenters;

		if (isActive())
		{
			zItMeshHalfEdgeArray cEdges;
			getConnectedHalfEdges(cEdges);


			for (auto &he : cEdges)
			{
				cECenters.push_back(he.getCenter());
				cFCenters.push_back(he.getFace().getCenter());
			}


			double vArea = 0;

			for (int j = 0; j < cEdges.size(); j++)
			{
				int curId = j;
				int nextId = (j + 1) % cEdges.size();

				zItMeshHalfEdge cE = cEdges[j];
				zItMeshHalfEdge nE = cEdges[(j + 1) % cEdges.size()];

				if (cE.onBoundary() || nE.getSym().onBoundary()) continue;

				if (cE.getFace().getId() != nE.getSym().getFace().getId()) continue;

				zVector vPos = getPosition();
				zVector fCen = cFCenters[curId];
				zVector currentEdge_cen = cECenters[curId];
				zVector nextEdge_cen = cECenters[nextId];

				double Area1 = zMeshObjectStorage::get(*meshObj).coreUtils.getTriangleArea(vPos, currentEdge_cen, fCen);
				vArea += (Area1);

				double Area2 = zMeshObjectStorage::get(*meshObj).coreUtils.getTriangleArea(vPos, nextEdge_cen, fCen);
				vArea += (Area2);
			}

			return vArea;

		}

		else return 0;
	}

	//---- GET METHODS

	ZSPACE_INLINE int zItMeshVertex::getId()
	{
		return index;
	}

	ZSPACE_INLINE zItMeshHalfEdge zItMeshVertex::getHalfEdge()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		auto& meshData = zMeshObjectStorage::get(*meshObj);
		const int halfedgeId = meshData.vertices[index].getHalfEdge();
		if (halfedgeId < 0 || halfedgeId >= static_cast<int>(meshData.halfEdges.size())) throw std::invalid_argument(" error: halfedge index out of bounds.");
		return zItMeshHalfEdge(*meshObj, halfedgeId);
	}

	ZSPACE_INLINE zVector zItMeshVertex::getPosition()
	{
		return zMeshObjectStorage::read(*meshObj).positions[index];
	}

	ZSPACE_INLINE zVector* zItMeshVertex::getRawPosition()
	{
		return &zMeshObjectStorage::edit(*meshObj).positions[index];
	}

	ZSPACE_INLINE zVector zItMeshVertex::getNormal()
	{
		return zMeshObjectStorage::read(*meshObj).vertexNormals[index];
	}

	ZSPACE_INLINE zVector* zItMeshVertex::getRawNormal()
	{
		return &zMeshObjectStorage::edit(*meshObj).vertexNormals[index];
	}

	ZSPACE_INLINE void zItMeshVertex::getNormals(vector<zVector> &vNormals)
	{
		vNormals.clear();
		zItMeshFaceArray cFaces;
		getConnectedFaces(cFaces);

		for (auto &f : cFaces)
		{
			vNormals.push_back(f.getNormal());
		}
	}

	ZSPACE_INLINE zColor zItMeshVertex::getColor()
	{
		return zMeshObjectStorage::read(*meshObj).vertexColors[index];
	}

	ZSPACE_INLINE zColor* zItMeshVertex::getRawColor()
	{
		return &zMeshObjectStorage::edit(*meshObj).vertexColors[index];
	}

	//---- SET METHODS

	ZSPACE_INLINE void zItMeshVertex::setId(int _id)
	{
		if (_id < 0 || _id >= size()) throw std::invalid_argument("vertex index out of bounds.");
		index = _id;
	}

	ZSPACE_INLINE void zItMeshVertex::setHalfEdge(zItMeshHalfEdge &he)
	{
		

		int id = getId();
		int heId = he.getId();

		zMeshObjectStorage::get(*meshObj).vertices[index].setHalfEdge(heId);

		zMeshObjectStorage::get(*meshObj).vHandles[id].he = heId;
	}

	ZSPACE_INLINE void zItMeshVertex::setPosition(zVector pos)
	{
		zMeshObjectStorage::edit(*meshObj).positions[index] = pos;
	}

	ZSPACE_INLINE void zItMeshVertex::setColor(zColor col)
	{
		zMeshObjectStorage::edit(*meshObj).vertexColors[index] = col;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE bool zItMeshVertex::isActive()
	{
		return meshObj && index >= 0 && index < size();
	}

	//---- OPERATOR METHODS

	ZSPACE_INLINE bool zItMeshVertex::operator==(zItMeshVertex &other)
	{
		return (getId() == other.getId());
	}

	ZSPACE_INLINE bool zItMeshVertex::operator!=(zItMeshVertex &other)
	{
		return (getId() != other.getId());
	}

}

//---- ZIT_MESH_EDGE ------------------------------------------------------------------------------

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zItMeshEdge::zItMeshEdge()
	{
		meshObj = nullptr;
		index = 0;
	}

	ZSPACE_INLINE zItMeshEdge::zItMeshEdge(zObjectMesh &_meshObj)
	{
		meshObj = &_meshObj;
		index = 0;
	}

	ZSPACE_INLINE zItMeshEdge::zItMeshEdge(zObjectMesh &_meshObj, int _index)
	{
		meshObj = &_meshObj;
		if (_index < 0 || _index >= zMeshObjectStorage::read(*meshObj).numEdges()) throw std::invalid_argument(" error: index out of bounds");
		index = _index;
	}

	//--------------------------
	//---- OVERRIDE METHODS
	//--------------------------

	ZSPACE_INLINE void zItMeshEdge::begin()
	{
		index = 0;
	}

	ZSPACE_INLINE void zItMeshEdge::operator++(int)
	{
		index++;
	}

	ZSPACE_INLINE void zItMeshEdge::operator--(int)
	{
		index--;
	}

	ZSPACE_INLINE bool zItMeshEdge::end()
	{
		return !meshObj || index >= zMeshObjectStorage::read(*meshObj).numEdges();
	}

	ZSPACE_INLINE void zItMeshEdge::reset()
	{
		index = 0;
	}

	ZSPACE_INLINE int zItMeshEdge::size()
	{
		if (!meshObj) return 0;
		return zMeshObjectStorage::read(*meshObj).numEdges();
	}

	ZSPACE_INLINE void zItMeshEdge::deactivate()
	{
		throw std::logic_error("Mesh topology editing is not supported by face-list iterators.");
	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE void zItMeshEdge::getVertices(zItMeshVertexArray &verticies)
	{
		zIntArray ids;
		getVertices(ids);
		verticies.emplace_back(*meshObj, ids[0]);
		verticies.emplace_back(*meshObj, ids[1]);
	}

	ZSPACE_INLINE void zItMeshEdge::getVertices(zIntArray &vertexIndicies)
	{
		const auto& edges = zMeshObjectStorage::read(*meshObj).edgeVertexIndices;
		vertexIndicies.push_back(edges[index * 2]);
		vertexIndicies.push_back(edges[index * 2 + 1]);
	}

	ZSPACE_INLINE void zItMeshEdge::getVertexPositions(vector<zVector> &vertPositions)
	{
		zIntArray eVerts;

		getVertices(eVerts);

		for (int i = 0; i < eVerts.size(); i++)
		{
			vertPositions.push_back(zMeshObjectStorage::read(*meshObj).positions[eVerts[i]]);
		}
	}

	ZSPACE_INLINE void zItMeshEdge::getFaces(zItMeshFaceArray &faces)
	{
		faces.clear();

		if (!getHalfEdge(0).onBoundary()) faces.push_back(getHalfEdge(0).getFace());
		if (!getHalfEdge(1).onBoundary()) faces.push_back(getHalfEdge(1).getFace());
	}

	ZSPACE_INLINE void zItMeshEdge::getFaces(zIntArray &faceIndicies)
	{
		faceIndicies.clear();

		if (!getHalfEdge(0).onBoundary()) faceIndicies.push_back(getHalfEdge(0).getFace().getId());
		if (!getHalfEdge(1).onBoundary()) faceIndicies.push_back(getHalfEdge(1).getFace().getId());
	}

	ZSPACE_INLINE bool zItMeshEdge::onBoundary()
	{
		return (getHalfEdge(0).onBoundary() || getHalfEdge(1).onBoundary());
	}

	ZSPACE_INLINE zVector zItMeshEdge::getCenter()
	{
		zIntArray eVerts;
		getVertices(eVerts);

		return (zMeshObjectStorage::get(*meshObj).vertexPositions[eVerts[0]] + zMeshObjectStorage::get(*meshObj).vertexPositions[eVerts[1]]) * 0.5;
	}

	ZSPACE_INLINE zVector zItMeshEdge::getVector()
	{

		int v1 = getHalfEdge(0).getVertex().getId();
		int v2 = getHalfEdge(1).getVertex().getId();

		zVector out = zMeshObjectStorage::get(*meshObj).vertexPositions[v1] - (zMeshObjectStorage::get(*meshObj).vertexPositions[v2]);

		return out;
	}

	ZSPACE_INLINE double zItMeshEdge::getLength()
	{
		return getVector().length();
	}

	ZSPACE_INLINE double zItMeshEdge::getDihedralAngle()
	{
		if (isActive())
		{
			if (!onBoundary())
			{
				// get connected face to edge
				zIntArray cFaces;
				getFaces(cFaces);

				zVector n0 = zMeshObjectStorage::get(*meshObj).faceNormals[cFaces[0]];
				zVector n1 = zMeshObjectStorage::get(*meshObj).faceNormals[cFaces[1]];

				zVector eVec = getVector();

				double di_ang;
				di_ang = eVec.dihedralAngle(n0, n1);

				// per edge
				return (di_ang);
			}
			else
			{
				// per  edge
				return (-1);

			}
		}
		else return -2;
	}

	//---- GET METHODS

	ZSPACE_INLINE int zItMeshEdge::getId()
	{
		return index;
	}

	ZSPACE_INLINE zItMeshHalfEdge zItMeshEdge::getHalfEdge(int _index)
	{	
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		if (_index < 0 || _index > 1) throw std::invalid_argument(" error: halfedge slot out of bounds.");
		auto& meshData = zMeshObjectStorage::get(*meshObj);
		const int halfedgeId = meshData.edges[index].getHalfEdge(_index);
		if (halfedgeId < 0 || halfedgeId >= static_cast<int>(meshData.halfEdges.size())) throw std::invalid_argument(" error: halfedge index out of bounds.");
		return zItMeshHalfEdge(*meshObj, halfedgeId);
	}

	ZSPACE_INLINE zColor zItMeshEdge::getColor()
	{
		return zMeshObjectStorage::read(*meshObj).edgeColors[index];
	}

	ZSPACE_INLINE zColor* zItMeshEdge::getRawColor()
	{
		return &zMeshObjectStorage::edit(*meshObj).edgeColors[index];
	}

	//---- SET METHODS

	ZSPACE_INLINE void zItMeshEdge::setId(int _id)
	{
		if (_id < 0 || _id >= size()) throw std::invalid_argument("edge index out of bounds.");
		index = _id;
	}

	ZSPACE_INLINE void zItMeshEdge::setHalfEdge(zItMeshHalfEdge &he, int _index)
	{
		

		int id = getId();
		int heId = he.getId();
		zMeshObjectStorage::get(*meshObj).edges[index].setHalfEdge(heId, _index);

		if (_index == 0) zMeshObjectStorage::get(*meshObj).eHandles[id].he0 = heId;
		if (_index == 1) zMeshObjectStorage::get(*meshObj).eHandles[id].he1 = heId;
	}

	ZSPACE_INLINE void zItMeshEdge::setColor(zColor col)
	{
		zMeshObjectStorage::edit(*meshObj).edgeColors[index] = col;
	}

	ZSPACE_INLINE void zItMeshEdge::setWeight(double wt)
	{
		zMeshObjectStorage::edit(*meshObj).edgeWeights[index] = wt;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE bool zItMeshEdge::isActive()
	{
		return meshObj && index >= 0 && index < size();
	}

	//---- OPERATOR METHODS

	ZSPACE_INLINE bool zItMeshEdge::operator==(zItMeshEdge &other)
	{
		return (getId() == other.getId());
	}

	ZSPACE_INLINE bool zItMeshEdge::operator!=(zItMeshEdge &other)
	{
		return (getId() != other.getId());
	}

}

//---- ZIT_MESH_FACE ------------------------------------------------------------------------------

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zItMeshFace::zItMeshFace()
	{
		meshObj = nullptr;
		index = 0;
	}

	ZSPACE_INLINE zItMeshFace::zItMeshFace(zObjectMesh &_meshObj)
	{
		meshObj = &_meshObj;
		index = 0;
	}

	ZSPACE_INLINE zItMeshFace::zItMeshFace(zObjectMesh &_meshObj, int _index)
	{
		meshObj = &_meshObj;
		if (_index < 0 || _index >= zMeshObjectStorage::read(*meshObj).numFaces()) throw std::invalid_argument(" error: index out of bounds");
		index = _index;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zItMeshFace::begin()
	{
		index = 0;
	}

	ZSPACE_INLINE void zItMeshFace::operator++(int)
	{
		index++;
	}

	ZSPACE_INLINE void zItMeshFace::operator--(int)
	{
		index--;
	}

	ZSPACE_INLINE bool zItMeshFace::end()
	{
		return !meshObj || index >= zMeshObjectStorage::read(*meshObj).numFaces();
	}

	ZSPACE_INLINE void zItMeshFace::reset()
	{
		index = 0;
	}

	ZSPACE_INLINE int zItMeshFace::size()
	{
		if (!meshObj) return 0;
		return zMeshObjectStorage::read(*meshObj).numFaces();
	}

	ZSPACE_INLINE void zItMeshFace::deactivate()
	{
		throw std::logic_error("Mesh topology editing is not supported by face-list iterators.");

	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE void zItMeshFace::getHalfEdges(zItMeshHalfEdgeArray &halfedges)
	{
		halfedges.clear();
		if (!isActive()) return;

		zItMeshHalfEdge start;
		try
		{
			start = getHalfEdge();
			if (start.onBoundary()) return;
		}
		catch (...)
		{
			return;
		}

		zItMeshHalfEdge e = start;
		std::unordered_set<int> visitedHalfedges;
		const int maxSteps = static_cast<int>(zMeshObjectStorage::get(*meshObj).halfEdges.size());
		int stepCount = 0;

		while (stepCount++ < maxSteps)
		{
			const int currentId = e.getId();
			if (visitedHalfedges.find(currentId) != visitedHalfedges.end()) break;

			visitedHalfedges.insert(currentId);
			halfedges.push_back(e);

			try
			{
				e = e.getNext();
			}
			catch (...)
			{
				break;
			}

			if (e == start) break;
		}
	}

	ZSPACE_INLINE void zItMeshFace::getHalfEdges(zIntArray &halfedgeIndicies)
	{
		halfedgeIndicies.clear();

		zItMeshHalfEdgeArray halfedges;
		getHalfEdges(halfedges);
		for (auto& he : halfedges)
		{
			halfedgeIndicies.push_back(he.getId());
		}
	}

	ZSPACE_INLINE void zItMeshFace::getVertices(zItMeshVertexArray &verticies)
	{
		zIntArray ids;
		getVertices(ids);
		for (int id : ids) verticies.emplace_back(*meshObj, id);
	}

	ZSPACE_INLINE void zItMeshFace::getVertices(zIntArray &vertexIndicies)
	{
		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int i = data.faceOffsets[index]; i < data.faceOffsets[index + 1]; ++i)
			vertexIndicies.push_back(data.faceVertexIndices[i]);
	}

	ZSPACE_INLINE void zItMeshFace::getVertexPositions(vector<zVector> &vertPositions)
	{
		zIntArray fVerts;

		getVertices(fVerts);

		for (int i = 0; i < fVerts.size(); i++)
		{
			vertPositions.push_back(zMeshObjectStorage::read(*meshObj).positions[fVerts[i]]);
		}
	}

	ZSPACE_INLINE void zItMeshFace::getConnectedFaces(zItMeshFaceArray& faces)
	{
		zItMeshHalfEdgeArray cHEdges;
		getHalfEdges(cHEdges);		

		for (auto &he : cHEdges)
		{
			zItMeshFaceArray eFaces;
			he.getFaces(eFaces);

			//printf("\n eFaces %i", eFaces.size());

			for (int k = 0; k < eFaces.size(); k++)
			{
				if (eFaces[k].getId() != getId()) faces.push_back(eFaces[k]);
			}
		}

		//printf("\n %i  e %i %i", getId(), cHEdges.size(), faces.size());
	}

	ZSPACE_INLINE void zItMeshFace::getConnectedFaces(zIntArray& faceIndicies)
	{
		zItMeshHalfEdgeArray cHEdges;
		getHalfEdges(cHEdges);


		for (auto &he : cHEdges)
		{
			zIntArray eFaces;
			he.getFaces(eFaces);

			for (int k = 0; k < eFaces.size(); k++)
			{
				if (eFaces[k] != getId()) faceIndicies.push_back(eFaces[k]);
			}
		}
	}

	ZSPACE_INLINE bool zItMeshFace::onBoundary()
	{
		bool out = false;

		zItMeshHalfEdgeArray fHEdges;
		getHalfEdges(fHEdges);


		for (auto &he : fHEdges)
		{
			if (he.getSym().onBoundary())
			{
				out = true;
				break;
			}
		}

		return out;
	}

	ZSPACE_INLINE zVector zItMeshFace::getCenter()
	{
		zIntArray fVerts;
		getVertices(fVerts);
		zVector cen;

		for (int j = 0; j < fVerts.size(); j++) cen += zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[j]];
		cen /= fVerts.size();

		return cen;
	}

	ZSPACE_INLINE int zItMeshFace::getNumVertices()
	{
		zIntArray fEdges;
		getHalfEdges(fEdges);

		return fEdges.size();
	}

	ZSPACE_INLINE void zItMeshFace::getTriangles(int &numTris, zIntArray &tris)
	{
		double angle_Max = 90;
		bool noEars = true; // check for if there are no ears

		vector<bool> ears;
		vector<bool> reflexVerts;

		// get face vertices

		zIntArray fVerts;

		getVertices(fVerts);
		zIntArray vertexIndices = fVerts;

		int faceIndex = getId();

		vector<zVector> points;
		getVertexPositions(points);


		if (fVerts.size() < 3) throw std::invalid_argument(" error: invalid face, triangulation is not succesful.");

		// compute 			
		zVector norm = zMeshObjectStorage::get(*meshObj).faceNormals[faceIndex];

		// compute ears

		for (int i = 0; i < vertexIndices.size(); i++)
		{
			int nextId = (i + 1) % vertexIndices.size();
			int prevId = (i - 1 + vertexIndices.size()) % vertexIndices.size();

			// Triangle edges - e1 and e2 defined above
			zVector v1 = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[nextId]] - zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]];
			zVector v2 = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[prevId]] - zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]];

			zVector cross = v1 ^ v2;
			double ang = v1.angle(v2);

			if (cross * norm < 0) ang *= -1;

			if (ang <= 0 || ang == 180) reflexVerts.push_back(true);
			else reflexVerts.push_back(false);

			// calculate ears
			if (!reflexVerts[i])
			{
				bool ear = true;

				zVector p0 = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[i]];
				zVector p1 = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[nextId]];
				zVector p2 = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[prevId]];

				bool CheckPtTri = false;

				for (int j = 0; j < fVerts.size(); j++)
				{
					if (!CheckPtTri)
					{
						if (j != i && j != nextId && j != prevId)
						{
							// vector to point to be checked
							zVector pt = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[j]];

							bool Chk = zMeshObjectStorage::get(*meshObj).coreUtils.pointInTriangle(pt, p0, p1, p2);
							CheckPtTri = Chk;

						}
					}

				}

				if (CheckPtTri) ear = false;
				ears.push_back(ear);

				if (noEars && ear) noEars = !noEars;
			}
			else ears.push_back(false);

			//printf("\n id: %i ang: %1.2f reflex: %s ear: %s", vertexIndices[i], ang, (reflexVerts[i] == true) ? "true" : "false",(ears[i] == true)?"true":"false");
		}

		if (noEars)
		{
			for (int i = 0; i < fVerts.size(); i++)
			{
				//printf("\n %1.2f %1.2f %1.2f ", points[i].x, points[i].y, points[i].z);
			}

			throw std::invalid_argument(" error: no ears found in the face, triangulation is not succesful.");
		}

		int maxTris = fVerts.size() - 2;

		// // triangulate 

		while (numTris < maxTris - 1)
		{
			//printf("\n working!");

			int earId = -1;
			bool earFound = false;;

			for (int i = 0; i < ears.size(); i++)
			{
				if (!earFound)
				{
					if (ears[i])
					{
						earId = i;
						earFound = !earFound;
					}
				}

			}

			if (earFound)
			{


				for (int i = -1; i <= 1; i++)
				{
					int id = (earId + i + vertexIndices.size()) % vertexIndices.size();
					tris.push_back(vertexIndices[id]);
				}
				numTris++;

				// remove vertex earid 
				vertexIndices.erase(vertexIndices.begin() + earId);

				reflexVerts.clear();
				ears.clear();

				// check for ears
				for (int i = 0; i < vertexIndices.size(); i++)
				{

					int nextId = (i + 1) % vertexIndices.size();
					int prevId = (i - 1 + vertexIndices.size()) % vertexIndices.size();

					// Triangle edges - e1 and e2 defined above
					zVector v1 = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[nextId]] - zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]];
					zVector v2 = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[prevId]] - zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]];

					zVector cross = v1 ^ v2;
					double ang = v1.angle(v2);

					if (cross * norm < 0) ang *= -1;

					if (ang <= 0 || ang == 180) reflexVerts.push_back(true);
					else reflexVerts.push_back(false);

					// calculate ears
					if (!reflexVerts[i])
					{
						bool ear = true;

						zVector p0 = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]];
						zVector p1 = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[nextId]];
						zVector p2 = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[prevId]];

						bool CheckPtTri = false;

						for (int j = 0; j < vertexIndices.size(); j++)
						{
							if (!CheckPtTri)
							{
								if (j != i && j != nextId && j != prevId)
								{
									// vector to point to be checked
									zVector pt = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[j]];

									bool Chk = zMeshObjectStorage::get(*meshObj).coreUtils.pointInTriangle(pt, p0, p1, p2);
									CheckPtTri = Chk;
								}
							}

						}

						if (CheckPtTri) ear = false;
						ears.push_back(ear);

					}
					else ears.push_back(false);


					//printf("\n earId %i id: %i ang: %1.2f reflex: %s ear: %s", earId, vertexIndices[i], ang, (reflexVerts[i] == true) ? "true" : "false", (ears[i] == true) ? "true" : "false");
				}



			}
			else
			{
				for (int i = 0; i < vertexIndices.size(); i++)
				{
					//printf("\n %1.2f %1.2f %1.2f ", zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]].x, zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]].y, zMeshObjectStorage::get(*meshObj).vertexPositions[vertexIndices[i]].z);
				}

				throw std::invalid_argument(" error: no ears found in the face, triangulation is not succesful.");
			}

		}

		// add the last remaining triangle
		tris.push_back(vertexIndices[0]);
		tris.push_back(vertexIndices[1]);
		tris.push_back(vertexIndices[2]);
		numTris++;

	}

	ZSPACE_INLINE double zItMeshFace::getVolume(zIntArray &faceTris, zVector &fCenter, bool absoluteVolume)
	{

		int faceNumTris;

		if (faceTris.size() == 0) 	getTriangles(faceNumTris, faceTris);

		if (faceNumTris == 1) return 0.0;

		zVector v;
		if (fCenter == v) fCenter = getCenter();

		double out = 0;

		int index = getId();

		// add volume of face tris			
		for (int j = 0; j < faceTris.size(); j += 3)
		{
			double vol = zMeshObjectStorage::get(*meshObj).coreUtils.getSignedTriangleVolume(zMeshObjectStorage::get(*meshObj).vertexPositions[faceTris[j + 0]], zMeshObjectStorage::get(*meshObj).vertexPositions[faceTris[j + 1]], zMeshObjectStorage::get(*meshObj).vertexPositions[faceTris[j + 2]]);

			out += vol;
		}

		// add volumes of tris formes by each pair of face edge vertices and face center

		vector<zVector> fVerts;
		getVertexPositions(fVerts);

		for (int j = 0; j < fVerts.size(); j += 1)
		{
			int prevId = (j - 1 + fVerts.size()) % fVerts.size();

			double vol = zMeshObjectStorage::get(*meshObj).coreUtils.getSignedTriangleVolume(fVerts[j], fVerts[prevId], fCenter);

			out += vol;
		}

		if (absoluteVolume) out = abs(out);

		return out;

	}

	ZSPACE_INLINE double zItMeshFace::getPlanarFaceArea()
	{
		double fArea = 0;

		if (isActive())
		{
			zVector fNorm = getNormal();

			vector<zVector> fVerts;
			getVertexPositions(fVerts);

			for (int j = 0; j < fVerts.size(); j++)
			{
				zVector v1 = fVerts[j];
				zVector v2 = fVerts[(j + 1) % fVerts.size()];


				fArea += fNorm * (v1 ^ v2);
			}

			fArea *= 0.5;

		}

		return fArea;
	}

	ZSPACE_INLINE zCurvature zItMeshFace::getPrincipalCurvature()
	{
		double angleSum = 0;
		double cotangentSum = 0;
		double areaSum = 0;
		double areaSumMixed = 0;
		double edgeLengthSquare = 0;
		float gaussianCurv = 0;
		float gaussianAngle = 0;

		float multFactor = 0.125;

		zCurvature curv;
		curv.k1 = 0;
		curv.k2 = 0;

		zVector meanCurvNormal;

		zPoint fCenter = getCenter();

		zPointArray eCenters;
		zItMeshHalfEdgeArray fHEdges;
		getHalfEdges(fHEdges);

		for (auto& he : fHEdges) eCenters.push_back(he.getCenter());

		int i = 0;
		for (auto &v : eCenters)
		{
			int next = (i + 1) % eCenters.size();
			int prev = (i + eCenters.size() - 1) % eCenters.size();

			zVector pt1 = eCenters[i];
			zVector pt2 = eCenters[next];
			zVector pt3 = eCenters[prev];

			zVector p01 = fCenter - pt1;
			zVector p02 = fCenter - pt2;
			zVector p10 = pt1 - fCenter;
			zVector p20 = pt2 - fCenter;
			zVector p12 = pt1 - pt2;
			zVector p21 = pt2 - pt1;
			zVector p31 = pt3 - pt1;

			zVector cr = (p10) ^ (p20);

			float ang = (p10).angle(p20);
			angleSum += ang;
			cotangentSum += (((p20) * (p10)) / cr.length());


			float e_Length = (pt1 - pt2).length();

			edgeLengthSquare += (e_Length * e_Length);

			zVector cr_alpha = (p01) ^ (p21);
			zVector cr_beta = (p01) ^ (p31);

			float coTan_alpha = (((p01) * (p21)) / cr_alpha.length());
			float coTan_beta = (((p01) * (p31)) / cr_beta.length());

			// check if triangle is obtuse
			if ((p10).angle(p20) <= 90 && (p01).angle(p21) <= 90 && (p12).angle(p02) <= 90)
			{
				areaSumMixed += (coTan_alpha + coTan_beta) * edgeLengthSquare * 0.125;
			}
			else
			{

				double triArea = (((p10) ^ (p20)).length()) / 2;

				if ((ang) <= 90) areaSumMixed += triArea * 0.25;
				else areaSumMixed += triArea * 0.5;

			}

			meanCurvNormal += ((fCenter - pt1) * (coTan_alpha + coTan_beta));

			i++;
		}

		meanCurvNormal /= (2 * areaSumMixed);

		gaussianCurv = (360 - angleSum) / ((0.5 * areaSum) - (multFactor * cotangentSum * edgeLengthSquare));

		double meanCurv = (meanCurvNormal.length() / 2);

		double deltaX = (meanCurv * meanCurv) - gaussianCurv;
		if (deltaX < 0) deltaX = 0;

		curv.k1 = meanCurv + sqrt(deltaX);
		curv.k2 = meanCurv - sqrt(deltaX);			

		return curv;
	}

	//---- GET METHODS

	ZSPACE_INLINE int zItMeshFace::getId()
	{
		return index;
	}

	ZSPACE_INLINE zItMeshHalfEdge zItMeshFace::getHalfEdge()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		auto& meshData = zMeshObjectStorage::get(*meshObj);
		const int halfedgeId = meshData.faces[index].getHalfEdge();
		if (halfedgeId < 0 || halfedgeId >= static_cast<int>(meshData.halfEdges.size())) throw std::invalid_argument(" error: halfedge index out of bounds.");
		return zItMeshHalfEdge(*meshObj, halfedgeId);
	}

	ZSPACE_INLINE void zItMeshFace::getOffsetFacePositions(double offset, vector<zVector>& offsetPositions)
	{
		vector<zVector> out;

		zIntArray fVerts;
		getVertices(fVerts);

		for (int j = 0; j < fVerts.size(); j++)
		{
			int next = (j + 1) % fVerts.size();
			int prev = (j - 1 + fVerts.size()) % fVerts.size();


			zVector Ori = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[j]];;
			zVector v1 = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[prev]] - zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[j]];
			v1.normalize();

			zVector v2 = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[next]] - zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[j]];
			v2.normalize();

			zVector v3 = v1;

			v1 = v1 ^ v2;
			v3 = v3 + v2;

			double cs = v3 * v2;

			zVector a1 = v2 * cs;
			zVector a2 = v3 - a1;

			double alpha = sqrt(a2.length() * a2.length());
			if (cs < 0) alpha *= -1;

			double length = offset / alpha;

			zVector mPos = zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[j]];
			zVector offPos = mPos + (v3 * length);

			out.push_back(offPos);

		}

		offsetPositions = out;

	}

	ZSPACE_INLINE void zItMeshFace::getOffsetFacePositions_Variable(vector<double>& offsets, zVector& faceCenter, zVector& faceNormal, vector<zVector>& intersectionPositions)
	{
		vector<zVector> offsetPoints;
		zIntArray fEdges;
		getHalfEdges(fEdges);

		for (int j = 0; j < fEdges.size(); j++)
		{
			zItMeshHalfEdge he(*meshObj, fEdges[j]);


			zVector p2 = zMeshObjectStorage::get(*meshObj).vertexPositions[he.getVertex().getId()];
			zVector p1 = zMeshObjectStorage::get(*meshObj).vertexPositions[he.getSym().getVertex().getId()];

			zVector norm1 = ((p1 - p2) ^ faceNormal);
			norm1.normalize();
			if ((faceCenter - p1) * norm1 < 0) norm1 *= -1;


			offsetPoints.push_back(p1 + norm1 * offsets[j]);
			offsetPoints.push_back(p2 + norm1 * offsets[j]);

		}


		for (int j = 0; j < fEdges.size(); j++)
		{
			int prevId = (j - 1 + fEdges.size()) % fEdges.size();

			zVector a0 = offsetPoints[j * 2];
			zVector a1 = offsetPoints[j * 2 + 1];

			zVector b0 = offsetPoints[prevId * 2];
			zVector b1 = offsetPoints[prevId * 2 + 1];



			double uA = -1;
			double uB = -1;
			bool intersect = zMeshObjectStorage::get(*meshObj).coreUtils.line_lineClosestPoints(a0, a1, b0, b1, uA, uB);

			if (intersect)
			{
				//printf("\n %i working!! ", j);

				zVector closestPt;

				if (uA >= uB)
				{
					zVector dir = a1 - a0;
					double len = dir.length();
					dir.normalize();

					if (uA < 0) dir *= -1;
					closestPt = a0 + dir * len * uA;
				}
				else
				{
					zVector dir = b1 - b0;
					double len = dir.length();
					dir.normalize();

					if (uB < 0) dir *= -1;

					closestPt = b0 + dir * len * uB;
				}


				intersectionPositions.push_back(closestPt);
			}

		}
	}

	ZSPACE_INLINE zVector zItMeshFace::getNormal()
	{
		return zMeshObjectStorage::read(*meshObj).faceNormals[index];
	}

	ZSPACE_INLINE zVector* zItMeshFace::getRawNormal()
	{
		return &zMeshObjectStorage::edit(*meshObj).faceNormals[index];

	}

	ZSPACE_INLINE zColor zItMeshFace::getColor()
	{
		return zMeshObjectStorage::read(*meshObj).faceColors[index];
	}

	ZSPACE_INLINE zColor* zItMeshFace::getRawColor()
	{
		return &zMeshObjectStorage::edit(*meshObj).faceColors[index];
	}

	//---- SET METHODS

	ZSPACE_INLINE void zItMeshFace::setId(int _id)
	{
		if (_id < 0 || _id >= size()) throw std::invalid_argument("face index out of bounds.");
		index = _id;
	}

	ZSPACE_INLINE void zItMeshFace::setHalfEdge(zItMeshHalfEdge &he)
	{
		

		int id = getId();
		int heId = he.getId();

		zMeshObjectStorage::get(*meshObj).faces[index].setHalfEdge(heId);
		zMeshObjectStorage::get(*meshObj).fHandles[id].he = heId;
	}

	ZSPACE_INLINE void zItMeshFace::setColor(zColor col)
	{
		zMeshObjectStorage::edit(*meshObj).faceColors[index] = col;
	}

	ZSPACE_INLINE void zItMeshFace::setNormal(zVector norm)
	{
		zMeshObjectStorage::edit(*meshObj).faceNormals[index] = norm;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE bool zItMeshFace::isActive()
	{
		return meshObj && index >= 0 && index < size();
	}

	ZSPACE_INLINE bool zItMeshFace::checkPointInHalfSpace(zPoint & pt)
	{
		zVector n = getNormal();
		zPoint p = getHalfEdge().getVertex().getPosition();

		double D = n * p* -1;
		double Dis = (pt*n) + D;

		bool out = (Dis / sqrt(n*n) < 0) ? true : false;

		return out;
	}

	ZSPACE_INLINE void zItMeshFace::updateNormal()
	{
		int nV = getNumVertices();
		
		zPoint fCen = getCenter();
		zPointArray points;
		getVertexPositions(points);

		zVector fNorm; // face normal

		if (nV != 3)
		{
			for (int j = 0; j < points.size(); j++)
			{
				fNorm += (points[j] - fCen) ^ (points[(j + 1) % points.size()] - fCen);
			}
		}
		else
		{
			zVector cross = (points[1] - points[0]) ^ (points[points.size() - 1] - points[0]);
			cross.normalize();
			fNorm = cross;
		}
				
		setNormal(fNorm);

	}

	//---- OPERATOR METHODS

	ZSPACE_INLINE bool zItMeshFace::operator==(zItMeshFace &other)
	{
		return (getId() == other.getId());
	}

	ZSPACE_INLINE bool zItMeshFace::operator!=(zItMeshFace &other)
	{
		return (getId() != other.getId());
	}

}

//---- ZIT_MESH_HALFEDGE ------------------------------------------------------------------------------

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zItMeshHalfEdge::zItMeshHalfEdge()
	{
		meshObj = nullptr;
		index = -1;
	}

	ZSPACE_INLINE zItMeshHalfEdge::zItMeshHalfEdge(zObjectMesh &_meshObj)
	{
		meshObj = &_meshObj;
		index = 0;
	}

	ZSPACE_INLINE zItMeshHalfEdge::zItMeshHalfEdge(zObjectMesh &_meshObj, int _index)
	{
		meshObj = &_meshObj;
		index = _index;

		if (_index < 0 || _index >= zMeshObjectStorage::get(*meshObj).halfEdges.size()) throw std::invalid_argument(" error: index out of bounds");
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zItMeshHalfEdge::begin()
	{
		index = 0;
	}

	ZSPACE_INLINE void zItMeshHalfEdge::operator++(int)
	{
		index++;
	}

	ZSPACE_INLINE void zItMeshHalfEdge::operator--(int)
	{
		index--;
	}

	ZSPACE_INLINE bool zItMeshHalfEdge::end()
	{
		return !meshObj || index >= static_cast<int>(zMeshObjectStorage::get(*meshObj).halfEdges.size());
	}

	ZSPACE_INLINE void zItMeshHalfEdge::reset()
	{
		index = 0;
	}

	ZSPACE_INLINE int zItMeshHalfEdge::size()
	{
		if (!meshObj) return 0;
		return zMeshObjectStorage::get(*meshObj).halfEdges.size();
	}

	ZSPACE_INLINE void zItMeshHalfEdge::deactivate()
	{
		if (!isActive()) return;
		auto& meshData = zMeshObjectStorage::get(*meshObj);
		meshData.heHandles[index] = zHalfEdgeHandle();
		meshData.halfEdges[index].reset();
	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE zItMeshVertex zItMeshHalfEdge::getStartVertex()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");

		return getSym().getVertex();
	}

	ZSPACE_INLINE void zItMeshHalfEdge::getVertices(zItMeshVertexArray &verticies)
	{
		verticies.push_back(getVertex());
		verticies.push_back(getSym().getVertex());
	}

	ZSPACE_INLINE void zItMeshHalfEdge::getVertices(zIntArray &vertexIndicies)
	{
		vertexIndicies.push_back(getVertex().getId());
		vertexIndicies.push_back(getSym().getVertex().getId());
	}

	ZSPACE_INLINE void zItMeshHalfEdge::getVertexPositions(vector<zVector> &vertPositions)
	{
		zIntArray eVerts;

		getVertices(eVerts);

		for (int i = 0; i < eVerts.size(); i++)
		{
			vertPositions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[eVerts[i]]);
		}
	}

	ZSPACE_INLINE void zItMeshHalfEdge::getConnectedHalfEdges(zItMeshHalfEdgeArray& edgeIndicies)
	{
		zItMeshVertex v1 = getVertex();
		zItMeshHalfEdgeArray connectedEdgestoVert0;
		v1.getConnectedHalfEdges(connectedEdgestoVert0);

		zItMeshVertex v2 = getSym().getVertex();
		zItMeshHalfEdgeArray connectedEdgestoVert1;
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

	ZSPACE_INLINE void zItMeshHalfEdge::getConnectedHalfEdges(zIntArray& edgeIndicies)
	{
		zItMeshVertex v1 = getVertex();
		zIntArray connectedEdgestoVert0;
		v1.getConnectedHalfEdges(connectedEdgestoVert0);

		zItMeshVertex v2 = getSym().getVertex();
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

	ZSPACE_INLINE void zItMeshHalfEdge::getFaces(zItMeshFaceArray &faceIndicies)
	{
		this->getEdge().getFaces(faceIndicies);
	}

	ZSPACE_INLINE void zItMeshHalfEdge::getFaces(zIntArray &faceIndicies)
	{
		this->getEdge().getFaces(faceIndicies);
	}

	ZSPACE_INLINE bool zItMeshHalfEdge::onBoundary()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		return zMeshObjectStorage::get(*meshObj).halfEdges[index].getFace() == -1;
	}

	ZSPACE_INLINE zVector zItMeshHalfEdge::getCenter()
	{
		zIntArray eVerts;
		getVertices(eVerts);

		return (zMeshObjectStorage::get(*meshObj).vertexPositions[eVerts[0]] + zMeshObjectStorage::get(*meshObj).vertexPositions[eVerts[1]]) * 0.5;
	}

	ZSPACE_INLINE zVector zItMeshHalfEdge::getVector()
	{

		int v1 = getVertex().getId();
		int v2 = getSym().getVertex().getId();

		zVector out = zMeshObjectStorage::get(*meshObj).vertexPositions[v1] - (zMeshObjectStorage::get(*meshObj).vertexPositions[v2]);

		return out;
	}

	ZSPACE_INLINE double zItMeshHalfEdge::getLength()
	{
		return getVector().length();
	}

	//---- GET METHODS

	ZSPACE_INLINE int zItMeshHalfEdge::getId()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		return index;
	}

	ZSPACE_INLINE zItMeshHalfEdge zItMeshHalfEdge::getSym()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		const int symId = zMeshObjectStorage::get(*meshObj).halfEdges[index].getSym();
		if(symId == -1) throw std::invalid_argument(" error: sym index is -1. "); 
		return zItMeshHalfEdge(*meshObj, symId);
	}

	ZSPACE_INLINE zItMeshHalfEdge zItMeshHalfEdge::getNext()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		const int nextId = zMeshObjectStorage::get(*meshObj).halfEdges[index].getNext();
		if (nextId == -1) throw std::invalid_argument(" error: next index is -1. ");
		return zItMeshHalfEdge(*meshObj, nextId);
	}

	ZSPACE_INLINE zItMeshHalfEdge zItMeshHalfEdge::getPrev()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		const int prevId = zMeshObjectStorage::get(*meshObj).halfEdges[index].getPrev();
		if (prevId == -1) throw std::invalid_argument(" error: prev index is -1. ");
		return zItMeshHalfEdge(*meshObj, prevId);
	}

	ZSPACE_INLINE zItMeshVertex zItMeshHalfEdge::getVertex()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		const int vertexId = zMeshObjectStorage::get(*meshObj).halfEdges[index].getVertex();
		if (vertexId == -1) throw std::invalid_argument(" error: vertex index is -1. ");
		return zItMeshVertex(*meshObj, vertexId);
	}

	ZSPACE_INLINE zItMeshFace zItMeshHalfEdge::getFace()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		const int faceId = zMeshObjectStorage::get(*meshObj).halfEdges[index].getFace();
		if (faceId == -1) throw std::invalid_argument(" error: face index is -1. ");
		return zItMeshFace(*meshObj, faceId);
	}

	ZSPACE_INLINE zItMeshEdge zItMeshHalfEdge::getEdge()
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		const int edgeId = zMeshObjectStorage::get(*meshObj).halfEdges[index].getEdge();
		if (edgeId == -1) throw std::invalid_argument(" error: edge index is -1. ");
		return zItMeshEdge(*meshObj, edgeId);
	}

	ZSPACE_INLINE zColor zItMeshHalfEdge::getColor()
	{
		zItMeshEdge edge = getEdge();
		return zMeshObjectStorage::get(*meshObj).edgeColors[edge.getId()];
	}

	ZSPACE_INLINE zColor* zItMeshHalfEdge::getRawColor()
	{
		zItMeshEdge edge = getEdge();
		return &zMeshObjectStorage::get(*meshObj).edgeColors[edge.getId()];
	}

	//---- SET METHODS

	ZSPACE_INLINE void zItMeshHalfEdge::setId(int _id)
	{
		if (!isActive()) throw std::invalid_argument(" error: out of bounds.");
		zMeshObjectStorage::get(*meshObj).halfEdges[index].setId(_id);
		index = _id;
	}

	ZSPACE_INLINE void zItMeshHalfEdge::setSym(zItMeshHalfEdge &he)
	{
		auto& meshData = zMeshObjectStorage::get(*meshObj);
		const int id = getId();
		const int symId = he.getId();
		meshData.halfEdges[id].setSym(symId);
		meshData.halfEdges[symId].setSym(id);
	}

	ZSPACE_INLINE void zItMeshHalfEdge::setNext(zItMeshHalfEdge &he)
	{
		

		int id = getId();
		int nextId = he.getId();

		auto& meshData = zMeshObjectStorage::get(*meshObj);
		meshData.halfEdges[id].setNext(nextId);
		//he.setPrev(*this);
		meshData.halfEdges[nextId].setPrev(id);

		meshData.heHandles[id].n = nextId;
		meshData.heHandles[nextId].p = id;
	}

	ZSPACE_INLINE void zItMeshHalfEdge::setPrev(zItMeshHalfEdge &he)
	{
		

		int id = getId();
		int prevId = he.getId();

		auto& meshData = zMeshObjectStorage::get(*meshObj);
		meshData.halfEdges[id].setPrev(prevId);
		//he.setNext(*this);
		meshData.halfEdges[prevId].setNext(id);

		meshData.heHandles[id].p = prevId;
		meshData.heHandles[prevId].n = id;
	}

	ZSPACE_INLINE void zItMeshHalfEdge::setVertex(zItMeshVertex &v)
	{
		int id = getId();
		int vId = v.getId();
		auto& meshData = zMeshObjectStorage::get(*meshObj);

		meshData.halfEdges[id].setVertex(vId);
		meshData.heHandles[id].v = vId;
	}

	ZSPACE_INLINE void zItMeshHalfEdge::setEdge(zItMeshEdge &e)
	{
		int id = getId();
		int eId = e.getId();
		auto& meshData = zMeshObjectStorage::get(*meshObj);

		meshData.halfEdges[id].setEdge(eId);
		meshData.heHandles[id].e = eId;
	}

	ZSPACE_INLINE void zItMeshHalfEdge::setFace(zItMeshFace &f)
	{
		int id = getId();
		int fId = f.getId();
		auto& meshData = zMeshObjectStorage::get(*meshObj);

		meshData.halfEdges[id].setFace(fId);
		meshData.heHandles[id].f = fId;
	}

	//---- UTILITY METHODS

	ZSPACE_INLINE bool zItMeshHalfEdge::isActive()
	{
		if (!meshObj) return false;
		const auto& halfEdges = zMeshObjectStorage::get(*meshObj).halfEdges;
		return index >= 0 && index < static_cast<int>(halfEdges.size()) && halfEdges[index].isActive();
	}

	//---- OPERATOR METHODS

	ZSPACE_INLINE bool zItMeshHalfEdge::operator==(zItMeshHalfEdge &other)
	{
		return (getId() == other.getId());
	}

	ZSPACE_INLINE bool zItMeshHalfEdge::operator!=(zItMeshHalfEdge &other)
	{
		return (getId() != other.getId());
	}

}
