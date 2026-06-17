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


#include<zspace/zInterface/functionsets/zFnMesh.h>

#include<zspace/zInterface/functionsets/zFnGraph.h>
#include <src/zInterface/objects/zMeshObjectStorage.h>
#include <src/zCore/geometry/detail/zMeshStorage.h>

#include <algorithm>
#include <cmath>

namespace zSpace
{
	namespace
	{
		const double ZSPACE_CURVATURE_EPS = 1.0e-12;

		double clampUnit(double value)
		{
			return std::max(-1.0, std::min(1.0, value));
		}

		double vectorLength(zVector v)
		{
			return std::sqrt((double)v.x * v.x + (double)v.y * v.y + (double)v.z * v.z);
		}

		zVector normalized(zVector v)
		{
			double len = vectorLength(v);
			if (len <= ZSPACE_CURVATURE_EPS) return zVector();

			v /= (float)len;
			return v;
		}

		double angleAtVertex(zVector center, zVector prev, zVector next)
		{
			zVector a = prev - center;
			zVector b = next - center;

			double aLen = vectorLength(a);
			double bLen = vectorLength(b);
			if (aLen <= ZSPACE_CURVATURE_EPS || bLen <= ZSPACE_CURVATURE_EPS) return 0.0;

			double dot = (double)(a * b);
			return std::acos(clampUnit(dot / (aLen * bLen)));
		}

		double triangleArea(zVector a, zVector b, zVector c)
		{
			zVector ab = b - a;
			zVector ac = c - a;
			return 0.5 * vectorLength(ab ^ ac);
		}

		double polygonArea(const zVectorArray& positions)
		{
			if (positions.size() < 3) return 0.0;

			double area = 0.0;
			for (size_t i = 1; i + 1 < positions.size(); ++i)
			{
				area += triangleArea(positions[0], positions[i], positions[i + 1]);
			}

			return area;
		}

		void computeVertexCurvatureData(
			zFnMesh& meshFn,
			zItMeshVertex& vertex,
			double& gaussianCurvature,
			double& meanCurvature,
			zVector& tangentDirection)
		{
			gaussianCurvature = 0.0;
			meanCurvature = 0.0;
			tangentDirection = zVector();

			if (!vertex.isActive()) return;

			zVector position = vertex.getPosition();
			double angleSum = 0.0;
			double vertexArea = 0.0;

			zItMeshFaceArray connectedFaces;
			vertex.getConnectedFaces(connectedFaces);

			for (auto& face : connectedFaces)
			{
				zItMeshVertexArray faceVertices;
				face.getVertices(faceVertices);
				if (faceVertices.size() < 3) continue;

				int localIndex = -1;
				for (int i = 0; i < (int)faceVertices.size(); ++i)
				{
					if (faceVertices[i].getId() == vertex.getId())
					{
						localIndex = i;
						break;
					}
				}

				if (localIndex < 0) continue;

				zVectorArray facePositions;
				face.getVertexPositions(facePositions);

				int previousIndex = (localIndex + (int)faceVertices.size() - 1) % (int)faceVertices.size();
				int nextIndex = (localIndex + 1) % (int)faceVertices.size();
				angleSum += angleAtVertex(position, facePositions[previousIndex], facePositions[nextIndex]);

				vertexArea += polygonArea(facePositions) / (double)faceVertices.size();
			}

			if (vertexArea > ZSPACE_CURVATURE_EPS)
			{
				double targetAngle = vertex.onBoundary() ? Z_PI : Z_TWO_PI;
				gaussianCurvature = (targetAngle - angleSum) / vertexArea;
			}

			zItMeshHalfEdgeArray connectedHalfEdges;
			vertex.getConnectedHalfEdges(connectedHalfEdges);

			zVector meanNormal;
			for (auto& halfEdge : connectedHalfEdges)
			{
				zItMeshVertex neighbour = halfEdge.getVertex();
				if (!neighbour.isActive()) continue;

				double weight = meshFn.getEdgeCotangentWeight(halfEdge);
				if (!std::isfinite(weight)) continue;

				zVector edgeVector = position - neighbour.getPosition();
				meanNormal += edgeVector * (float)weight;

				if (vectorLength(tangentDirection) <= ZSPACE_CURVATURE_EPS)
				{
					tangentDirection = neighbour.getPosition() - position;
				}
			}

			if (vertexArea > ZSPACE_CURVATURE_EPS)
			{
				meanNormal /= (float)(2.0 * vertexArea);
				meanCurvature = 0.5 * vectorLength(meanNormal);
			}

			zVector normal = normalized(vertex.getNormal());
			if (vectorLength(normal) > ZSPACE_CURVATURE_EPS && vectorLength(tangentDirection) > ZSPACE_CURVATURE_EPS)
			{
				tangentDirection -= normal * (float)(tangentDirection * normal);
			}
			tangentDirection = normalized(tangentDirection);
		}
	}

	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnMesh::zFnMesh()
	{
		fnType = zFnType::zMeshFn; 
		meshObj = nullptr;
	}

	ZSPACE_INLINE zFnMesh::zFnMesh(zObjMesh &_meshObj)
	{
		fnType = zFnType::zMeshFn;
		setObject(_meshObj);
	}

	ZSPACE_INLINE void zFnMesh::setObject(zObjectMesh &_meshObject)
	{
		meshObj = &_meshObject;
	}

	ZSPACE_INLINE zObjectMesh* zFnMesh::object() const
	{
		return meshObj;
	}

	ZSPACE_INLINE bool zFnMesh::hasObject() const
	{
		return meshObj != nullptr;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnMesh::~zFnMesh() {}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE zFnType zFnMesh::getType()
	{
		return zMeshFn;
	}

#if defined ZSPACE_USD_INTEROP

#endif

	ZSPACE_INLINE void zFnMesh::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		coreUtils.getBounds(zMeshObjectStorage::get(*meshObj).vertexPositions, minBB, maxBB);
	}

	ZSPACE_INLINE void zFnMesh::clear()
	{
		zMeshObjectStorage::get(*meshObj).clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnMesh::reserve(int _n_v, int  _n_e, int _n_f)
	{
		zMeshObjectStorage::get(*meshObj).clear();

		zMeshObjectStorage::get(*meshObj).vertices.reserve(_n_v);
		zMeshObjectStorage::get(*meshObj).faces.reserve(_n_f);
		zMeshObjectStorage::get(*meshObj).edges.reserve(_n_e);
		zMeshObjectStorage::get(*meshObj).halfEdges.reserve(_n_e * 2);
	}

	ZSPACE_INLINE void zFnMesh::create(zPointArray& _positions, zIntArray& polyCounts, zIntArray& polyConnects, bool staticMesh)
	{

		zMeshObjectStorage::get(*meshObj).create(_positions, polyCounts, polyConnects);
			
		// compute mesh normals
		computeMeshNormals();

		if (staticMesh) setStaticContainers();
	}

	ZSPACE_INLINE bool zFnMesh::addVertex(zPoint &_pos, bool checkDuplicates, zItMeshVertex &vertex)
	{
		if (checkDuplicates)
		{
			int id;
			bool chk = vertexExists(_pos, vertex);
			if (chk)	return false;

		}

		bool out = zMeshObjectStorage::get(*meshObj).addVertex(_pos);
		vertex = zItMeshVertex(*meshObj, numVertices() - 1);

		return out;
	}

	ZSPACE_INLINE bool zFnMesh::addEdges(int v1, int v2, bool checkDuplicates, zItMeshHalfEdge &halfEdge)
	{
		if (v1 < 0 && v1 >= numVertices()) throw std::invalid_argument(" error: index out of bounds");
		if (v2 < 0 && v2 >= numVertices()) throw std::invalid_argument(" error: index out of bounds");

		if (checkDuplicates)
		{
			int id;
			bool chk = halfEdgeExists(v1, v2, id);
			if (chk)
			{
				halfEdge = zItMeshHalfEdge(*meshObj, id);
				return false;
			}
		}

		bool out = zMeshObjectStorage::get(*meshObj).addEdges(v1, v2);

		halfEdge = zItMeshHalfEdge(*meshObj, numHalfEdges() - 2);

		

		return out;
	}

	ZSPACE_INLINE bool zFnMesh::addPolygon(zIntArray &fVertices, zItMeshFace &face)
	{
		for (auto &v : fVertices)
		{
			if (v < 0 && v >= numVertices()) throw std::invalid_argument(" error: index out of bounds");
		}

		bool out = zMeshObjectStorage::get(*meshObj).addPolygon(fVertices);
			
		

		face = zItMeshFace(*meshObj, numPolygons() - 1);

		return out;
	}

	ZSPACE_INLINE bool zFnMesh::addPolygon(zPointArray &fVertices, zItMeshFace &face)
	{
		vector<int> fVerts;

		for (auto &v : fVertices)
		{
			zItMeshVertex vId;
			addVertex(v, true, vId);
			fVerts.push_back(vId.getId());
		}

		return addPolygon(fVerts, face);
	}

	ZSPACE_INLINE bool zFnMesh::addPolygon(zItMeshFace &face)
	{
		bool out = zMeshObjectStorage::get(*meshObj).addPolygon();
		face = zItMeshFace(*meshObj, numPolygons() - 1);

		return out;
	}

	ZSPACE_INLINE bool zFnMesh::updatePolygon(zItMeshFace& face, zIntArray& fVertices)
	{
		for (auto& v : fVertices)
		{
			if (v < 0 && v >= numVertices()) throw std::invalid_argument(" error: index out of bounds");
		}

		bool out = zMeshObjectStorage::get(*meshObj).updatePolygon(face.getId(), fVertices);
		

		return out;
	}

	//--- TOPOLOGY QUERY METHODS 

	ZSPACE_INLINE int zFnMesh::numVertices()
	{
		return zMeshObjectStorage::get(*meshObj).n_v;
	}

	ZSPACE_INLINE int zFnMesh::numEdges()
	{
		return zMeshObjectStorage::get(*meshObj).n_e;
	}

	ZSPACE_INLINE int zFnMesh::numHalfEdges()
	{
		return zMeshObjectStorage::get(*meshObj).n_he;
	}

	ZSPACE_INLINE int zFnMesh::numPolygons()
	{
		return zMeshObjectStorage::get(*meshObj).n_f;
	}

	ZSPACE_INLINE bool zFnMesh::vertexExists(zPoint pos, zItMeshVertex &outVertex, int precisionfactor)
	{
		int id;
		bool chk = zMeshObjectStorage::get(*meshObj).vertexExists(pos, id, precisionfactor);

		if (chk) outVertex = zItMeshVertex(*meshObj, id);

		return chk;
	}
	
	ZSPACE_INLINE bool zFnMesh::halfEdgeExists(int v1, int v2, int &outHalfEdgeId)
	{
		return zMeshObjectStorage::get(*meshObj).halfEdgeExists(v1, v2, outHalfEdgeId);
	}

	ZSPACE_INLINE bool zFnMesh::halfEdgeExists(int v1, int v2, zItMeshHalfEdge &outHalfEdge)
	{
		int id;
		bool chk = halfEdgeExists(v1, v2, id);

		if (chk) outHalfEdge = zItMeshHalfEdge(*meshObj, id);

		return chk;
	}

	ZSPACE_INLINE zSparseMatrix zFnMesh::getTopologicalLaplacian()
	{
		int n_v = numVertices();

		//MatrixXd meshLaplacian(n_v, n_v);
		//meshLaplacian.setZero();

		zSparseMatrix meshLaplacian(n_v, n_v);
		//meshLaplacian.setZero();


		// compute laplacian weights
		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			vector<zItMeshHalfEdge> cEdges;
			v.getConnectedHalfEdges(cEdges);

			int j = v.getId();

			double out = 0;

			for (int k = 0; k < cEdges.size(); k++)
			{

				double val = getEdgeCotangentWeight(cEdges[k]) * 0.5;
				out += val;

				int i = cEdges[k].getVertex().getId();

				meshLaplacian.insert(j, i) = val * -1;
			}

			meshLaplacian.insert(j, j) = out;

			/*for (int i = 0; i < n_v; i++)
			{
				if (i == j)
				{
					vector<int> cEdges;
					getConnectedEdges(i, zVertexData, cEdges);

					double out = 0;

					for (int k = 0; k < cEdges.size(); k++)
					{
						out += getEdgeCotangentWeight(cEdges[k]) * 0.5;
					}

					meshLaplacian(j, i) = out;
				}
				else
				{
					int e;
					bool chk = edgeExists(i, j, e);

					if (chk)  meshLaplacian(j, i) = getEdgeCotangentWeight(e) * 0.5 * -1;
					else meshLaplacian(j, i) = 0;
				}

			}*/
		}

		return meshLaplacian;
	}

	ZSPACE_INLINE double zFnMesh::getEdgeCotangentWeight(zItMeshHalfEdge &he)
	{
		zItMeshVertex i = he.getStartVertex();
		zItMeshVertex j = he.getVertex();

		zVector* pt = i.getRawPosition();

		zVector* pt1 = j.getRawPosition();

		zItMeshHalfEdge nextEdge = he.getSym().getNext();
		zItMeshVertex nextVert = nextEdge.getVertex();;
		zVector* pt2 = nextVert.getRawPosition();

		zItMeshHalfEdge prevEdge = he.getPrev().getSym();
		zItMeshVertex prevVert = prevEdge.getVertex();
		zVector* pt3 = prevVert.getRawPosition();

		zVector alpha1 = (*pt - *pt2);
		zVector alpha2 = (*pt1 - *pt2);
		double coTan_alpha = alpha1.cotan(alpha2);

		zVector beta1 = (*pt - *pt3);
		zVector beta2 = (*pt1 - *pt3);
		double coTan_beta = beta1.cotan(beta2);

		if (he.onBoundary())coTan_beta = 0;;
		if (he.getSym().onBoundary())coTan_alpha = 0;;

		double wt = coTan_alpha + coTan_beta;

		if (isnan(wt)) wt = 0.0;

		return wt;
	}

	ZSPACE_INLINE bool zFnMesh::checkPointInConvexHull(zPoint &pt)
	{
		bool out = true;

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			out = f.checkPointInHalfSpace(pt);
			if (!out) break;
		}

		return out;
	}

	//--- COMPUTE METHODS 

	ZSPACE_INLINE void zFnMesh::computeEdgeColorfromVertexColor()
	{
		for (zItMeshEdge e(*meshObj); !e.end(); e++)
		{
			if (e.isActive())
			{
				int v0 = e.getHalfEdge(0).getVertex().getId();
				int v1 = e.getHalfEdge(0).getVertex().getId();

				zColor col;
				col.r = (zMeshObjectStorage::get(*meshObj).vertexColors[v0].r + zMeshObjectStorage::get(*meshObj).vertexColors[v1].r) * 0.5;
				col.g = (zMeshObjectStorage::get(*meshObj).vertexColors[v0].g + zMeshObjectStorage::get(*meshObj).vertexColors[v1].g) * 0.5;
				col.b = (zMeshObjectStorage::get(*meshObj).vertexColors[v0].b + zMeshObjectStorage::get(*meshObj).vertexColors[v1].b) * 0.5;
				col.a = (zMeshObjectStorage::get(*meshObj).vertexColors[v0].a + zMeshObjectStorage::get(*meshObj).vertexColors[v1].a) * 0.5;

				if (zMeshObjectStorage::get(*meshObj).edgeColors.size() <= e.getId()) zMeshObjectStorage::get(*meshObj).edgeColors.push_back(col);
				else zMeshObjectStorage::get(*meshObj).edgeColors[e.getId()] = col;
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::computeVertexColorfromEdgeColor()
	{
		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			if (v.isActive())
			{
				vector<int> cEdges;
				v.getConnectedHalfEdges(cEdges);

				zColor col;
				for (int j = 0; j < cEdges.size(); j++)
				{
					col.r += zMeshObjectStorage::get(*meshObj).edgeColors[cEdges[j]].r;
					col.g += zMeshObjectStorage::get(*meshObj).edgeColors[cEdges[j]].g;
					col.b += zMeshObjectStorage::get(*meshObj).edgeColors[cEdges[j]].b;
				}

				col.r /= cEdges.size(); col.g /= cEdges.size(); col.b /= cEdges.size();

				zMeshObjectStorage::get(*meshObj).vertexColors[v.getId()] = col;
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::computeFaceColorfromVertexColor()
	{
		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			if (f.isActive())
			{
				vector<int> fVerts;
				f.getVertices(fVerts);

				zColor col;
				for (int j = 0; j < fVerts.size(); j++)
				{
					col.r += zMeshObjectStorage::get(*meshObj).vertexColors[fVerts[j]].r;
					col.g += zMeshObjectStorage::get(*meshObj).vertexColors[fVerts[j]].g;
					col.b += zMeshObjectStorage::get(*meshObj).vertexColors[fVerts[j]].b;
				}

				col.r /= fVerts.size(); col.g /= fVerts.size(); col.b /= fVerts.size();

				zMeshObjectStorage::get(*meshObj).faceColors[f.getId()] = col;
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::computeVertexColorfromFaceColor()
	{
		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			if (v.isActive())
			{
				vector<int> cFaces;
				v.getConnectedFaces(cFaces);

				zColor col;
				for (int j = 0; j < cFaces.size(); j++)
				{
					col.r += zMeshObjectStorage::get(*meshObj).faceColors[cFaces[j]].r;
					col.g += zMeshObjectStorage::get(*meshObj).faceColors[cFaces[j]].g;
					col.b += zMeshObjectStorage::get(*meshObj).faceColors[cFaces[j]].b;
				}

				col.r /= cFaces.size(); col.g /= cFaces.size(); col.b /= cFaces.size();

				zMeshObjectStorage::get(*meshObj).vertexColors[v.getId()] = col;
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::smoothColors(int smoothVal, zHEData type)
	{
		for (int j = 0; j < smoothVal; j++)
		{
			if (type == zVertexData)
			{
				vector<zColor> tempColors;

				for (zItMeshVertex v(*meshObj); !v.end(); v++)
				{
					zColor col;
					//if (v.isActive())
					//{
						vector<int> cVerts;
						v.getConnectedVertices(cVerts);

						zColor currentCol = zMeshObjectStorage::get(*meshObj).vertexColors[v.getId()];


						for (int j = 0; j < cVerts.size(); j++)
						{
							col.r += zMeshObjectStorage::get(*meshObj).vertexColors[cVerts[j]].r;
							col.g += zMeshObjectStorage::get(*meshObj).vertexColors[cVerts[j]].g;
							col.b += zMeshObjectStorage::get(*meshObj).vertexColors[cVerts[j]].b;
						}

						col.r += (currentCol.r); col.g += (currentCol.g); col.b += (currentCol.b);

						col.r /= cVerts.size(); col.g /= cVerts.size(); col.b /= cVerts.size();
					//}

					tempColors.push_back(col);
				}

				for (zItMeshVertex v(*meshObj); !v.end(); v++)
				{
					//if (v.isActive())
					//{
						zMeshObjectStorage::get(*meshObj).vertexColors[v.getId()] = (tempColors[v.getId()]);
					//}
				}
			}

			else if (type == zFaceData)
			{
				vector<zColor> tempColors;

				for (zItMeshFace f(*meshObj); !f.end(); f++)
				{
					zColor col;
					if (f.isActive())
					{
						vector<int> cFaces;
						f.getConnectedFaces(cFaces);

						zColor currentCol = zMeshObjectStorage::get(*meshObj).faceColors[f.getId()];
						for (int j = 0; j < cFaces.size(); j++)
						{
							col.r += zMeshObjectStorage::get(*meshObj).faceColors[cFaces[j]].r;
							col.g += zMeshObjectStorage::get(*meshObj).faceColors[cFaces[j]].g;
							col.b += zMeshObjectStorage::get(*meshObj).faceColors[cFaces[j]].b;
						}

						col.r += (currentCol.r); col.g += (currentCol.g); col.b += (currentCol.b);
						col.r /= cFaces.size(); col.g /= cFaces.size(); col.b /= cFaces.size();
					}

					tempColors.push_back(col);
				}

				for (zItMeshFace f(*meshObj); !f.end(); f++)
				{
					if (f.isActive())
					{
						zMeshObjectStorage::get(*meshObj).faceColors[f.getId()] = (tempColors[f.getId()]);
					}
				}
			}

			else throw std::invalid_argument(" error: invalid zHEData type");
		}
	}

	ZSPACE_INLINE void zFnMesh::computeVertexNormalfromFaceNormal()
	{
		zMeshObjectStorage::get(*meshObj).vertexNormals.clear();

		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			if (v.isActive())
			{

				//if (v.getHalfEdge().onBoundary()) v.setHalfEdge(v.getHalfEdge().getSym().getNext());

				vector<int> cFaces;
				v.getConnectedFaces(cFaces);

				zVector norm;

				for (int j = 0; j < cFaces.size(); j++)
				{
					norm += zMeshObjectStorage::get(*meshObj).faceNormals[cFaces[j]];
				}

				norm /= cFaces.size();
				norm.normalize();
				zMeshObjectStorage::get(*meshObj).vertexNormals.push_back(norm);
			}
			else zMeshObjectStorage::get(*meshObj).vertexNormals.push_back(zVector());
		}
	}

	ZSPACE_INLINE void zFnMesh::computeMeshNormals()
	{
		zMeshObjectStorage::get(*meshObj).faceNormals.clear();

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			if (f.isActive())
			{
				// get face vertices and correspondiing positions

				//printf("\n f %i :", f.getId());
				vector<int> fVerts;
				f.getVertices(fVerts);

				zVector fCen; // face center

				vector<zVector> points;
				for (int i = 0; i < fVerts.size(); i++)
				{
					//printf(" %i ", fVerts[i]);
					points.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[i]]);

					fCen += zMeshObjectStorage::get(*meshObj).vertexPositions[fVerts[i]];
				}

				fCen /= fVerts.size();

				zVector fNorm; // face normal

				if (fVerts.size() != 3)
				{
					for (int j = 0; j < fVerts.size(); j++)
					{
						fNorm += (points[j] - fCen) ^ (points[(j + 1) % fVerts.size()] - fCen);
					}


					//  https://stackoverflow.com/questions/27326636/calculate-normal-vector-of-a-polygon-newells-method
					/*for (int j = 0; j < fVerts.size(); j++) 
					{
						int k = (j + 1) % (fVerts.size());
						fNorm.x += (points[j].y - points[k].y) * (points[j].z + points[k].z);
						fNorm.y += (points[j].z - points[k].z) * (points[j].x + points[k].x);
						fNorm.z += (points[j].x - points[k].x) * (points[j].y + points[k].y);						
					}*/
					


				}
				else
				{
					zVector cross = (points[1] - points[0]) ^ (points[fVerts.size() - 1] - points[0]);
					cross.normalize();

					fNorm = cross;

					//printf("\n working! %i ", i);
				}

				fNorm.normalize();
				zMeshObjectStorage::get(*meshObj).faceNormals.push_back(fNorm);

							}
			else zMeshObjectStorage::get(*meshObj).faceNormals.push_back(zVector());
		}
		// compute vertex normal
		computeVertexNormalfromFaceNormal();

		
	}

	ZSPACE_INLINE void zFnMesh::averageVertices(int numSteps)
	{
		for (int k = 0; k < numSteps; k++)
		{
			vector<zVector> tempVertPos;

			for (zItMeshVertex v(*meshObj); !v.end(); v++)
			{
				tempVertPos.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[v.getId()]);

				if (v.isActive())
				{
					if (!v.checkValency(1))
					{
						vector<int> cVerts;

						v.getConnectedVertices(cVerts);

						for (int j = 0; j < cVerts.size(); j++)
						{
							zVector p = zMeshObjectStorage::get(*meshObj).vertexPositions[cVerts[j]];
							tempVertPos[v.getId()] += p;
						}

						tempVertPos[v.getId()] /= (cVerts.size() + 1);
					}
				}
			}

			// update position
			for (int i = 0; i < tempVertPos.size(); i++) zMeshObjectStorage::get(*meshObj).vertexPositions[i] = tempVertPos[i];
		}
	}

	ZSPACE_INLINE void zFnMesh::garbageCollection(zHEData type)
	{
		removeInactive(type);
	}

	ZSPACE_INLINE void zFnMesh::makeStatic()
	{
		setStaticContainers();
	}

	ZSPACE_INLINE void zFnMesh::makeConvexHull(zPointArray &_pts)
	{
		int num = _pts.size();
		qh_vertex_t *vertices = new qh_vertex_t[num];

		for (int i = 0; i < num; ++i)
		{
			vertices[i].x = _pts[i].x;
			vertices[i].y = _pts[i].y;
			vertices[i].z = _pts[i].z;
		}

		qh_mesh_t mesh = qh_quickhull3d(vertices, num);

		//reserve(mesh.nvertices, floor(mesh.nvertices / 3), floor(mesh.nvertices / 3));

		zPointArray positions;
		zIntArray pCounts, pConnects;

		for (int i = 0; i < mesh.nvertices; i += 3)
		{
			//zItMeshFace f(*meshObj);
			//zPointArray pts;
			
			/*pts.push_back(zVector(mesh.vertices[i + 0].x, mesh.vertices[i + 0].y, mesh.vertices[i + 0].z));
			pts.push_back(zVector(mesh.vertices[i + 1].x, mesh.vertices[i + 1].y, mesh.vertices[i + 1].z));
			pts.push_back(zVector(mesh.vertices[i + 2].x, mesh.vertices[i + 2].y, mesh.vertices[i + 2].z));*/
			//addPolygon(pts, f);

			printf("\n ");
			for (int j = 0; j < 3; j += 1)
			{
				int vID = -1;

				zPoint pt(mesh.vertices[i + j].x, mesh.vertices[i + j].y, mesh.vertices[i + j].z);
				bool check = coreUtils.checkRepeatElement(pt, positions, vID);

				if (!check)
				{
					vID = positions.size();
					positions.push_back(pt);
				}

				pConnects.push_back(vID);
				printf(" %i ", vID);
			}

			pCounts.push_back(3);
		}
		printf("\n Working %i %i %i ", positions.size(), pCounts.size(), pConnects.size());
		create(positions, pCounts, pConnects);
		

		//computeMeshNormals();
	}

	ZSPACE_INLINE bool zFnMesh::isTriMesh()
	{
		bool out = true;

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			if (f.getNumVertices() != 3)
			{
				out = false;
				break;
			}
		}

		return out;
	}

	ZSPACE_INLINE bool zFnMesh::isQuadMesh()
	{
		bool out = true;

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			if (f.getNumVertices() != 4)
			{
				out = false;
				break;
			}
		}

		return out;
	}

	ZSPACE_INLINE void zFnMesh::computeEdgeLoop(zItMeshHalfEdge& heStart, vector<zItMeshHalfEdge>& _heLoop)
	{
		_heLoop.clear();

		bool heBoundary = heStart.onBoundary();

		bool exit = false;
		zItMeshHalfEdge he = heStart;

		do
		{
			_heLoop.push_back(he);
			
			if (he.getVertex().onBoundary()) exit = true;
			if (he.getVertex().getValence() != 4 ) exit = true;

			if (!exit) he = (heBoundary)? he.getNext() :  he.getNext().getSym().getNext();

			if (he == heStart) exit = true;

		} while (!exit);
	}

	ZSPACE_INLINE void zFnMesh::computeEdgeLoop_Split(vector<zItMeshHalfEdge> &_heLoop, int divs, vector<zPoint> &divPoints)
	{
		divPoints.clear();

		float length = computeEdgeLoop_Length(_heLoop);
		float actualWidth = length / divs;

		
		divPoints.push_back(_heLoop[0].getStartVertex().getPosition()); // first
		int currentindex = 0;

		//////////////////////// loop 
		int currentIndex = 0;
		zItMeshHalfEdge walkHe = _heLoop[currentIndex];
		zPoint pOnCurve = walkHe.getStartVertex().getPosition();;
		bool exit = false;

		zPoint start;

		float dStart = 0;
		float dIncrement = actualWidth;
		while (!exit)
		{
			zPoint eEndPoint = walkHe.getVertex().getPosition();
			dStart += dIncrement;
			float distance_increment = dIncrement;
			while (pOnCurve.distanceTo(eEndPoint) < distance_increment)
			{
				distance_increment = distance_increment - pOnCurve.distanceTo(eEndPoint);
				pOnCurve = eEndPoint;

				currentIndex++;
				walkHe = _heLoop[currentIndex];
				eEndPoint = walkHe.getVertex().getPosition();
			}

			zVector he_vec = walkHe.getVector();
			he_vec.normalize();

			start = pOnCurve + he_vec * distance_increment;
			pOnCurve = start;

			divPoints.push_back(pOnCurve);

			if (divPoints.size() == divs) exit = true;
		}

		///////////////////////////////////
		currentIndex = _heLoop.size() - 1;
		divPoints.push_back(_heLoop[currentIndex].getVertex().getPosition()); // last

	}

	ZSPACE_INLINE void zFnMesh::computeEdgeLoop_SplitLength(vector<zItMeshHalfEdge>& _heLoop, float divLength, vector<zPoint>& divPoints)
	{
		divPoints.clear();

		float length = computeEdgeLoop_Length(_heLoop);
		int numInternalPts = floor(length / divLength);


		divPoints.push_back(_heLoop[0].getStartVertex().getPosition()); // first
		int currentindex = 0;

		//////////////////////// loop 
		int currentIndex = 0;
		zItMeshHalfEdge walkHe = _heLoop[currentIndex];
		zPoint pOnCurve = walkHe.getStartVertex().getPosition();;
		bool exit = false;

		zPoint start;

		float dStart = 0;
		float dIncrement = divLength;
		while (!exit)
		{
			zPoint eEndPoint = walkHe.getVertex().getPosition();
			dStart += dIncrement;
			float distance_increment = dIncrement;
			while (pOnCurve.distanceTo(eEndPoint) < distance_increment)
			{
				distance_increment = distance_increment - pOnCurve.distanceTo(eEndPoint);
				pOnCurve = eEndPoint;

				currentIndex++;
				walkHe = _heLoop[currentIndex];
				eEndPoint = walkHe.getVertex().getPosition();
			}

			zVector he_vec = walkHe.getVector();
			he_vec.normalize();

			start = pOnCurve + he_vec * distance_increment;
			pOnCurve = start;

			divPoints.push_back(pOnCurve);

			if (divPoints.size() == numInternalPts + 1) exit = true;
		}

		///////////////////////////////////
		currentIndex = _heLoop.size() - 1;
		divPoints.push_back(_heLoop[currentIndex].getVertex().getPosition()); // last

	}

	ZSPACE_INLINE float zFnMesh::computeEdgeLoop_Length(vector<zItMeshHalfEdge> &_heLoop)
	{
		float outLength = 0;

		for (auto& he : _heLoop) outLength += he.getLength();

		return outLength;
	}

	//--- SET METHODS 

	ZSPACE_INLINE void zFnMesh::setVertexPositions(zPointArray& pos)
	{
		if (pos.size() != zMeshObjectStorage::get(*meshObj).vertexPositions.size()) throw std::invalid_argument("size of position contatiner is not equal to number of graph vertices.");

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			zMeshObjectStorage::get(*meshObj).vertexPositions[i] = pos[i];
		}
	}

	ZSPACE_INLINE void zFnMesh::setVertexColor(zColor col, bool setFaceColor)
	{
		zMeshObjectStorage::get(*meshObj).vertexColors.clear();
		zMeshObjectStorage::get(*meshObj).vertexColors.assign(zMeshObjectStorage::get(*meshObj).n_v, col);

		if (setFaceColor) computeFaceColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnMesh::setVertexColorsfromScalars(zScalarArray& vertexScalars, bool setFaceColor)
	{
		if (vertexScalars.size() != numVertices())
		{
			throw std::invalid_argument(" error: scalars values to match number of vertices");
			return;
		}

		for (int i = 0; i < vertexScalars.size(); i++)
		{

			zColor col;

			if (vertexScalars[i] < 0) col = zColor(0.941, 0, 0.157, 1);
			if (vertexScalars[i] > 0.0) col = zColor(0, 0.941, 0.157, 1);

			zMeshObjectStorage::get(*meshObj).vertexColors[i] = col;

		}

		if (setFaceColor) computeFaceColorfromVertexColor();

	}

	ZSPACE_INLINE void zFnMesh::setVertexColors(zColorArray& col, bool setFaceColor)
	{
		if (zMeshObjectStorage::get(*meshObj).vertexColors.size() != zMeshObjectStorage::get(*meshObj).vertices.size())
		{
			zMeshObjectStorage::get(*meshObj).vertexColors.clear();
			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertices.size(); i++) zMeshObjectStorage::get(*meshObj).vertexColors.push_back(zColor(1, 0, 0, 1));
		}

		if (col.size() != zMeshObjectStorage::get(*meshObj).vertexColors.size()) throw std::invalid_argument("size of color contatiner is not equal to number of mesh vertices.");

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexColors.size(); i++)
		{
			zMeshObjectStorage::get(*meshObj).vertexColors[i] = col[i];
		}

		if (setFaceColor) computeFaceColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnMesh::setVertexWeight(double wt)
	{
		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexWeights.size(); i++) zMeshObjectStorage::get(*meshObj).vertexWeights[i] = wt;
	}

	ZSPACE_INLINE void zFnMesh::setVertexWeights(zDoubleArray& wt)
	{
		if (wt.size() != zMeshObjectStorage::get(*meshObj).vertexWeights.size()) throw std::invalid_argument("size of wt contatiner is not equal to number of mesh vertices.");

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexWeights.size(); i++)
		{
			zMeshObjectStorage::get(*meshObj).vertexWeights[i] = wt[i];
		}
	}

	ZSPACE_INLINE void zFnMesh::setFaceColor(zColor col, bool setVertexColor)
	{
		zMeshObjectStorage::get(*meshObj).faceColors.clear();
		zMeshObjectStorage::get(*meshObj).faceColors.assign(zMeshObjectStorage::get(*meshObj).n_f, col);

		if (setVertexColor) computeVertexColorfromFaceColor();
	}

	ZSPACE_INLINE void zFnMesh::setFaceColors(zColorArray& col, bool setVertexColor)
	{
		if (zMeshObjectStorage::get(*meshObj).faceColors.size() != zMeshObjectStorage::get(*meshObj).faces.size())
		{
			zMeshObjectStorage::get(*meshObj).faceColors.clear();
			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).faces.size(); i++) zMeshObjectStorage::get(*meshObj).faceColors.push_back(zColor(0.5, 0.5, 0.5, 1));
		}

		if (col.size() != zMeshObjectStorage::get(*meshObj).faceColors.size()) throw std::invalid_argument("size of color contatiner is not equal to number of mesh faces.");

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).faceColors.size(); i++)
		{
			zMeshObjectStorage::get(*meshObj).faceColors[i] = col[i];
		}

		if (setVertexColor) computeVertexColorfromFaceColor();
	}

	ZSPACE_INLINE void zFnMesh::setFaceColorOcclusion(zVector &lightVec, bool setVertexColor)
	{
		zVector* norm = getRawFaceNormals();
		zColor* col = getRawFaceColors();

		for (int i = 0; i < numPolygons(); i++)
		{
			double ang = norm[i].angle(lightVec);

			zDomainDouble in(90.0, 180.0);
			zDomainDouble out(0.4, 1.0);

			double val;

			if (ang <= 90) 	val = /*ofMap(ang, 0, 90, 1, 0.4)*/ 0.4;
			else if (ang > 90) 	val = coreUtils.ofMap<double>(ang, in, out);

			col[i] = zColor(val, val, val, 1);
		}

		if (setVertexColor) computeVertexColorfromFaceColor();
	}

	ZSPACE_INLINE void zFnMesh::setFaceNormals(zVector &fNormal)
	{
		zMeshObjectStorage::get(*meshObj).faceNormals.clear();
		zMeshObjectStorage::get(*meshObj).faceNormals.assign(zMeshObjectStorage::get(*meshObj).n_f, fNormal);

		// compute normals per face based on vertex normals and store it in faceNormals
		computeVertexNormalfromFaceNormal();
	}

	ZSPACE_INLINE void zFnMesh::setFaceNormals(zVectorArray &fNormals)
	{
		if (zMeshObjectStorage::get(*meshObj).faces.size() != fNormals.size()) throw std::invalid_argument("size of color contatiner is not equal to number of mesh faces.");

		zMeshObjectStorage::get(*meshObj).faceNormals.clear();

		zMeshObjectStorage::get(*meshObj).faceNormals = fNormals;

		// compute normals per face based on vertex normals and store it in faceNormals
		computeVertexNormalfromFaceNormal();
	}

	ZSPACE_INLINE void zFnMesh::setEdgeColor(zColor col, bool setVertexColor)
	{
		zMeshObjectStorage::get(*meshObj).edgeColors.clear();
		zMeshObjectStorage::get(*meshObj).edgeColors.assign(zMeshObjectStorage::get(*meshObj).n_e, col);

		if (setVertexColor) computeVertexColorfromEdgeColor();
	}

	ZSPACE_INLINE void zFnMesh::setEdgeColors(zColorArray& col, bool setVertexColor)
	{
		if (col.size() != zMeshObjectStorage::get(*meshObj).edgeColors.size()) throw std::invalid_argument("size of color container is not equal to number of mesh edges.");

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).edgeColors.size(); i++)
		{
			zMeshObjectStorage::get(*meshObj).edgeColors[i] = col[i];
		}

		if (setVertexColor) computeVertexColorfromEdgeColor();
	}

	ZSPACE_INLINE void zFnMesh::setEdgeWeight(double wt)
	{
		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).edges.size(); i++) zMeshObjectStorage::get(*meshObj).edgeWeights[i] = wt;
	}

	ZSPACE_INLINE void zFnMesh::setEdgeWeight(int index, double wt)
	{
		if (zMeshObjectStorage::get(*meshObj).edgeWeights.size() != zMeshObjectStorage::get(*meshObj).edges.size())
		{
			zMeshObjectStorage::get(*meshObj).edgeWeights.clear();
			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).edges.size(); i++) zMeshObjectStorage::get(*meshObj).edgeWeights.push_back(1);

		}

		zMeshObjectStorage::get(*meshObj).edgeWeights[index] = wt;

		int symEdge = (index % 2 == 0) ? index + 1 : index - 1;

		zMeshObjectStorage::get(*meshObj).edgeWeights[symEdge] = wt;
	}

	ZSPACE_INLINE void zFnMesh::setEdgeWeights(zDoubleArray& wt)
	{
		if (wt.size() != zMeshObjectStorage::get(*meshObj).edgeWeights.size()) throw std::invalid_argument("size of weight container is not equal to number of mesh edges.");

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).edgeWeights.size(); i++)
		{
			zMeshObjectStorage::get(*meshObj).edgeWeights[i] = wt[i];
		}
	}

	//--- GET METHODS 
	ZSPACE_INLINE void zFnMesh::getVertexPositions(zPointArray &pos, bool exludeCornerVertices)
	{
		pos = zMeshObjectStorage::get(*meshObj).vertexPositions;

		//// LB fix: this is crashing when excluding corner vertices 
		//pos.clear();

		//for (zItMeshVertex v(*meshObj); !v.end(); v++)
		//{
		//	if (exludeCornerVertices && v.checkValency(2)) continue;
		//	pos.push_back(v.getPosition());
		//}
	}

	ZSPACE_INLINE zPoint* zFnMesh::getRawVertexPositions()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zMeshObjectStorage::get(*meshObj).vertexPositions[0];
	}

	ZSPACE_INLINE void zFnMesh::getRawVertexPositions(float** points)
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");
			
		

		for (int i = 0; i < numVertices(); i++)
		{
			points[(i * 3) + 0] = &zMeshObjectStorage::get(*meshObj).vertexPositions[i].x;
			points[(i * 3) + 1] = &zMeshObjectStorage::get(*meshObj).vertexPositions[i].y;
			points[(i * 3) + 2] = &zMeshObjectStorage::get(*meshObj).vertexPositions[i].z;
		}

	}

	ZSPACE_INLINE void zFnMesh::getVertexNormals(zVectorArray& norm)
	{
		norm = zMeshObjectStorage::get(*meshObj).vertexNormals;
	}

	ZSPACE_INLINE zVector* zFnMesh::getRawVertexNormals()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zMeshObjectStorage::get(*meshObj).vertexNormals[0];
	}

	ZSPACE_INLINE void zFnMesh::getRawVertexNormals(float** normals)
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		for (int i = 0; i < numVertices(); i++)
		{
			normals[(i * 3) + 0] = &zMeshObjectStorage::get(*meshObj).vertexNormals[i].x;
			normals[(i * 3) + 1] = &zMeshObjectStorage::get(*meshObj).vertexNormals[i].y;
			normals[(i * 3) + 2] = &zMeshObjectStorage::get(*meshObj).vertexNormals[i].z;
		}

	}

	ZSPACE_INLINE void zFnMesh::getVertexColors(zColorArray& col)
	{
		col = zMeshObjectStorage::get(*meshObj).vertexColors;
	}

	ZSPACE_INLINE void zFnMesh::getVertexWeights(zDoubleArray& weights)
	{
		weights = zMeshObjectStorage::get(*meshObj).vertexWeights;
	}

	ZSPACE_INLINE zColor* zFnMesh::getRawVertexColors()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zMeshObjectStorage::get(*meshObj).vertexColors[0];
	}

	ZSPACE_INLINE void zFnMesh::getEdgeColors(zColorArray& col)
	{
		col = zMeshObjectStorage::get(*meshObj).edgeColors;
	}

	ZSPACE_INLINE void zFnMesh::getEdgeWeights(zDoubleArray& weights)
	{
		weights = zMeshObjectStorage::get(*meshObj).edgeWeights;
	}

	ZSPACE_INLINE zColor* zFnMesh::getRawEdgeColors()
	{
		if (numEdges() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zMeshObjectStorage::get(*meshObj).edgeColors[0];
	}

	ZSPACE_INLINE void zFnMesh::getFaceNormals(zVectorArray& norm)
	{
		norm = zMeshObjectStorage::get(*meshObj).faceNormals;
	}

	ZSPACE_INLINE zVector* zFnMesh::getRawFaceNormals()
	{
		if (numPolygons() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zMeshObjectStorage::get(*meshObj).faceNormals[0];
	}

	ZSPACE_INLINE void zFnMesh::getFaceColors(zColorArray& col)
	{
		col = zMeshObjectStorage::get(*meshObj).faceColors;
	}

	ZSPACE_INLINE zColor* zFnMesh::getRawFaceColors()
	{
		if (numEdges() == 0) throw std::invalid_argument(" error: null pointer.");

		return &zMeshObjectStorage::get(*meshObj).faceColors[0];
	}

	ZSPACE_INLINE zPoint zFnMesh::getCenter()
	{
		zPoint out;

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			out += zMeshObjectStorage::get(*meshObj).vertexPositions[i];
		}

		out /= zMeshObjectStorage::get(*meshObj).vertexPositions.size();

		return out;
	}

	ZSPACE_INLINE void zFnMesh::getCenters(zHEData type, zPointArray &centers)
	{
		// Mesh Edge 
		if (type == zHalfEdgeData)
		{
			centers.clear();

			for (zItMeshHalfEdge he(*meshObj); !he.end(); he++)
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

			for (zItMeshEdge e(*meshObj); !e.end(); e++)
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

		// Mesh Face 
		else if (type == zFaceData)
		{
			centers.clear();

			for (zItMeshFace f(*meshObj); !f.end(); f++)
			{
				if (f.isActive())
				{
					centers.push_back(f.getCenter());
				}
				else
				{
					centers.push_back(zVector());

				}
			}
		}
		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	ZSPACE_INLINE void zFnMesh::getDualMesh(zObjMesh &dualMeshObj, zIntArray &inEdge_dualEdge, zIntArray &dualEdge_inEdge, bool excludeBoundary, bool keepExistingBoundary, bool rotate90)
	{
		vector<zVector> positions;
		vector<int> polyConnects;
		vector<int> polyCounts;

		vector<zVector> fCenters;
		getCenters(zFaceData, fCenters);

		vector<zVector> eCenters;
		getCenters(zHalfEdgeData, eCenters);

		positions = fCenters;

		// store map for input mesh edge to new vertex Id
		vector<int> inEdge_dualVertex;

		//for (int i = 0; i < zMeshObjectStorage::get(*meshObj).edgeActive.size(); i++)
		for (zItMeshHalfEdge he(*meshObj); !he.end(); he++)
		{
			inEdge_dualVertex.push_back(-1);

			if (!he.isActive()) continue;

			if (he.onBoundary())
			{
				if (!excludeBoundary)
				{
					inEdge_dualVertex[he.getId()] = positions.size();
					positions.push_back(eCenters[he.getId()]);
				}
			}
			else
			{
				inEdge_dualVertex[he.getId()] = he.getFace().getId(); ;
			}
		}

		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			if (!v.isActive()) continue;

			if (v.onBoundary())
			{
				if (excludeBoundary) continue;

				zItMeshHalfEdge e = v.getHalfEdge(); ;

				zItMeshHalfEdge start = v.getHalfEdge();

				vector<int> tempPolyConnects;
				do
				{
					int eId = e.getId();
					int index = -1;
					bool checkRepeat = zMeshObjectStorage::get(*meshObj).coreUtils.checkRepeatElement(inEdge_dualVertex[eId], tempPolyConnects, index);
					if (!checkRepeat) tempPolyConnects.push_back(inEdge_dualVertex[eId]);

					if (keepExistingBoundary)
					{
						int vId = e.getVertex().getId();
						if (e.onBoundary() && vId == v.getId())
						{
							//printf("\n working!");
							//tempPolyConnects.push_back(vId);
						}
					}

					e = e.getPrev();
					eId = e.getId();

					if (keepExistingBoundary)
					{
						int vId = e.getVertex().getId();
						if (e.onBoundary() && vId == v.getId())
						{
							//printf("\n working2!");
							//tempPolyConnects.push_back(positions.size());
							//positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[vId]);

						}
					}

					index = -1;
					checkRepeat = zMeshObjectStorage::get(*meshObj).coreUtils.checkRepeatElement(inEdge_dualVertex[eId], tempPolyConnects, index);
					if (!checkRepeat) tempPolyConnects.push_back(inEdge_dualVertex[eId]);

					e = e.getSym();

				} while (e != start);


				for (int j = 0; j < tempPolyConnects.size(); j++)
				{
					polyConnects.push_back(tempPolyConnects[j]);
				}

				polyCounts.push_back(tempPolyConnects.size());
			}

			else
			{
				vector<int> cEdges;
				v.getConnectedHalfEdges(cEdges);

				for (int j = 0; j < cEdges.size(); j++)
				{
					polyConnects.push_back(inEdge_dualVertex[cEdges[j]]);
				}

				polyCounts.push_back(cEdges.size());
			}
		}

		zMeshObjectStorage::get(dualMeshObj).create(positions, polyCounts, polyConnects);

		// rotate by 90

		if (rotate90)
		{
			zVector meshNorm = zMeshObjectStorage::get(*meshObj).vertexNormals[0];
			meshNorm.normalize();

			// bounding box
			zVector minBB, maxBB;
			zMeshObjectStorage::get(*meshObj).coreUtils.getBounds(zMeshObjectStorage::get(dualMeshObj).vertexPositions, minBB, maxBB);

			zVector cen = (maxBB + minBB) * 0.5;

			for (int i = 0; i < zMeshObjectStorage::get(dualMeshObj).vertexPositions.size(); i++)
			{
				zMeshObjectStorage::get(dualMeshObj).vertexPositions[i] -= cen;
				zMeshObjectStorage::get(dualMeshObj).vertexPositions[i] = zMeshObjectStorage::get(dualMeshObj).vertexPositions[i].rotateAboutAxis(meshNorm, -90);
			}
		}

		// compute dualEdge_inEdge
		dualEdge_inEdge.clear();

		for (auto &he : zMeshObjectStorage::get(dualMeshObj).halfEdges)
		{
			dualEdge_inEdge.push_back(-1);
		}

		// compute inEdge to dualEdge	
		inEdge_dualEdge.clear();

		for (int i = 0; i < numHalfEdges(); i++)
		{
			int v1 = inEdge_dualVertex[i];
			int v2 = (i % 2 == 0) ? inEdge_dualVertex[i + 1] : inEdge_dualVertex[i - 1];

			int eId;
			bool chk = zMeshObjectStorage::get(dualMeshObj).halfEdgeExists(v1, v2, eId);
			if (chk) inEdge_dualEdge.push_back(eId);
			else inEdge_dualEdge.push_back(-1);

			if (inEdge_dualEdge[i] != -1)
			{
				dualEdge_inEdge[inEdge_dualEdge[i]] = i;
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::getDualGraph(zObjGraph &dualGraphObj, zIntArray &inEdge_dualEdge, zIntArray &dualEdge_inEdge, bool excludeBoundary , bool PlanarMesh , bool rotate90)
	{
		vector<zVector> positions;
		vector<int> edgeConnects;

		vector<zVector> fCenters;
		getCenters(zFaceData, fCenters);

		vector<zVector> eCenters;
		getCenters(zHalfEdgeData, eCenters);

		positions = fCenters;

		// store map for input mesh edge to new vertex Id
		vector<int> inEdge_dualVertex;

		for (zItMeshHalfEdge he(*meshObj); !he.end(); he++)
		{
			inEdge_dualVertex.push_back(-1);

			int i = he.getId();

			if (!he.isActive()) continue;

			if (he.onBoundary())
			{
				if (!excludeBoundary)
				{
					inEdge_dualVertex[i] = positions.size();
					positions.push_back(eCenters[i]);
				}
			}
			else
			{
				inEdge_dualVertex[i] = he.getFace().getId();
			}
		}

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).halfEdges.size(); i += 2)
		{
			int v_0 = inEdge_dualVertex[i];
			int v_1 = inEdge_dualVertex[i + 1];

			if (v_0 != -1 && v_1 != -1)
			{
				edgeConnects.push_back(v_0);
				edgeConnects.push_back(v_1);
			}

		}

		zFnGraph fnDualGraph(dualGraphObj);

		if (PlanarMesh)
		{
			zVector graphNorm = zMeshObjectStorage::get(*meshObj).vertexNormals[0];
			graphNorm.normalize();

			fnDualGraph.create(positions, edgeConnects, graphNorm);
		}

		else fnDualGraph.create(positions, edgeConnects);

		// rotate by 90

		if (rotate90 && PlanarMesh)
		{
			zVector graphNorm = zMeshObjectStorage::get(*meshObj).vertexNormals[0];
			graphNorm.normalize();

			// bounding box
			zVector minBB, maxBB;
			fnDualGraph.getBounds(minBB, maxBB);

			zVector cen = (maxBB + minBB) * 0.5;

			zPointArray dualPositions;
			fnDualGraph.getVertexPositions(dualPositions);
			for (auto& position : dualPositions)
			{
				position -= cen;
				position = position.rotateAboutAxis(graphNorm, -90);
			}
			fnDualGraph.setVertexPositions(dualPositions);
		}

		// compute dualEdge_inEdge
		dualEdge_inEdge.clear();
		dualEdge_inEdge.assign(fnDualGraph.numHalfEdges(), -1);

		// compute inEdge to dualEdge	
		inEdge_dualEdge.clear();

		for (int i = 0; i < numHalfEdges(); i++)
		{
			int v1 = inEdge_dualVertex[i];
			int v2 = (i % 2 == 0) ? inEdge_dualVertex[i + 1] : inEdge_dualVertex[i - 1];

			int eId;
			bool chk = fnDualGraph.halfEdgeExists(v1, v2, eId);

			if (chk) inEdge_dualEdge.push_back(eId);
			else inEdge_dualEdge.push_back(-1);

			if (inEdge_dualEdge[i] != -1)
			{
				dualEdge_inEdge[inEdge_dualEdge[i]] = i;
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::getRainflowGraph(zObjGraph &rainflowGraphObj, bool excludeBoundary)
	{
		vector<zVector> positions;
		vector<int> edgeConnects;

		zVector* pos = getRawVertexPositions();

		unordered_map <string, int> positionVertex;

		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			if (excludeBoundary && v.onBoundary()) continue;

			vector<zItMeshFace> cFaces;
			v.getConnectedFaces(cFaces);

			vector<int> positionIndicies;
			for (auto &f : cFaces)
			{
				vector<int> fVerts;
				f.getVertices(fVerts);

				for (int k = 0; k < fVerts.size(); k++) positionIndicies.push_back(fVerts[k]);
			}

			// get lowest positions

			zVector lowPosition = pos[v.getId()];

			for (int j = 0; j < positionIndicies.size(); j++)
			{
				if (pos[positionIndicies[j]].z < lowPosition.z)
				{
					lowPosition = pos[positionIndicies[j]];

				}
			}

			vector<int> lowId;
			if (lowPosition.z != pos[v.getId()].z)
			{
				for (int j = 0; j < positionIndicies.size(); j++)
				{
					if (pos[positionIndicies[j]].z == lowPosition.z)
					{
						lowId.push_back(positionIndicies[j]);

					}
				}
			}

			if (lowId.size() > 0)
			{
				for (int j = 0; j < lowId.size(); j++)
				{
					zVector pos1 = pos[v.getId()];
					int v1;
					bool check1 = coreUtils.vertexExists(positionVertex, pos1, 3, v1);
					if (!check1)
					{
						v1 = positions.size();
						positions.push_back(pos1);
						coreUtils.addToPositionMap(positionVertex, pos1, v1, 3);
					}


					zVector pos2 = pos[lowId[j]];
					int v2;
					bool check2 = coreUtils.vertexExists(positionVertex, pos2, 3, v2);
					if (!check2)
					{
						v2 = positions.size();
						positions.push_back(pos2);
						coreUtils.addToPositionMap(positionVertex, pos2, v2, 3);
					}


					edgeConnects.push_back(v1);
					edgeConnects.push_back(v2);
				}
			}
		}
		zFnGraph fnRainflowGraph(rainflowGraphObj);
		fnRainflowGraph.create(positions, edgeConnects);
	}

	ZSPACE_INLINE void zFnMesh::getMeshTriangles(vector<zIntArray> &faceTris)
	{
		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() == 0 || zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();

		faceTris.clear();

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			vector<int> Tri_connects;
			int i = f.getId();

			if (f.isActive())
			{

				vector<int> fVerts;
				f.getVertices(fVerts);

				// compute polygon Triangles


				int n_Tris = 0;
				if (fVerts.size() > 0) f.getTriangles(n_Tris, Tri_connects);
				else Tri_connects = fVerts;
			}
			faceTris.push_back(Tri_connects);
		}
	}

	ZSPACE_INLINE double zFnMesh::getMeshVolume()
	{
		double out = 0;

		vector<vector<int>> faceTris;
		getMeshTriangles(faceTris);

		for (int i = 0; i < faceTris.size(); i++)
		{
			for (int j = 0; j < faceTris[i].size(); j += 3)
			{
				double vol = zMeshObjectStorage::get(*meshObj).coreUtils.getSignedTriangleVolume(zMeshObjectStorage::get(*meshObj).vertexPositions[faceTris[i][j + 0]], zMeshObjectStorage::get(*meshObj).vertexPositions[faceTris[i][j + 1]], zMeshObjectStorage::get(*meshObj).vertexPositions[faceTris[i][j + 2]]);

				out += vol;
			}
		}

		return out;
	}

	ZSPACE_INLINE void zFnMesh::getMeshFaceVolumes(vector<zIntArray> &faceTris, zPointArray &fCenters, zDoubleArray &faceVolumes, bool absoluteVolumes)
	{
		if (faceTris.size() == 0) getMeshTriangles(faceTris);
		if (fCenters.size() == 0 || fCenters.size() != numPolygons()) getCenters(zFaceData, fCenters);

		faceVolumes.clear();

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			int i = f.getId();
			double vol = f.getVolume(faceTris[i], fCenters[i], absoluteVolumes);

			faceVolumes.push_back(vol);
		}
	}

	ZSPACE_INLINE void zFnMesh::getPrincipalCurvatures(zCurvatureArray &vertexCurvatures, zVectorArray& pVector1, zVectorArray& pVector2)
	{
		vertexCurvatures.clear();
		pVector1.clear();
		pVector2.clear();

		vertexCurvatures.assign(numVertices(), zCurvature());
		pVector1.assign(numVertices(), zVector());
		pVector2.assign(numVertices(), zVector());

		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			int id = v.getId();

			if (!v.isActive())
			{
				vertexCurvatures[id].k1 = -1;
				vertexCurvatures[id].k2 = -1;
				continue;
			}

			double gaussianCurvature = 0.0;
			double meanCurvature = 0.0;
			zVector tangentDirection;
			computeVertexCurvatureData(*this, v, gaussianCurvature, meanCurvature, tangentDirection);

			double discriminant = (meanCurvature * meanCurvature) - gaussianCurvature;
			if (discriminant < 0.0) discriminant = 0.0;

			double root = std::sqrt(discriminant);
			vertexCurvatures[id].k1 = meanCurvature + root;
			vertexCurvatures[id].k2 = meanCurvature - root;

			zVector normal = normalized(v.getNormal());
			zVector tangent2 = normalized(normal ^ tangentDirection);
			if (vectorLength(tangent2) <= ZSPACE_CURVATURE_EPS)
			{
				tangent2 = zVector();
			}

			pVector1[id] = tangentDirection;
			pVector2[id] = tangent2;
		}
		
	}


	ZSPACE_INLINE void zFnMesh::getGaussianCurvature(zDoubleArray &vertexCurvatures)
	{	
		vertexCurvatures.clear();
		vertexCurvatures.assign(numVertices(), -1);

		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			int id = v.getId();
			if (!v.isActive()) continue;

			double gaussianCurvature = 0.0;
			double meanCurvature = 0.0;
			zVector tangentDirection;
			computeVertexCurvatureData(*this, v, gaussianCurvature, meanCurvature, tangentDirection);
			vertexCurvatures[id] = gaussianCurvature;
		}
	}

	ZSPACE_INLINE void zFnMesh::getPlanarityDeviationPerFace(zDoubleArray& planarityDevs, zPlanarSolverType type, bool colorFaces, double tolerance)
	{

		if (planarityDevs.size() != numPolygons())
		{
			planarityDevs.clear();
			planarityDevs.assign(numPolygons(), -1);
		}

		
		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			int i = f.getId();

			zPointArray fVerts;
			f.getVertexPositions(fVerts);

			if (type == zQuadPlanar)
			{
				if (fVerts.size() == 3)planarityDevs[i] = 0.0;

				if (fVerts.size() == 4)
				{
					double uA, uB;
					zPoint pA, pB;

					coreUtils.line_lineClosestPoints(fVerts[0], fVerts[2], fVerts[1], fVerts[3], uA, uB, pA, pB);
					planarityDevs[i] = pA.distanceTo(pB);
				}
				
			}

			if (type == zVolumePlanar)
			{
				zIntArray fTris;
				zPoint fCenter = f.getCenter();
				zVector fNorm = f.getNormal();
				float dev = f.getVolume(fTris, fCenter, false);
				planarityDevs[i] = abs(dev);
			}

			if (planarityDevs[i] == -1) continue;
			if (planarityDevs[i] < tolerance) f.setColor(zGREEN);
			else f.setColor(zMAGENTA);
		}
			
		

		
	}

	ZSPACE_INLINE void zFnMesh::getEdgeDihedralAngles(zDoubleArray &dihedralAngles)
	{
		vector<double> out;

		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();

		for (zItMeshEdge e(*meshObj); !e.end(); e++)
		{
			out.push_back(e.getDihedralAngle());
		}

		dihedralAngles = out;
	}

	ZSPACE_INLINE double zFnMesh::getHalfEdgeLengths(zDoubleArray &halfEdgeLengths)
	{
		double total = 0.0;

		halfEdgeLengths.clear();

		for (zItMeshEdge e(*meshObj); !e.end(); e++)
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

	ZSPACE_INLINE double zFnMesh::getEdgeLengths(zDoubleArray &edgeLengths)
	{
		double total = 0.0;


		edgeLengths.clear();

		for (zItMeshEdge e(*meshObj); !e.end(); e++)
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

	ZSPACE_INLINE double zFnMesh::getVertexAreas(zPointArray &faceCenters, zPointArray &edgeCenters, zFloatArray &vertexAreas)
	{
		vector<float> out;

		double totalArea = 0;

		for (zItMeshVertex v(*meshObj); !v.end(); v++)
		{
			double vArea = 0;

			int i = v.getId();

			if (v.isActive())
			{
				vector<zItMeshHalfEdge> cEdges;
				v.getConnectedHalfEdges(cEdges);

				for (int j = 0; j < cEdges.size(); j++)
				{


					zItMeshHalfEdge cE = cEdges[j];
					zItMeshHalfEdge nE = cEdges[(j + 1) % cEdges.size()];

					if (cE.onBoundary() || nE.getSym().onBoundary()) continue;

					if (cE.getFace().getId() != nE.getSym().getFace().getId()) continue;

					zVector vPos = zMeshObjectStorage::get(*meshObj).vertexPositions[i];
					zVector fCen = faceCenters[cE.getFace().getId()];
					zVector currentEdge_cen = edgeCenters[cE.getId()];
					zVector nextEdge_cen = edgeCenters[nE.getId()];

					double Area1 = zMeshObjectStorage::get(*meshObj).coreUtils.getTriangleArea(vPos, currentEdge_cen, fCen);
					vArea += (Area1);

					double Area2 = zMeshObjectStorage::get(*meshObj).coreUtils.getTriangleArea(vPos, nextEdge_cen, fCen);
					vArea += (Area2);

				}

			}
			out.push_back(vArea);

			totalArea += vArea;
		}

		//printf("\n totalArea : %1.4f ",  totalArea);

		vertexAreas = out;

		return totalArea;
	}

	ZSPACE_INLINE double zFnMesh::getPlanarFaceAreas(zDoubleArray &faceAreas)
	{
		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();

		faceAreas.clear();

		double totalArea = 0;

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			double fArea = f.getPlanarFaceArea();
			faceAreas.push_back(fArea);

			totalArea += fArea;
		}

		return totalArea;
	}

	ZSPACE_INLINE void zFnMesh::getPolygonData(zIntArray(&polyConnects), zIntArray(&polyCounts))
	{
		polyConnects.clear();
		polyCounts.clear();

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			if (!f.isActive()) continue;

			vector<int> facevertices;
			f.getVertices(facevertices);

			polyCounts.push_back(facevertices.size());

			for (int j = 0; j < facevertices.size(); j++)
			{
				polyConnects.push_back(facevertices[j]);
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::getMatrices_trimesh(MatrixXd& V, MatrixXi& F)
	{
		zPoint* vPositions = getRawVertexPositions();
		MatrixXd triMesh_V(numVertices(), 3);

		// fill vertex matrix
		for (int i = 0; i < numVertices(); i++)
		{
			triMesh_V(i, 0) = vPositions[i].x;
			triMesh_V(i, 1) = vPositions[i].y;
			triMesh_V(i, 2) = vPositions[i].z;
		}

		V = triMesh_V;

		// fill triangle matrix
		MatrixXi FTris(numPolygons(), 3);

		int nTris = 0;
		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			int i = f.getId();

			zIntArray fVerts;
			f.getVertices(fVerts);

			FTris(i, 0) = fVerts[0] ;
			FTris(i, 1) = fVerts[1] ;
			FTris(i, 2) = fVerts[2] ;
		}

		F = FTris;
	}

	ZSPACE_INLINE void zFnMesh::getMatrices_quadmesh(MatrixXd& V, MatrixXi& F)
	{
		zPoint* vPositions = getRawVertexPositions();
		MatrixXd quadMesh_V(numVertices(), 3);

		// fill vertex matrix
		for (int i = 0; i < numVertices(); i++)
		{
			quadMesh_V(i, 0) = vPositions[i].x;
			quadMesh_V(i, 1) = vPositions[i].y;
			quadMesh_V(i, 2) = vPositions[i].z;
		}

		V = quadMesh_V;

		// fill triangle matrix
		MatrixXi FQuads(numPolygons(), 4);

		int nTris = 0;
		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			int i = f.getId();

			zIntArray fVerts;
			f.getVertices(fVerts);

			FQuads(i, 0) = fVerts[0];
			FQuads(i, 1) = fVerts[1];
			FQuads(i, 2) = fVerts[2];
			FQuads(i, 3) = fVerts[3];
		}

		F = FQuads;
	}

	ZSPACE_INLINE void zFnMesh::getEdgeData(zIntArray &edgeConnects, bool excludeBoundary)
	{
		edgeConnects.clear();
		
		for (zItMeshEdge e(*meshObj); !e.end(); e++)
		{
			if (excludeBoundary && e.onBoundary()) continue;

			edgeConnects.push_back(e.getHalfEdge(0).getVertex().getId());
			edgeConnects.push_back(e.getHalfEdge(1).getVertex().getId());
		}
	}

	ZSPACE_INLINE void zFnMesh::getDuplicate(zObjMesh &out)
	{
		/*vector<zVector> positions;
		vector<int> polyConnects;
		vector<int> polyCounts;

		positions = zMeshObjectStorage::get(*meshObj).vertexPositions;
		getPolygonData(polyConnects, polyCounts);

		zMeshObjectStorage::get(out).create(positions, polyCounts, polyConnects);

		zFnMesh tempFn(out);

		tempFn.computeMeshNormals();
		tempFn.setVertexColors(zMeshObjectStorage::get(*meshObj).vertexColors, false);
		tempFn.setEdgeColors(zMeshObjectStorage::get(*meshObj).edgeColors, false);
		tempFn.setFaceColors(zMeshObjectStorage::get(*meshObj).faceColors, false);*/

		out = zObjMesh(*meshObj);
	}

	

	//---- CONTOUR METHODS

	ZSPACE_INLINE void zFnMesh::splitMesh_Mixed(zPointArray& splitPlanes_origins, zVectorArray& splitPlanes_normals, zObjMesh& resultMeshObj)
	{
		zObjMesh tempObj;
		getDuplicate(tempObj);


		for (int j = 0; j < splitPlanes_origins.size() ; j++)
		{
			zFnMesh tempFn(tempObj);			
			zScalarArray scalars;

			int i = 0;
			for (zItMeshVertex v(tempObj); !v.end(); v++, i++)
			{

				zPoint O = splitPlanes_origins[j];
				zVector N = splitPlanes_normals[j];
								 
				zPoint P = v.getPosition();
				float minDist_Plane = coreUtils.minDist_Point_Plane(P, O, N);
				scalars.push_back(minDist_Plane);

			}

			tempFn.getIsoMesh_mixed(scalars, 0.0, false, resultMeshObj);

			if (j < splitPlanes_origins.size() - 1)
			{
				tempFn.clear();

				zFnMesh resultFn(resultMeshObj);
				resultFn.getDuplicate(tempObj);			

				resultFn.clear();
			}

		}


	}

	ZSPACE_INLINE void zFnMesh::splitMesh_Quad(vector<zPlane>& splitPlanes, bool invertMesh, zObjMesh& resultMeshObj)
	{
		
		zScalarArray scalars;
		scalars.assign(numVertices(), 100000);


		for (int j = 0; j < splitPlanes.size(); j++)
		{
			int i = 0;
			for (zItMeshVertex v(*meshObj); !v.end(); v++, i++)
			{
				zPoint O(splitPlanes[j](3, 0), splitPlanes[j](3, 1), splitPlanes[j](3, 2));
				zVector N(splitPlanes[j](2, 0), splitPlanes[j](2, 1), splitPlanes[j](2, 2));

				zPoint P = v.getPosition();
				float minDist_Plane = coreUtils.minDist_Point_Plane(P, O, N);

				/*if (j == 0)
				{
					scalars[i] = minDist_Plane;
				}
				else
				{
					float bool_union = coreUtils.zMin(minDist_Plane, scalars[i]);
					float bool_intersect = coreUtils.zMax(minDist_Plane, scalars[i]);

					float bool_difference = coreUtils.zMax(bool_union, bool_intersect);

					scalars[i] = bool_difference;
				}*/


				scalars[i] = (j == 0) ? minDist_Plane : coreUtils.zMax(scalars[i], minDist_Plane);
				//scalars[i] = coreUtils.zMin(minDist_Plane, scalars[i]);
			}
		}


		setVertexColorsfromScalars(scalars, false);
		getIsoMesh(scalars, 0.0, invertMesh, resultMeshObj);

				
	}

	ZSPACE_INLINE void zFnMesh::getIsoMesh_mixed(zScalarArray& vertexScalars, float threshold, bool invertMesh, zObjMesh& coutourMeshObj)
	{

		if (vertexScalars.size() != numVertices())
		{
			throw std::invalid_argument(" error: scalars values to match number of vertices");
			return;
		}

		zFnMesh tempFn(coutourMeshObj);
		tempFn.clear(); // clear memory if the mobject exists.

		vector<zVector>positions;
		vector<int>polyConnects;
		vector<int>polyCounts;

		unordered_map <string, int> positionVertex;


		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			getIsolinePoly_mixed(vertexScalars, f, positions, polyConnects, polyCounts, positionVertex, threshold, invertMesh);
		}

		tempFn.create(positions, polyCounts, polyConnects);;

	}

	ZSPACE_INLINE void zFnMesh::getIsoContour(zScalarArray& vertexScalars, float threshold, zPointArray& positions, zIntArray& edgeConnects,  zColorArray& cVertexColor, int precision, float distTolerance, bool selectedFaces, zColor selectedFaceColor)
	{

		if (vertexScalars.size() != numVertices())
		{
			throw std::invalid_argument(" error: scalars values to match number of vertices");
			return;
		}

		positions.clear();
		edgeConnects.clear();
		cVertexColor.clear();

		unordered_map <string, int> positionVertex;
				

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			if (selectedFaces)
			{
				if(f.getColor() == selectedFaceColor)
					getIsoline(vertexScalars, f, positions, edgeConnects, cVertexColor, positionVertex, threshold, precision, distTolerance);
			}
			else getIsoline(vertexScalars, f, positions, edgeConnects, cVertexColor, positionVertex, threshold, precision, distTolerance);
				
		}
		
	

	}


	ZSPACE_INLINE void zFnMesh::getIsoMesh(zScalarArray& vertexScalars, float threshold, bool invertMesh, zObjMesh& coutourMeshObj)
	{

		if (vertexScalars.size() != numVertices())
		{
			throw std::invalid_argument(" error: scalars values to match number of vertices");
			return;
		}

		zFnMesh tempFn(coutourMeshObj);
		tempFn.clear(); // clear memory if the mobject exists.

		vector<zVector>positions;
		vector<int>polyConnects;
		vector<int>polyCounts;

		unordered_map <string, int> positionVertex;


		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			getIsolinePoly(vertexScalars, f, positions, polyConnects, polyCounts, positionVertex, threshold, invertMesh);
		}

		tempFn.create(positions, polyCounts, polyConnects);;

	}
	
	ZSPACE_INLINE void zFnMesh::getIsobandMesh(zScalarArray& vertexScalars, float inThresholdLow, float inThresholdHigh, zObjMesh& coutourMeshObj)
	{
		zFnMesh tempFn(coutourMeshObj);
		tempFn.clear(); // clear memory if the mobject exists.

		vector<zVector>positions;
		vector<int>polyConnects;
		vector<int>polyCounts;

		unordered_map <string, int> positionVertex;

		
		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			getIsobandPoly(vertexScalars, f, positions, polyConnects, polyCounts, positionVertex, (inThresholdLow < inThresholdHigh) ? inThresholdLow : inThresholdHigh, (inThresholdLow < inThresholdHigh) ? inThresholdHigh : inThresholdLow);
		}

		//printf("\n %i %i ", positions.size(), polyCounts.size());
		tempFn.create(positions, polyCounts, polyConnects);;
		
	}

	//---- TRI-MESH MODIFIER METHODS

	ZSPACE_INLINE void zFnMesh::faceTriangulate(zItMeshFace &face)
	{
		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() == 0 || zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();

		vector<int> fVerts;
		face.getVertices(fVerts);

		int numfaces_original = zMeshObjectStorage::get(*meshObj).faces.size();
		int numHalfEdges_original = zMeshObjectStorage::get(*meshObj).halfEdges.size();

		if (fVerts.size() != 3)
		{
			// compute polygon Triangles
			int n_Tris = 0;
			vector<int> Tri_connects;
			face.getTriangles(n_Tris, Tri_connects);

			//printf("\n %i numtris: %i %i ", faceIndex, n_Tris, Tri_connects.size());

			for (int j = 0; j < n_Tris; j++)
			{
				vector<int> triVerts;
				triVerts.push_back(Tri_connects[j * 3]);
				triVerts.push_back(Tri_connects[j * 3 + 1]);
				triVerts.push_back(Tri_connects[j * 3 + 2]);

				//printf("\n %i %i %i ", Tri_connects[j * 3], Tri_connects[j * 3 + 1], Tri_connects[j * 3 + 2]);

				// check if edges e01, e12 or e20					
				zItMeshHalfEdge e01, e12, e20;

				bool e01_Boundary = false;
				bool e12_Boundary = false;
				bool e20_Boundary = false;

				for (int k = 0; k < triVerts.size(); k++)
				{

					if (k == 0)
					{
						addEdges(triVerts[k], triVerts[(k + 1) % triVerts.size()], true, e01);

						if (e01.getId() < numHalfEdges_original)
						{
							if (e01.onBoundary())  e01_Boundary = true;
						}
					}

					if (k == 1)
					{
						addEdges(triVerts[k], triVerts[(k + 1) % triVerts.size()], true, e12);

						if (e12.getId() < numHalfEdges_original)
						{
							if (e12.onBoundary())  e12_Boundary = true;
						}
					}

					if (k == 2)
					{
						addEdges(triVerts[k], triVerts[(k + 1) % triVerts.size()], true, e20);

						if (e20.getId() < numHalfEdges_original)
						{
							if (e20.onBoundary())  e20_Boundary = true;
						}
					}
				}

				//printf("\n %i %i %i ", e01.getId(), e12.getId(), e20.getId());

				if (j > 0)
				{
					zItMeshFace newFace;
					bool check = addPolygon(newFace);

					newFace.setHalfEdge(e01);

					if (!e01_Boundary) e01.setFace(newFace);
					if (!e12_Boundary) e12.setFace(newFace);
					if (!e20_Boundary) e20.setFace(newFace);
				}
				else
				{
					if (!e01_Boundary) face.setHalfEdge(e01);
					else if (!e12_Boundary) face.setHalfEdge(e12);
					else if (!e20_Boundary) face.setHalfEdge(e20);


					if (!e01_Boundary) e01.setFace(face);
					if (!e12_Boundary) e12.setFace(face);
					if (!e20_Boundary) e20.setFace(face);
				}

				// update edge pointers
				e01.setNext(e12);
				e12.setPrev(e01);

				e01.setPrev(e20);
				e20.setNext(e01);

				e12.setNext(e20);
				e20.setPrev(e12);
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::triangulate()
	{
		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() == 0 || zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();

		// iterate through faces and triangulate faces with more than 3 vetices
		int numfaces_original = zMeshObjectStorage::get(*meshObj).faces.size();

		for (int i = 0; i < numfaces_original; i++)
		{
			zItMeshFace f(*meshObj, i);
			if (!f.isActive()) continue;

			faceTriangulate(f);
		}

		computeMeshNormals();
	}

	//---- DELETE MODIFIER METHODS

	ZSPACE_INLINE void zFnMesh::deleteVertex(int index, bool removeInactiveElems)
	{
		//if (index >= zMeshObjectStorage::get(*meshObj).vertices.size()) throw std::invalid_argument(" error: index out of bounds.");
		//if (!zMeshObjectStorage::get(*meshObj).indexToVertex[index]->isActive()) throw std::invalid_argument(" error: index out of bounds.");

		//// check if boundary vertex
		//bool boundaryVertex = (onBoundary(index, zVertexData));

		//// get connected faces
		//vector<int> cFaces;
		//getConnectedFaces(index, zVertexData, cFaces);

		//// get connected edges
		//vector<int> cEdges;
		//getConnectedEdges(index, zVertexData, cEdges);



		//// get vertices in cyclical orders without the  the vertex to be removed. remove duplicates if any
		//vector<int> outerVertices;

		//vector<int> deactivateVertices;
		//vector<int> deactivateEdges;

		//deactivateVertices.push_back(index);

		//// add to deactivate edges connected edges
		//for (int i = 0; i < cEdges.size(); i++) deactivateEdges.push_back(cEdges[i]);

		//// add to deactivate vertices with valence 2 and  connected edges of that vertex
		//for (int i = 0; i < cEdges.size(); i++)
		//{
		//	int v0 = zMeshObjectStorage::get(*meshObj).edges[cEdges[i]].getVertex()->getVertexId();

		//	if (!onBoundary(v0, zVertexData) && checkVertexValency(v0, 2))
		//	{
		//		deactivateVertices.push_back(v0);
		//		deactivateEdges.push_back(zMeshObjectStorage::get(*meshObj).edges[cEdges[i]].getNext()->getEdgeId());
		//	}
		//}

		//// compute new face vertices
		//for (int i = 0; i < cEdges.size(); i++)
		//{
		//	if (!zMeshObjectStorage::get(*meshObj).edges[cEdges[i]].getFace()) continue;

		//	zEdge *curEdge = &zMeshObjectStorage::get(*meshObj).edges[cEdges[i]];
		//	int v0 = curEdge->getVertex()->getVertexId();

		//	do
		//	{
		//		bool vertExists = false;

		//		for (int k = 0; k < outerVertices.size(); k++)
		//		{
		//			if (v0 == outerVertices[k])
		//			{
		//				vertExists = true;
		//				break;
		//			}
		//		}

		//		if (!vertExists)
		//		{
		//			for (int k = 0; k < deactivateVertices.size(); k++)
		//			{
		//				if (v0 == deactivateVertices[k])
		//				{
		//					vertExists = true;
		//					break;
		//				}
		//			}
		//		}

		//		if (!vertExists) outerVertices.push_back(v0);



		//		curEdge = curEdge->getNext();
		//		v0 = curEdge->getVertex()->getVertexId();


		//	} while (v0 != index);

		//}


		//// deactivate connected edges 
		//for (int i = 0; i < deactivateEdges.size(); i++)
		//{
		//	if (zMeshObjectStorage::get(*meshObj).edgeActive[deactivateEdges[i]])deactivateElement(deactivateEdges[i], zEdgeData);
		//}

		//// disable connected faces
		//for (int i = 0; i < cFaces.size(); i++)
		//{
		//	if (zMeshObjectStorage::get(*meshObj).faceActive[cFaces[i]]) deactivateElement(cFaces[i], zFaceData);
		//}

		//// deactivate vertex
		//for (int i = 0; i < deactivateVertices.size(); i++)
		//{
		//	if (zMeshObjectStorage::get(*meshObj).vertexActive[deactivateVertices[i]]) deactivateElement(deactivateVertices[i], zVertexData);
		//}



		//// add new face if outerVertices has more than 2 vertices

		//if (outerVertices.size() > 2)
		//{
		//	zMeshObjectStorage::get(*meshObj).addPolygon(outerVertices);

		//	if (boundaryVertex)  zMeshObjectStorage::get(*meshObj).update_BoundaryEdgePointers();
		//}

		//computeMeshNormals();

		//if (removeInactiveElems)
		//{
		//	removeInactiveElements(zVertexData);
		//	removeInactiveElements(zEdgeData);
		//	removeInactiveElements(zFaceData);
		//}
	}

	ZSPACE_INLINE void zFnMesh::deleteFace(int index, bool removeInactiveElems)
	{
		//if (index > zMeshObjectStorage::get(*meshObj).faceActive.size()) throw std::invalid_argument(" error: index out of bounds.");
		//if (!zMeshObjectStorage::get(*meshObj).faceActive[index]) throw std::invalid_argument(" error: index out of bounds.");

		//// check if there is only 1 polygon. If true, cant perform collapse.
		//if (numPolygons() == 1)
		//{
		//	printf("\n Can't delete on single face mesh.");
		//	return;
		//}

		//// get faces vertices
		//vector<int> fVerts;
		//getVertices(index, zFaceData, fVerts);



		//// get face edges.
		//vector<int> fEdges;
		//getEdges(index, zFaceData, fEdges);

		//// connected edge for each face vertex
		//vector<int> fVertsValence;
		//for (int i = 0; i < fVerts.size(); i++)
		//{

		//	vector<int> cEdges;
		//	getConnectedEdges(fVerts[i], zVertexData, cEdges);
		//	fVertsValence.push_back(cEdges.size());

		//	// update vertex edge pointer if ther are pointing to face edges , as they will be disabled.

		//	for (int j = 0; j < cEdges.size(); j++)
		//	{
		//		bool chk = false;

		//		for (int k = 0; k < fEdges.size(); k++)
		//		{
		//			int sEdge = zMeshObjectStorage::get(*meshObj).edges[fEdges[k]].getSym()->getEdgeId();

		//			if (cEdges[j] == fEdges[k] || cEdges[j] == sEdge)
		//			{
		//				chk = true;
		//				break;
		//			}
		//		}

		//		if (!chk)
		//		{
		//			zMeshObjectStorage::get(*meshObj).vertices[fVerts[i]].setEdge(&zMeshObjectStorage::get(*meshObj).edges[cEdges[j]]);
		//			break;
		//		}
		//	}

		//}

		//// make face edges as  boundary edges, and disable them if both half edges have null face pointers.
		//for (int i = 0; i < fEdges.size(); i++)
		//{
		//	zMeshObjectStorage::get(*meshObj).edges[fEdges[i]].setFace(nullptr);

		//	int symEdge = zMeshObjectStorage::get(*meshObj).edges[fEdges[i]].getSym()->getEdgeId();

		//	if (onBoundary(fEdges[i], zEdgeData) && onBoundary(symEdge, zEdgeData))
		//	{
		//		deactivateElement(fEdges[i], zEdgeData);
		//	}
		//}

		//// get face vertices and deactivate them if all connected half edges are in active.
		//for (int i = 0; i < fVerts.size(); i++)
		//{
		//	bool removeVertex = true;
		//	if (fVertsValence[i] > 2) removeVertex = false;


		//	if (removeVertex)
		//	{
		//		deactivateElement(fVerts[i], zVertexData);
		//	}

		//}


		//// deactivate face
		//deactivateElement(index, zFaceData);


		//if (removeInactiveElems)
		//{
		//	removeInactiveElements(zVertexData);
		//	removeInactiveElements(zEdgeData);
		//	removeInactiveElements(zFaceData);
		//}

	}

	ZSPACE_INLINE void zFnMesh::deleteEdge(zItMeshEdge &edge, bool removeInactiveElements)
	{
		//// check if the edge is boundary.
		if (edge.onBoundary())
		{
			printf("\n Can't delete on boundary edge.");
			return;
		}

		int edgeId = edge.getId();

		zItMeshHalfEdge he = edge.getHalfEdge(0);
		zItMeshHalfEdge heS = edge.getHalfEdge(1);

		zItMeshHalfEdge he_next = he.getNext();
		zItMeshHalfEdge he_prev = he.getPrev();

		zItMeshHalfEdge heS_next = heS.getNext();
		zItMeshHalfEdge heS_prev = heS.getPrev();
		
		zItMeshFace he_face = he.getFace();
		zItMeshFace heS_face = heS.getFace();
		int remove_faceId = heS_face.getId();

		// set face pointer of heS_face to he_face
		zItMeshHalfEdgeArray heS_faceEdges;
		heS_face.getHalfEdges(heS_faceEdges);

		for (auto & tmpHe : heS_faceEdges) tmpHe.setFace(he_face);
		
		// set next and prev pointers
		he_prev.setNext(heS_next);
		he_next.setPrev(heS_prev);

		// set he_face edge pointer
		he_face.setHalfEdge(he_next);

		if (he.getVertex().getHalfEdge() == heS) he.getVertex().setHalfEdge(he_next);


		if (heS.getVertex().getHalfEdge() == he) he.getVertex().setHalfEdge(heS_next);

		// deactivate face and symmetry half edge
		removeFromHalfEdgesMap(he);
		he.deactivate();

		heS.deactivate();
		edge.deactivate();

		heS_face.deactivate();

		// update indices and clean up
		for (auto &f : zMeshObjectStorage::get(*meshObj).fHandles) if (f.he > (edgeId * 2) + 1) f.he -= 2;
		for (auto &v: zMeshObjectStorage::get(*meshObj).vHandles) if (v.he > (edgeId * 2) + 1) v.he -= 2;
		for (auto &he : zMeshObjectStorage::get(*meshObj).heHandles)
		{
			if (he.n > (edgeId * 2) + 1) he.n -= 2;
			if (he.p > (edgeId * 2) + 1) he.p -= 2;
			if (he.f > remove_faceId) he.f -= 1;			
		}
		
		garbageCollection(zVertexData);
		garbageCollection(zFaceData);
		garbageCollection(zEdgeData);	

		computeMeshNormals();
	}

	//---- TOPOLOGY MODIFIER METHODS

	ZSPACE_INLINE void zFnMesh::collapseEdge(zItMeshEdge &edge, double edgeFactor, bool removeInactiveElems )
	{
		//if (index > zMeshObjectStorage::get(*meshObj).edgeActive.size()) throw std::invalid_argument(" error: index out of bounds.");
		//if (!zMeshObjectStorage::get(*meshObj).edgeActive[index]) throw std::invalid_argument(" error: index out of bounds.");

		//int nFVerts = (edge.onBoundary()) ? 0 : getNumPolygonVertices( zMeshObjectStorage::get(*meshObj).edges[index].getFace()->getFaceId());

		//int sEdge = zMeshObjectStorage::get(*meshObj).edges[index].getSym()->getEdgeId();
		//int nFVerts_Sym = (onBoundary(sEdge, zEdgeData)) ? 0 : getNumPolygonVertices( zMeshObjectStorage::get(*meshObj).edges[sEdge].getFace()->getFaceId());

		//// check if there is only 1 polygon and its a triangle. If true, cant perform collapse.
		//if (numPolygons() == 1)
		//{
		//	if (nFVerts == 3 || nFVerts_Sym == 3)
		//	{
		//		printf("\n Can't perform collapse on single trianglular face.");
		//		return;
		//	}

		//}

		//// get edge faces
		//vector<int> eFaces;
		//getFaces(index, zEdgeData, eFaces);

		//if (numPolygons() == eFaces.size())
		//{
		//	if (nFVerts == nFVerts_Sym && nFVerts_Sym == 3)
		//	{
		//		printf("\n Can't perform collapse on common edge of 2 triangular face mesh.");
		//		return;
		//	}

		//}

		//int v1 = zMeshObjectStorage::get(*meshObj).edges[index].getVertex()->getVertexId();
		//int v2 = zMeshObjectStorage::get(*meshObj).edges[sEdge].getVertex()->getVertexId();

		//int vertexRemoveID = v1;
		//int vertexRetainID = v2;

		//if (getVertexValence(v1) > getVertexValence(v2))
		//{
		//	vertexRemoveID = v2;
		//	vertexRetainID = v1;

		//	edgeFactor = 1 - edgeFactor;

		//}

		//// set new position of retained vertex
		//zVector e = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexRemoveID] - zMeshObjectStorage::get(*meshObj).vertexPositions[vertexRetainID];
		//double eLength = e.length();
		//e.normalize();

		//zMeshObjectStorage::get(*meshObj).vertexPositions[vertexRetainID] = zMeshObjectStorage::get(*meshObj).vertexPositions[vertexRetainID] + e * (edgeFactor * eLength);


		//// get connected edges of vertexRemoveID
		//vector<int> cEdges;
		//getConnectedEdges(vertexRemoveID, zVertexData, cEdges);


		//// get connected edges

		//int vNext = zMeshObjectStorage::get(*meshObj).edges[index].getNext()->getVertex()->getVertexId();
		//vector<int> cEdgesVNext;
		//getConnectedEdges(vNext, zVertexData, cEdgesVNext);

		//int vPrev = zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getVertex()->getVertexId();
		//vector<int> cEdgesVPrev;
		//getConnectedEdges(vPrev, zVertexData, cEdgesVPrev);

		//int vNext_sEdge = zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->getVertex()->getVertexId();
		//vector<int> cEdgesVNext_sEdge;
		//getConnectedEdges(vNext_sEdge, zVertexData, cEdgesVNext_sEdge);

		//int vPrev_sEdge = zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getVertex()->getVertexId();
		//vector<int> cEdgesVPrev_sEdge;
		//getConnectedEdges(vPrev_sEdge, zVertexData, cEdgesVPrev_sEdge);

		//// current edge 
		//if (nFVerts == 3)
		//{

		//	// update pointers
		//	zMeshObjectStorage::get(*meshObj).edges[index].getNext()->setNext(zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->getNext());
		//	zMeshObjectStorage::get(*meshObj).edges[index].getNext()->setPrev(zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->getPrev());

		//	zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->setPrev(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->setNext(nullptr);

		//	zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->setPrev(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->setNext(nullptr);

		//	zMeshObjectStorage::get(*meshObj).edges[index].getNext()->setFace(zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->getFace());

		//	if (zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->getFace())
		//	{
		//		zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->getFace()->setEdge(zMeshObjectStorage::get(*meshObj).edges[index].getNext());
		//		zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->setFace(nullptr);
		//	}

		//	// update vertex edge pointer if pointing to prev edge

		//	if (zMeshObjectStorage::get(*meshObj).vertices[vNext].getEdge()->getEdgeId() == zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getEdgeId())
		//	{
		//		for (int i = 0; i < cEdgesVNext.size(); i++)
		//		{
		//			if (cEdgesVNext[i] != zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getEdgeId())
		//			{
		//				zMeshObjectStorage::get(*meshObj).vertices[vNext].setEdge(&zMeshObjectStorage::get(*meshObj).edges[cEdgesVNext[i]]);
		//			}
		//		}
		//	}

		//	// update vertex edge pointer if pointing to prev edge

		//	if (zMeshObjectStorage::get(*meshObj).vertices[vPrev].getEdge()->getEdgeId() == zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->getEdgeId() || zMeshObjectStorage::get(*meshObj).vertices[vPrev].getEdge()->getEdgeId() == index)
		//	{
		//		for (int i = 0; i < cEdgesVPrev.size(); i++)
		//		{
		//			if (cEdgesVPrev[i] != zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym()->getEdgeId() && cEdgesVPrev[i] != index)
		//			{
		//				zMeshObjectStorage::get(*meshObj).vertices[vPrev].setEdge(&zMeshObjectStorage::get(*meshObj).edges[cEdgesVPrev[i]]);
		//			}
		//		}
		//	}

		//	// decativate prev edge
		//	deactivateElement(zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getEdgeId(), zEdgeData);

		//	// decativate next and sym pointer of the next edge are same, deactivate edge
		//	if (zMeshObjectStorage::get(*meshObj).edges[index].getNext()->getNext() == zMeshObjectStorage::get(*meshObj).edges[index].getNext()->getSym())
		//	{

		//		deactivateElement(zMeshObjectStorage::get(*meshObj).edges[index].getNext()->getEdgeId(), zEdgeData);
		//		deactivateElement(vNext, zVertexData);
		//	}

		//	// decativate prev and sym pointer of the next edge are same, deactivate edge
		//	else if (zMeshObjectStorage::get(*meshObj).edges[index].getNext()->getPrev() == zMeshObjectStorage::get(*meshObj).edges[index].getNext()->getSym())
		//	{
		//		deactivateElement(zMeshObjectStorage::get(*meshObj).edges[index].getNext()->getVertex()->getVertexId(), zVertexData);
		//		deactivateElement(vNext, zVertexData);
		//	}

		//	// deactivate face pointed by collapse edge
		//	deactivateElement(zMeshObjectStorage::get(*meshObj).edges[index].getFace()->getFaceId(), zFaceData);

		//	zMeshObjectStorage::get(*meshObj).edges[index].setFace(nullptr);

		//	zMeshObjectStorage::get(*meshObj).edges[index].setNext(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[index].setPrev(nullptr);

		//}
		//else
		//{
		//	// update vertex edge pointer if pointing to current edge
		//	if (zMeshObjectStorage::get(*meshObj).vertices[vPrev].getEdge()->getEdgeId() == zMeshObjectStorage::get(*meshObj).edges[index].getEdgeId())
		//	{
		//		zMeshObjectStorage::get(*meshObj).vertices[vPrev].setEdge(zMeshObjectStorage::get(*meshObj).edges[index].getPrev()->getSym());
		//	}

		//	// update pointers
		//	zMeshObjectStorage::get(*meshObj).edges[index].getNext()->setPrev(zMeshObjectStorage::get(*meshObj).edges[index].getPrev());

		//	zMeshObjectStorage::get(*meshObj).edges[index].setNext(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[index].setPrev(nullptr);
		//}

		//// symmetry edge 
		//if (nFVerts_Sym == 3)
		//{


		//	// update pointers
		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->setNext(zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->getNext());
		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->setPrev(zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->getPrev());

		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->setPrev(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->setNext(nullptr);

		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->setPrev(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->setNext(nullptr);

		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->setFace(zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->getFace());

		//	if (zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->getFace())
		//	{
		//		zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->getFace()->setEdge(zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext());
		//		zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->setFace(nullptr);
		//	}

		//	// update vertex edge pointer if pointing to prev edge

		//	if (zMeshObjectStorage::get(*meshObj).vertices[vNext_sEdge].getEdge()->getEdgeId() == zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getEdgeId())
		//	{
		//		for (int i = 0; i < cEdgesVNext_sEdge.size(); i++)
		//		{
		//			if (cEdgesVNext_sEdge[i] != zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getEdgeId())
		//			{
		//				zMeshObjectStorage::get(*meshObj).vertices[vNext_sEdge].setEdge(&zMeshObjectStorage::get(*meshObj).edges[cEdgesVNext_sEdge[i]]);
		//			}
		//		}
		//	}

		//	// update vertex edge pointer if pointing to prev edge

		//	if (zMeshObjectStorage::get(*meshObj).vertices[vPrev_sEdge].getEdge()->getEdgeId() == zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->getEdgeId() || zMeshObjectStorage::get(*meshObj).vertices[vPrev_sEdge].getEdge()->getEdgeId() == sEdge)
		//	{
		//		for (int i = 0; i < cEdgesVPrev_sEdge.size(); i++)
		//		{
		//			if (cEdgesVPrev_sEdge[i] != zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym()->getEdgeId() && cEdgesVPrev_sEdge[i] != sEdge)
		//			{
		//				zMeshObjectStorage::get(*meshObj).vertices[vPrev_sEdge].setEdge(&zMeshObjectStorage::get(*meshObj).edges[cEdgesVPrev_sEdge[i]]);
		//			}
		//		}
		//	}

		//	// decativate prev edge
		//	deactivateElement(zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getEdgeId(), zEdgeData);

		//	// decativate next and sym pointer of the next edge are same, deactivate edge
		//	if (zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->getNext() == zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->getSym())
		//	{
		//		deactivateElement(zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->getEdgeId(), zEdgeData);
		//		deactivateElement(vNext_sEdge, zVertexData);
		//	}

		//	// decativate prev and sym pointer of the next edge are same, deactivate edge
		//	else if (zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->getPrev() == zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->getSym())
		//	{
		//		deactivateElement(zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->getEdgeId(), zEdgeData);
		//		deactivateElement(vNext_sEdge, zVertexData);
		//	}

		//	// deactivate face pointed by collapse edge
		//	deactivateElement(zMeshObjectStorage::get(*meshObj).edges[sEdge].getFace()->getFaceId(), zFaceData);

		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].setFace(nullptr);

		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].setNext(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].setPrev(nullptr);

		//}
		//else
		//{
		//	// update vertex edge pointer if pointing to current edge
		//	if (zMeshObjectStorage::get(*meshObj).vertices[vPrev_sEdge].getEdge()->getEdgeId() == zMeshObjectStorage::get(*meshObj).edges[sEdge].getEdgeId())
		//	{
		//		zMeshObjectStorage::get(*meshObj).vertices[vPrev_sEdge].setEdge(zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev()->getSym());
		//	}

		//	// update pointers
		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].getNext()->setPrev(zMeshObjectStorage::get(*meshObj).edges[sEdge].getPrev());

		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].setNext(nullptr);
		//	zMeshObjectStorage::get(*meshObj).edges[sEdge].setPrev(nullptr);
		//}

		//// update connected edges verter pointer
		//for (int i = 0; i < cEdges.size(); i++)
		//{
		//	if (zMeshObjectStorage::get(*meshObj).edgeActive[cEdges[i]])
		//	{
		//		int v1 = zMeshObjectStorage::get(*meshObj).edges[cEdges[i]].getVertex()->getVertexId();
		//		int v2 = vertexRemoveID;
		//		zMeshObjectStorage::get(*meshObj).removeFromVerticesEdge(v1, v2);

		//		zMeshObjectStorage::get(*meshObj).edges[cEdges[i]].getSym()->setVertex(&zMeshObjectStorage::get(*meshObj).vertices[vertexRetainID]);

		//		zMeshObjectStorage::get(*meshObj).addToVerticesEdge(v1, vertexRetainID, cEdges[i]);
		//	}
		//}


		//// deactivate collapse edge
		//if (zMeshObjectStorage::get(*meshObj).edgeActive[index])
		//{
		//	deactivateElement(index, zEdgeData);
		//}

		//// deactivate vertexRemoveID
		//if (zMeshObjectStorage::get(*meshObj).vertexActive[vertexRemoveID])
		//{
		//	deactivateElement(vertexRemoveID, zVertexData);
		//}

		//// compute normals		
		//computeMeshNormals();


		//// remove inactive elements
		//if (removeInactiveElems)
		//{
		//	removeInactiveElements(zVertexData);
		//	removeInactiveElements(zEdgeData);
		//	removeInactiveElements(zFaceData);
		//}

	}

	ZSPACE_INLINE zItMeshVertex zFnMesh::splitEdge(zItMeshEdge &edge, double edgeFactor, bool checkDuplicates )
	{

		int edgeId = edge.getId();

		zItMeshHalfEdge he = edge.getHalfEdge(0);
		zItMeshHalfEdge heS = edge.getHalfEdge(1);

		zItMeshHalfEdge he_next = he.getNext();
		zItMeshHalfEdge he_prev = he.getPrev();

		zItMeshHalfEdge heS_next = heS.getNext();
		zItMeshHalfEdge heS_prev = heS.getPrev();

		zVector edgeDir = he.getVector();
		double  edgeLength = edgeDir.length();
		edgeDir.normalize();

		zVector newVertPos = he.getStartVertex().getPosition() + edgeDir * edgeFactor * edgeLength;

		int numOriginalVertices = numVertices();

		// check if vertex exists if not add new vertex
		zItMeshVertex newVertex;
		addVertex(newVertPos, checkDuplicates, newVertex);

		if (newVertex.getId() >= numOriginalVertices)
		{
			// remove from halfEdge vertices map
			removeFromHalfEdgesMap(he);

			// add new edges
			int v1 = newVertex.getId();
			int v2 = he.getVertex().getId();

			zItMeshHalfEdge newHe;
			bool edgesResize = addEdges(v1, v2, false, newHe);

			int newHeId = newHe.getId();

			// recompute iterators as adding edges might have changed the address
			edge = zItMeshEdge(*meshObj, edgeId);

			he = edge.getHalfEdge(0);
			heS = edge.getHalfEdge(1);

			he_next = he.getNext();
			he_prev = he.getPrev();

			heS_next = heS.getNext();
			heS_prev = heS.getPrev();

			newHe = zItMeshHalfEdge(*meshObj, newHeId);
			zItMeshHalfEdge newHeS = newHe.getSym();

			// update vertex pointers
			newVertex.setHalfEdge(newHe);
			he.getVertex().setHalfEdge(newHeS);

			//// update pointers
			he.setVertex(newVertex); // current hedge vertex pointer updated to new added vertex

			newHeS.setNext(heS); // new added symmetry hedge next pointer to point to the symmetry of current hedge
			newHeS.setPrev(heS_prev);

			if (!heS.onBoundary())
			{
				zItMeshFace heS_f = heS.getFace();
				newHeS.setFace(heS_f);
			}

			newHe.setPrev(he);
			newHe.setNext(he_next);

			if (!he.onBoundary())
			{
				zItMeshFace he_f = he.getFace();
				newHe.setFace(he_f);
			}

			// update verticesEdge map
			addToHalfEdgesMap(he);
		}
		return newVertex;
	}

	ZSPACE_INLINE zItMeshVertex zFnMesh::splitHalfEdge(zItMeshHalfEdge& hEdge, double edgeFactor, bool checkDuplicates)
	{

		zItMeshEdge edge = hEdge.getEdge();
		int edgeId = edge.getId();

		int heID = hEdge.getId();
		//zItMeshHalfEdge he = hEdge;
		zItMeshHalfEdge heS = hEdge.getSym();

		zItMeshHalfEdge he_next = hEdge.getNext();
		zItMeshHalfEdge he_prev = hEdge.getPrev();

		zItMeshHalfEdge heS_next = hEdge.getSym().getNext();
		zItMeshHalfEdge heS_prev = hEdge.getSym().getPrev();

		zVector edgeDir = hEdge.getVector();
		double  edgeLength = edgeDir.length();
		edgeDir.normalize();

		zVector newVertPos = hEdge.getStartVertex().getPosition() + edgeDir * edgeFactor * edgeLength;

		int numOriginalVertices = numVertices();

		// check if vertex exists if not add new vertex
		zItMeshVertex newVertex;
		addVertex(newVertPos, checkDuplicates, newVertex);

		//printf("\n newVert: %1.2f %1.2f %1.2f   %i ", newVertPos.x, newVertPos.y, newVertPos.z, newVertex.getId());

		if (newVertex.getId() >= numOriginalVertices)
		{		

			// remove from halfEdge vertices map
			removeFromHalfEdgesMap(hEdge);

			// add new edges
			int v1 = newVertex.getId();
			int v2 = hEdge.getVertex().getId();

			zItMeshHalfEdge newHe;
			bool edgesResize = addEdges(v1, v2, false, newHe);

			int newHeId = newHe.getId();

			// recompute iterators if resize is true
			//if (edgesResize)
			//{

				//edge = zItMeshEdge(*meshObj, edge.getId());
				edge = zItMeshEdge(*meshObj, edgeId);

				hEdge = zItMeshHalfEdge(*meshObj, heID);
				heS = hEdge.getSym();

				he_next = hEdge.getNext();
				he_prev = hEdge.getPrev();

				heS_next = heS.getNext();
				heS_prev = heS.getPrev();

				newHe = zItMeshHalfEdge(*meshObj, newHeId);

				//printf("\n working!");
			//}

			zItMeshHalfEdge newHeS = newHe.getSym();

			// update vertex pointers
			newVertex.setHalfEdge(newHe);
			hEdge.getVertex().setHalfEdge(newHeS);

			//// update pointers
			hEdge.setVertex(newVertex);		// current hedge vertex pointer updated to new added vertex
			//printf("\n he %i | %i %i ", hEdge.getId(), newVertex.getId(), hEdge.getVertex().getId());

			newHeS.setNext(heS); // new added symmetry hedge next pointer to point to the symmetry of current hedge
			newHeS.setPrev(heS_prev);

			if (!heS.onBoundary())
			{
				zItMeshFace heS_f = heS.getFace();
				newHeS.setFace(heS_f);
			}

			newHe.setPrev(hEdge);
			newHe.setNext(he_next);

			if (!hEdge.onBoundary())
			{
				zItMeshFace he_f = hEdge.getFace();
				newHe.setFace(he_f);
			}

			// update verticesEdge map
			addToHalfEdgesMap(hEdge);

			

		}

		//printf("\n after he %i | %i %i ", hEdge.getId(), newVertex.getId(), hEdge.getVertex().getId());
		return newVertex;
	}

	ZSPACE_INLINE int zFnMesh::detachEdge(int index) { return 0; }

	ZSPACE_INLINE void zFnMesh::flipTriangleEdge(zItMeshEdge &edge)
	{
		if(edge.onBoundary())
		{
			throw std::invalid_argument("\n Cannot flip boundary edge ");
			return;
		}

		if(edge.getHalfEdge(0).getFace().getNumVertices() != 3 || edge.getHalfEdge(1).getFace().getNumVertices() != 3)
		{
			throw std::invalid_argument("\n Cannot flip edge not shared by two Triangles. ");
			return;
		}

		zItMeshHalfEdge hEdgeToFlip = edge.getHalfEdge(0);
		zItMeshHalfEdge hEdgeToFlipSym = edge.getHalfEdge(1);

		zItMeshHalfEdge he_next = hEdgeToFlip.getNext();
		zItMeshHalfEdge he_prev = hEdgeToFlip.getPrev();

		zItMeshHalfEdge heS_next = hEdgeToFlipSym.getNext();
		zItMeshHalfEdge heS_prev = hEdgeToFlipSym.getPrev();
		
		//// remove fromhalfEdge map
		removeFromHalfEdgesMap(hEdgeToFlip);
		removeFromHalfEdgesMap(hEdgeToFlipSym);

		//// update pointers

		if (hEdgeToFlip.getVertex().getHalfEdge() == hEdgeToFlipSym)
		{
			zItMeshHalfEdge he = hEdgeToFlipSym.getPrev().getSym();
			hEdgeToFlip.getVertex().setHalfEdge(he);
		}

		if (hEdgeToFlipSym.getVertex().getHalfEdge() == hEdgeToFlip)
		{
			zItMeshHalfEdge he = hEdgeToFlip.getPrev().getSym();
			hEdgeToFlipSym.getVertex().setHalfEdge(he);
		}   

		hEdgeToFlip.setNext(he_prev);
		hEdgeToFlip.setPrev(heS_next);		

		hEdgeToFlipSym.setNext(heS_prev);
		hEdgeToFlipSym.setPrev(he_next);		

		he_prev.setNext(heS_next);
		heS_prev.setNext(he_next);		

		zItMeshFace f0 = hEdgeToFlip.getFace();
		hEdgeToFlip.getNext().setFace(f0);
		hEdgeToFlip.getPrev().setFace(f0);

		zItMeshFace f1 = hEdgeToFlipSym.getFace();
		hEdgeToFlipSym.getNext().setFace(f1);
		hEdgeToFlipSym.getPrev().setFace(f1);
				
		hEdgeToFlip.getFace().setHalfEdge(hEdgeToFlip);
		hEdgeToFlipSym.getFace().setHalfEdge(hEdgeToFlipSym);

		// update verticesEdge map
		addToHalfEdgesMap(hEdgeToFlip);
		addToHalfEdgesMap(hEdgeToFlipSym);
				
	}

	ZSPACE_INLINE void zFnMesh::splitFaces(vector<int> &edgeList, vector<double> &edgeFactor)
	{
		//if (edgeFactor.size() > 0)
		//{
		//	if (edgeList.size() != edgeFactor.size()) throw std::invalid_argument(" error: size of edgelist and edge factor dont match.");
		//}

		//int numOriginalVertices = zMeshObjectStorage::get(*meshObj).vertexActive.size();
		//int numOriginalEdges = zMeshObjectStorage::get(*meshObj).edgeActive.size();
		//int numOriginalFaces = zMeshObjectStorage::get(*meshObj).faceActive.size();

		//for (int i = 0; i < edgeList.size(); i++)
		//{
		//	if (edgeFactor.size() > 0) splitEdge( edgeList[i], edgeFactor[i]);
		//	else splitEdge( edgeList[i]);
		//}

		//for (int j = 0; j < edgeList.size(); j++)
		//{
		//	for (int i = 0; i < 2; i++)
		//	{
		//		zEdge *start = (i == 0) ? &zMeshObjectStorage::get(*meshObj).edges[edgeList[j]] : zMeshObjectStorage::get(*meshObj).edges[edgeList[j]].getSym();

		//		zEdge *e = start;

		//		if (!start->getFace()) continue;

		//		bool exit = false;

		//		int v1 = start->getVertex()->getVertexId();
		//		int v2 = start->getVertex()->getVertexId();

		//		do
		//		{
		//			if (e->getNext())
		//			{
		//				e = e->getNext();
		//				if (e->getVertex()->getVertexId() > numOriginalVertices)
		//				{
		//					v2 = e->getVertex()->getVertexId();
		//					exit = true;
		//				}
		//			}
		//			else exit = true;

		//		} while (e != start && !exit);

		//		// add new edges and face
		//		if (v1 == v2) continue;

		//		// check if edge exists continue loop. 
		//		int outEdgeId;
		//		bool eExists = zMeshObjectStorage::get(*meshObj).edgeExists(v1, v2, outEdgeId);

		//		if (eExists) continue;

		//		int startEdgeId = start->getEdgeId();
		//		int e_EdgeId = e->getEdgeId();

		//		bool resizeEdges = zMeshObjectStorage::get(*meshObj).addEdges(v1, v2);

		//		if (resizeEdges)
		//		{
		//			start = &zMeshObjectStorage::get(*meshObj).edges[startEdgeId];
		//			e = &zMeshObjectStorage::get(*meshObj).edges[e_EdgeId];
		//		}

		//		zMeshObjectStorage::get(*meshObj).addPolygon(); // empty polygon

		//							 // update pointers
		//		zEdge *start_next = start->getNext();
		//		zEdge *e_next = e->getNext();

		//		start->setNext(&zMeshObjectStorage::get(*meshObj).edges[numEdges() - 2]);
		//		e_next->setPrev(&zMeshObjectStorage::get(*meshObj).edges[numEdges() - 2]);

		//		start_next->setPrev(&zMeshObjectStorage::get(*meshObj).edges[numEdges() - 1]);
		//		e->setNext(&zMeshObjectStorage::get(*meshObj).edges[numEdges() - 1]);

		//		zMeshObjectStorage::get(*meshObj).faces[numPolygons() - 1].setEdge(start_next);

		//		// edge face pointers to new face
		//		zEdge *newFace_E = start_next;

		//		do
		//		{
		//			newFace_E->setFace(&zMeshObjectStorage::get(*meshObj).faces[numPolygons() - 1]);

		//			if (newFace_E->getNext()) newFace_E = newFace_E->getNext();
		//			else exit = true;

		//		} while (newFace_E != start_next && !exit);

		//	}

		//}


	}

	ZSPACE_INLINE void zFnMesh::splitFace(int faceID, int egdeID0, int egdeID1, float edge0_factor, float edge1_factor)
	{
		int numVerts_beforeSplit = numVertices();

		zItMeshFace face(*meshObj, faceID);
		
		int heID_0 = -1;
		int heID_1 = -1;
		
		zItMeshHalfEdgeArray fHEdges;
		face.getHalfEdges(fHEdges);

		for (auto& he : fHEdges)
		{
			if (he.getEdge().getId() == egdeID0) heID_0 = he.getId();
			if (he.getEdge().getId() == egdeID1) heID_1 = he.getId();
		}

		if (heID_0 == -1 || heID_1 == -1)
		{
			throw std::invalid_argument(" error: edge indicies provided are not part of the face. "); ; 
			return;
		}

		zItMeshHalfEdge he0(*meshObj, heID_0);
		zItMeshHalfEdge he1(*meshObj, heID_1);

		zItMeshVertex v0 = splitHalfEdge(he0, edge0_factor, true);			
		zItMeshVertex v1 = splitHalfEdge(he1, edge1_factor, true);

		zItMeshHalfEdge he;
		addEdges(v0.getId(), v1.getId(), true, he);

		// recompute iterators, as adding edge might have moved the address
		he0 = zItMeshHalfEdge(*meshObj, heID_0);
		he1 = zItMeshHalfEdge(*meshObj, heID_1);

		if (edge0_factor == 0.0) he0 = he0.getPrev();
		if (edge1_factor == 0.0) he1 = he1.getPrev();

		zItMeshHalfEdge he0_next = he0.getNext();
		zItMeshHalfEdge he1_next = he1.getNext();

		he.setPrev(he0);
		he.setNext(he1_next);
		
		he.getSym().setNext(he0_next);
		he.getSym().setPrev(he1);

		zItMeshHalfEdge face_he = face.getHalfEdge();
		zItMeshHalfEdge newFace_he;

		bool exit = false;
		do
		{
			if (he == face_he)
			{
				face.setHalfEdge(he);
				newFace_he = he.getSym();
				exit = true;
			}

			else if (he.getSym() == face_he )
			{
				zItMeshHalfEdge newHE = he.getSym();
				face.setHalfEdge(newHE);

				newFace_he = he;
				exit = true;
			}

			if(!exit) face_he = face_he.getNext();

		} while (!exit);

		zItMeshHalfEdge tmp_he = face_he;
		do
		{
			
			tmp_he.setFace(face);
			tmp_he = tmp_he.getNext();

		} while (tmp_he != face_he);

		zIntArray fVerts0;
		face.getVertices(fVerts0);

		/*printf("\n face verts Ids : ");
		for (auto fV : fVerts0) printf(" %i ", fV);*/

		// compute new face vertex
		zIntArray fVerts;
		zItMeshFace fNew;
		addPolygon(fNew);
		fNew.setHalfEdge(newFace_he);

		zItMeshHalfEdge new_he = newFace_he;
		do
		{
			fVerts.push_back(new_he.getSym().getVertex().getId());
			new_he.setFace(fNew);
			new_he = new_he.getNext();

		} while (new_he != newFace_he);

		/*printf("\n new face vert Ids : ");
		for (auto fV : fVerts) printf(" %i ", fV);*/

		
		computeMeshNormals();
		
	}

	ZSPACE_INLINE void zFnMesh::subdivide(int numDivisions)
	{
		for (int j = 0; j < numDivisions; j++)
		{

			int numOriginalVertices = numVertices();

			// split edges at center
			int numOriginalHalfEdges = numHalfEdges();

			int numOrginalEdges = numEdges();
			

			for (int i = 0; i < numOrginalEdges; i++)
			{
				zItMeshEdge e(*meshObj,i);

				if (e.isActive()) splitEdge(e);
			}

			// get face centers
			vector<zVector> fCenters;
			getCenters(zFaceData, fCenters);

			// add faces
			int numOriginalfaces = numPolygons();
			
			for (int i = 0; i < numOriginalfaces; i++)
			{
				zItMeshFace f(*meshObj, i);

				if (!f.isActive()) continue;

				zIntArray fEdges;
				f.getHalfEdges(fEdges);

				// disable current face
				//f.deactivate();

				// check if vertex exists if not add new vertex
				zItMeshVertex vertexCen;
				addVertex(fCenters[i], true, vertexCen);

				// add new faces				
				int startId = 0;
				zItMeshHalfEdge he_0(*meshObj, fEdges[0]);
				if (he_0.getVertex().getId() < numOriginalVertices) startId = 1;

				for (int k = startId; k < fEdges.size() + startId; k += 2)
				{
					vector<int> newFVerts;

					zItMeshHalfEdge he(*meshObj, fEdges[k]);
					newFVerts.push_back(he.getVertex().getId());

					newFVerts.push_back(vertexCen.getId());

					newFVerts.push_back(he.getPrev().getStartVertex().getId());

					newFVerts.push_back(he.getPrev().getVertex().getId());


					if (k == startId)
					{
						updatePolygon(f, newFVerts);
					}
					else
					{
						zItMeshFace newF;
						addPolygon(newFVerts, newF);
					}

				}
			}
	
			// update half edge handles. 
			//for (int i = 0; i < zMeshObjectStorage::get(*meshObj).heHandles.size(); i++)
			//{
			//	if(zMeshObjectStorage::get(*meshObj).heHandles[i].f != -1) zMeshObjectStorage::get(*meshObj).heHandles[i].f -= numOriginalfaces;
			//}
			//		
			//// remove inactive faces
			//garbageCollection(zFaceData);

			computeMeshNormals();
		}
	}

	ZSPACE_INLINE void zFnMesh::smoothMesh(int numDivisions, bool smoothCorner)
	{
		vector<zVector> fCenters;
		vector<zVector> tempECenters;
		vector<zVector> eCenters;

		for (int j = 0; j < numDivisions; j++)
		{
			// get face centers
			fCenters.clear();
			getCenters(zFaceData, fCenters);

			// get edge centers
		
			tempECenters.clear();

			eCenters.clear();
			getCenters(zEdgeData, eCenters);

			tempECenters = eCenters;

			zVector* vPositions = 	getRawVertexPositions();

			int numOriginalVertices = numVertices();
			int numOriginalEdges = numEdges();

			// compute new smooth positions of the edge centers
			for (int i =0; i< numOriginalEdges; i++)
			{

				zItMeshEdge e(*meshObj, i);

				if (e.onBoundary()) continue;

				zVector newPos;			

				vector<int> eVerts;
				e.getVertices(eVerts);
				for (auto &vId : eVerts) newPos += vPositions[vId];


				vector<int> eFaces;
				e.getFaces(eFaces);
				for (auto &fId : eFaces) newPos += fCenters[fId];

				newPos /= (eFaces.size() + eVerts.size());

				eCenters[i] = newPos;
			}

			// compute new smooth positions for the original vertices
			for (int i = 0; i < numOriginalVertices; i++)
			{
				zItMeshVertex v(*meshObj, i);

				if (v.onBoundary())
				{
					vector<zItMeshEdge> cEdges;
					v.getConnectedEdges(cEdges);

					if (!smoothCorner && cEdges.size() == 2) continue;

					zVector P = vPositions[i];
					//int n = 1; // rosetta , not matching with maya
					int n = 0;

					zVector R(0,0,0);
					for (auto &e : cEdges)
					{
						if (e.onBoundary())
						{
							R += tempECenters[e.getId()];
							n++;
						}
					}

					// rosetta , not matching with maya
					//vPositions[i] = (P + R) / n; 

					vPositions[i] = (P / n) + (R / (n*n));				
				}
				else
				{
					zVector R;

					vector<int> cEdges;
					v.getConnectedEdges(cEdges);

					for (auto &eId : cEdges) R += tempECenters[eId];
					R /= cEdges.size();

					zVector F;
					vector<int> cFaces;
					v.getConnectedFaces(cFaces);
					for (auto &fId : cFaces) F += fCenters[fId];
					F /= cFaces.size();

					zVector P = vPositions[i];
					int n = cFaces.size();

					vPositions[i] = (F + (R * 2) + (P * (n - 3))) / n;
				}
			}

			// split edges at center			
			for (int i = 0; i < numOriginalEdges; i++)
			{
				zItMeshEdge e(*meshObj,i);
				if (e.isActive())
				{			
					zItMeshHalfEdge he = e.getHalfEdge(0);
					zItMeshVertex newVert = splitHalfEdge(he,0.5,false);
					newVert.setPosition(eCenters[i]);
				}
			}



			// add faces
			int numOriginalFaces = numPolygons();
			
			for (int i = 0; i < numOriginalFaces; i++)
			{
				zItMeshFace f(*meshObj, i);

				if (!f.isActive()) continue;

				zIntArray fEdges;
				f.getHalfEdges(fEdges);	

				// disable current face
				//f.deactivate();

				// check if vertex exists if not add new vertex
				zItMeshVertex vertexCen;
				addVertex(fCenters[i], true, vertexCen);

				// add new faces				
				int startId = 0;
				zItMeshHalfEdge he_0(*meshObj, fEdges[0]);
				if (he_0.getVertex().getId() < numOriginalVertices) startId = 1;

				for (int k = startId; k < fEdges.size() + startId; k += 2)
				{
					vector<int> newFVerts;

					zItMeshHalfEdge he(*meshObj, fEdges[k]);
					newFVerts.push_back(he.getVertex().getId());

					newFVerts.push_back(vertexCen.getId());

					newFVerts.push_back(he.getPrev().getStartVertex().getId());

					newFVerts.push_back(he.getPrev().getVertex().getId());


					if (k == startId)
					{
						updatePolygon(f, newFVerts);
					}
					else
					{
						zItMeshFace newF;
						addPolygon(newFVerts, newF);
					}
					
				}

			
			}

			// update half edge handles. 
			/*for (int i = 0; i < zMeshObjectStorage::get(*meshObj).heHandles.size(); i++)
			{
				if (zMeshObjectStorage::get(*meshObj).heHandles[i].f != -1)
				{
					zMeshObjectStorage::get(*meshObj).heHandles[i].f -= numOriginalFaces;
					zMeshObjectStorage::get(*meshObj).halfEdges[i].setFace(zMeshObjectStorage::get(*meshObj).heHandles[i].f);
				}
			}*/

			/*for (zItMeshHalfEdge he(*meshObj); !he.end(); he++)
			{
				printf("\n %i |n %i p %i | v %i %i | f %i", he.getId(), he.getNext().getId(), he.getPrev().getId(), he.getStartVertex().getId(), he.getVertex().getId(), (!he.onBoundary()) ? he.getFace().getId() : -2);
			}*/

			// remove inactive faces
			//garbageCollection(zFaceData);
			//printf("\n faces a %i ", numPolygons());

			

			computeMeshNormals();
		}	
	}


	ZSPACE_INLINE void zFnMesh::extrudeMesh(float extrudeThickness,zObjMesh &out, bool thicknessTris)
	{
		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() == 0 || zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();
		

		vector<zVector> positions;
		vector<int> polyCounts;
		vector<int> polyConnects;

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[i]);
		}

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[i] + (zMeshObjectStorage::get(*meshObj).vertexNormals[i] * extrudeThickness));
		}

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			vector<int> fVerts;
			f.getVertices(fVerts);

			for (int j = 0; j < fVerts.size(); j++)
			{
				polyConnects.push_back(fVerts[j]);
			}

			polyCounts.push_back(fVerts.size());

			for (int j = fVerts.size() - 1; j >= 0; j--)
			{
				polyConnects.push_back(fVerts[j] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
			}

			polyCounts.push_back(fVerts.size());
		}

		for (zItMeshHalfEdge he(*meshObj); !he.end(); he++)
		{
			if (he.onBoundary())
			{
				vector<int> eVerts;
				he.getVertices(eVerts);

				if (thicknessTris)
				{
					polyConnects.push_back(eVerts[1]);
					polyConnects.push_back(eVerts[0]);
					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());

					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1]);

					polyCounts.push_back(3);
					polyCounts.push_back(3);
				}
				else
				{
					polyConnects.push_back(eVerts[1]);
					polyConnects.push_back(eVerts[0]);
					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());


					polyCounts.push_back(4);
				}
			}
		}

		zFnMesh tempFn(out);

		tempFn.clear();
		tempFn.create(positions, polyCounts, polyConnects);		
		
	}

	ZSPACE_INLINE void zFnMesh::extrudeVariableMesh(zFloatArray extrudeThickness, zObjMesh& out, bool bothSides, bool thicknessTris)
	{
		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() == 0 || zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();

		if (extrudeThickness.size() != numVertices()) return;

		vector<zVector> positions;
		vector<int> polyCounts;
		vector<int> polyConnects;

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			if(!bothSides) positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[i]);
			else positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[i] + (zMeshObjectStorage::get(*meshObj).vertexNormals[i] * extrudeThickness[i] * -1));
		}

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[i] + (zMeshObjectStorage::get(*meshObj).vertexNormals[i] * extrudeThickness[i]));
		}

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			vector<int> fVerts;
			f.getVertices(fVerts);

			for (int j = 0; j < fVerts.size(); j++)
			{
				polyConnects.push_back(fVerts[j]);
			}

			polyCounts.push_back(fVerts.size());

			for (int j = fVerts.size() - 1; j >= 0; j--)
			{
				polyConnects.push_back(fVerts[j] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
			}

			polyCounts.push_back(fVerts.size());
		}

		for (zItMeshHalfEdge he(*meshObj); !he.end(); he++)
		{
			if (he.onBoundary())
			{
				vector<int> eVerts;
				he.getVertices(eVerts);

				if (thicknessTris)
				{
					polyConnects.push_back(eVerts[1]);
					polyConnects.push_back(eVerts[0]);
					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());

					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1]);

					polyCounts.push_back(3);
					polyCounts.push_back(3);
				}
				else
				{
					polyConnects.push_back(eVerts[1]);
					polyConnects.push_back(eVerts[0]);
					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());


					polyCounts.push_back(4);
				}
			}
		}

		zFnMesh tempFn(out);

		tempFn.clear();
		tempFn.create(positions, polyCounts, polyConnects);

	}

	ZSPACE_INLINE void zFnMesh::extrudeBoundaryEdge(float extrudeThickness, zObjMesh &out, bool thicknessTris)
	{
		if (zMeshObjectStorage::get(*meshObj).faceNormals.size() == 0 || zMeshObjectStorage::get(*meshObj).faceNormals.size() != zMeshObjectStorage::get(*meshObj).faces.size()) computeMeshNormals();


		vector<zVector> positions;
		vector<int> polyCounts;
		vector<int> polyConnects;


		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[i]);
		}

		for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vertexPositions.size(); i++)
		{
			positions.push_back(zMeshObjectStorage::get(*meshObj).vertexPositions[i] + (zMeshObjectStorage::get(*meshObj).vertexNormals[i] * extrudeThickness));
		}	


		for (zItMeshHalfEdge he(*meshObj); !he.end(); he++)
		{
			if (he.onBoundary())
			{
				vector<int> eVerts;
				he.getVertices(eVerts);

				if (thicknessTris)
				{
					polyConnects.push_back(eVerts[1]);
					polyConnects.push_back(eVerts[0]);
					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());

					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1]);

					polyCounts.push_back(3);
					polyCounts.push_back(3);
				}
				else
				{
					polyConnects.push_back(eVerts[1]);
					polyConnects.push_back(eVerts[0]);
					polyConnects.push_back(eVerts[0] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());
					polyConnects.push_back(eVerts[1] + zMeshObjectStorage::get(*meshObj).vertexPositions.size());


					polyCounts.push_back(4);
				}


			}
		}

		zFnMesh tempFn(out);

		tempFn.clear();
		tempFn.create(positions, polyCounts, polyConnects);

	}

	//---- TRANSFORM METHODS OVERRIDES

	ZSPACE_INLINE void zFnMesh::setTransform(zTransform &inTransform, bool decompose, bool updatePositions)
	{
		if (updatePositions)
		{
			zTransformationMatrix to;
			to.setTransform(inTransform, decompose);

			zTransform transMat = meshObj->transformationMatrix.getToMatrix(to);
			transformObject(transMat);

			meshObj->transformationMatrix.setTransform(inTransform);

			// update pivot values of object transformation matrix
			zVector p = meshObj->transformationMatrix.getPivot();
			p = p * transMat;
			setPivot(p);
		}
		else
		{
			meshObj->transformationMatrix.setTransform(inTransform, decompose);

			zVector p = meshObj->transformationMatrix.getO();
			setPivot(p);
		}
	}

	ZSPACE_INLINE void zFnMesh::setScale(zFloat4 &scale)
	{
		// get  inverse pivot translations
		zTransform invScalemat = meshObj->transformationMatrix.asInverseScaleTransformMatrix();

		// set scale values of object transformation matrix
		meshObj->transformationMatrix.setScale(scale);

		// get new scale transformation matrix
		zTransform scaleMat = meshObj->transformationMatrix.asScaleTransformMatrix();

		// compute total transformation
		zTransform transMat = invScalemat * scaleMat;

		// transform object
		transformObject(transMat);
	}

	ZSPACE_INLINE void zFnMesh::setRotation(zFloat4 &rotation, bool appendRotations)
	{
		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = meshObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = meshObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = meshObj->transformationMatrix;
		to.setRotation(rotation, appendRotations);
		zTransform toMat = meshObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set rotation values of object transformation matrix
		meshObj->transformationMatrix.setRotation(rotation, appendRotations);;
	}

	ZSPACE_INLINE void zFnMesh::setTranslation(zVector &translation, bool appendTranslations)
	{
		// get vector as zDouble3
		zFloat4 t;
		translation.getComponents(t);

		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = meshObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = meshObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = meshObj->transformationMatrix;
		to.setTranslation(t, appendTranslations);
		zTransform toMat = meshObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set translation values of object transformation matrix
		meshObj->transformationMatrix.setTranslation(t, appendTranslations);;

		// update pivot values of object transformation matrix
		zVector p = meshObj->transformationMatrix.getPivot();
		p = p * transMat;
		setPivot(p);
	}

	ZSPACE_INLINE void zFnMesh::setPivot(zVector &pivot)
	{
		// get vector as zDouble3
		zFloat4 p;
		pivot.getComponents(p);

		// set pivot values of object transformation matrix
		meshObj->transformationMatrix.setPivot(p);
	}

	ZSPACE_INLINE void zFnMesh::getTransform(zTransform &transform)
	{
		transform = meshObj->transformationMatrix.asMatrix();
	}

	//---- PROTECTED TRANSFORM  METHODS

	ZSPACE_INLINE void zFnMesh::transformObject(zTransform &transform)
	{
		if (numVertices() == 0) return;

		zVector* pos = getRawVertexPositions();

		for (int i = 0; i < numVertices(); i++)
		{
			zVector newPos = pos[i] * transform;
			pos[i] = newPos;
		}
	}

	//---- FACTORY METHODS

	//---- PROTECTED CONTOUR METHODS

	ZSPACE_INLINE int zFnMesh::getIsolineCase_triangle(bool vertexBinary[3])
	{
		int out = -1;

		if (vertexBinary[0] && vertexBinary[1] && vertexBinary[2] ) out = 0;

		if (!vertexBinary[0] && vertexBinary[1] && vertexBinary[2] ) out = 1;

		if (vertexBinary[0] && !vertexBinary[1] && vertexBinary[2]) out = 2;

		if (vertexBinary[0] && vertexBinary[1] && !vertexBinary[2]) out = 3;

		if (!vertexBinary[0] && !vertexBinary[1] && !vertexBinary[2]) out = 4;

		if (vertexBinary[0] && !vertexBinary[1] && !vertexBinary[2]) out = 5;

		if (!vertexBinary[0] && vertexBinary[1] && !vertexBinary[2]) out = 6;

		if (!vertexBinary[0] && !vertexBinary[1] && vertexBinary[2]) out = 7;

		return out;
	}

	ZSPACE_INLINE int zFnMesh::getIsolineCase(bool vertexBinary[4])
	{
		int out = -1;

		if (vertexBinary[0] && vertexBinary[1] && vertexBinary[2] && vertexBinary[3]) out = 0;

		if (!vertexBinary[0] && vertexBinary[1] && vertexBinary[2] && vertexBinary[3]) out = 1;

		if (vertexBinary[0] && !vertexBinary[1] && vertexBinary[2] && vertexBinary[3]) out = 2;

		if (!vertexBinary[0] && !vertexBinary[1] && vertexBinary[2] && vertexBinary[3]) out = 3;

		if (vertexBinary[0] && vertexBinary[1] && !vertexBinary[2] && vertexBinary[3]) out = 4;

		if (!vertexBinary[0] && vertexBinary[1] && !vertexBinary[2] && vertexBinary[3]) out = 5;

		if (vertexBinary[0] && !vertexBinary[1] && !vertexBinary[2] && vertexBinary[3]) out = 6;

		if (!vertexBinary[0] && !vertexBinary[1] && !vertexBinary[2] && vertexBinary[3]) out = 7;

		if (vertexBinary[0] && vertexBinary[1] && vertexBinary[2] && !vertexBinary[3]) out = 8;

		if (!vertexBinary[0] && vertexBinary[1] && vertexBinary[2] && !vertexBinary[3]) out = 9;

		if (vertexBinary[0] && !vertexBinary[1] && vertexBinary[2] && !vertexBinary[3]) out = 10;

		if (!vertexBinary[0] && !vertexBinary[1] && vertexBinary[2] && !vertexBinary[3]) out = 11;

		if (vertexBinary[0] && vertexBinary[1] && !vertexBinary[2] && !vertexBinary[3]) out = 12;

		if (!vertexBinary[0] && vertexBinary[1] && !vertexBinary[2] && !vertexBinary[3]) out = 13;

		if (vertexBinary[0] && !vertexBinary[1] && !vertexBinary[2] && !vertexBinary[3]) out = 14;

		if (!vertexBinary[0] && !vertexBinary[1] && !vertexBinary[2] && !vertexBinary[3]) out = 15;

		return out;
	}

	ZSPACE_INLINE int zFnMesh::getIsobandCase(int vertexTernary[4])
	{
		int out = -1;

		// No Contour
		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 0;

		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 1;


		// Single Triangle

		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 2;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 3;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 4;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 5;


		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 6;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 7;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 8;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 9;

		// Single Trapezoid

		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 10;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 11;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 12;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 13;


		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 14;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 15;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 16;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 17;

		// Single Rectangle

		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 18;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 19;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 20;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 21;


		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 22;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 23;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 24;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 25;


		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 26;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 27;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 28;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 29;

		// Single Square

		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 30;

		// Single Pentagon

		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 31;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 32;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 33;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 34;


		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 35;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 36;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 37;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 38;

		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 39;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 40;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 41;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 42;

		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 43;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 44;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 45;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 46;

		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 47;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 48;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 49;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 0) out = 50;

		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 51;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 52;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 53;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 2) out = 54;

		// Single Hexagon

		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 55;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 56;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 57;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 58;

		if (vertexTernary[0] == 1 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 59;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 60;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 1) out = 61;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 62;

		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 63;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 64;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 65;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 66;


		// Saddles:  1 or 2 polygon

		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 67;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 68;

		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 69;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 70;

		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 71;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 72;

		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 1 && vertexTernary[3] == 2) out = 73;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 74;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 1) out = 75;
		if (vertexTernary[0] == 2 && vertexTernary[1] == 1 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 76;

		if (vertexTernary[0] == 2 && vertexTernary[1] == 0 && vertexTernary[2] == 1 && vertexTernary[3] == 0) out = 77;
		if (vertexTernary[0] == 1 && vertexTernary[1] == 0 && vertexTernary[2] == 2 && vertexTernary[3] == 0) out = 78;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 2 && vertexTernary[2] == 0 && vertexTernary[3] == 1) out = 79;
		if (vertexTernary[0] == 0 && vertexTernary[1] == 1 && vertexTernary[2] == 0 && vertexTernary[3] == 2) out = 80;

		return out;

	}

	ZSPACE_INLINE zVector zFnMesh::getContourPosition(float& threshold, zVector & vertex_lower, zVector & vertex_higher, float& thresholdLow, float& thresholdHigh)
	{
		float scaleVal = coreUtils.ofMap(threshold, thresholdLow, thresholdHigh, 0.0000f, 1.0000f);

		zVector e = vertex_higher - vertex_lower;
		double edgeLen = e.length();
		e.normalize();

		return (vertex_lower + (e * edgeLen *scaleVal));
	}

	ZSPACE_INLINE void zFnMesh::getIsoline(zScalarArray& vertexScalars, zItMeshFace& f, zPointArray& positions, zIntArray& edgeConnects, zColorArray& cVertexColor, unordered_map <string, int>& positionVertex, float& threshold, int precision, float distTolerance)
	{
		vector<zItMeshVertex> fVerts;
		f.getVertices(fVerts);

		if (fVerts.size() != 4 && fVerts.size() != 3) return;

		// tri mesh
		if (fVerts.size() == 3)
		{
			// chk if all the face vertices are below the threshold
			bool vertexBinary[3];
			float averageScalar = 0;

			for (int j = 0; j < fVerts.size(); j++)
			{
				if (vertexScalars[fVerts[j].getId()] < threshold)
				{
					vertexBinary[j] = true;
				}
				else vertexBinary[j] = false;

				averageScalar += vertexScalars[fVerts[j].getId()];
			}

			averageScalar /= fVerts.size();

			int MS_case = getIsolineCase_triangle(vertexBinary);

			vector<zVector> newPositions;
			zColorArray newColors;

			// CASE 0 or 4
			if (MS_case == 0 || MS_case == 0)
			{
				// No Veritices to be added 
			}

			// CASE 1 or 5
			if (MS_case == 1 || MS_case == 5)
			{
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos1 = (getContourPosition(threshold, v1, v0, s1, s0));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[0].getId(), fVerts[1].getId(), he1);


				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[0].getId(), fVerts[2].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 2 or 6
			if (MS_case == 2 || MS_case == 6)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos1 = (getContourPosition(threshold, v1, v0, s1, s0));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[1].getId(), fVerts[2].getId(), he1);


				v1 = fVerts[0].getPosition();
				s1 = vertexScalars[fVerts[0].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[1].getId(), fVerts[0].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 3 or 7
			if (MS_case == 3 || MS_case == 7)
			{
				zVector v0 = fVerts[2].getPosition();
				float s0 = vertexScalars[fVerts[2].getId()];

				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos1 = (getContourPosition(threshold, v1, v0, s1, s0));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[2].getId(), fVerts[0].getId(), he1);


				v1 = fVerts[1].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[2].getId(), fVerts[1].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// check for edge lengths

			if (newPositions.size() == 2)
			{
				zPoint p0 = coreUtils.factorise(newPositions[0], precision);
				zPoint p1 = coreUtils.factorise(newPositions[1], precision);

				//zPoint p0 = newPositions[0];
				//zPoint p1 = newPositions[1];

				if (p0.distanceTo(p1) < distTolerance)
					newPositions.clear();
			}



			// compute edge 
			if (newPositions.size() == 2)
			{
				for (int i = 0; i < newPositions.size(); i++)
				{
					zVector p0 = newPositions[i];
					int v0;

					//bool vExists = coreUtils.vertexExists(positionVertex, p0, PRECISION, v0);
					bool vExists = coreUtils.checkRepeatVector(p0, positions, v0, precision);


					if (!vExists)
					{
						v0 = positions.size();
						positions.push_back(p0);
						cVertexColor.push_back(newColors[i]);

						//string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
						//positionVertex[hashKey] = v0;

						//coreUtils.addToPositionMap(positionVertex, p0, v0, PRECISION);
					}

					edgeConnects.push_back(v0);
				}

			}

		}

		// quad mesh
		if (fVerts.size() == 4)
		{
			// chk if all the face vertices are below the threshold
			bool vertexBinary[4];
			float averageScalar = 0;

			for (int j = 0; j < fVerts.size(); j++)
			{
				if (vertexScalars[fVerts[j].getId()] < threshold)
				{
					vertexBinary[j] = true;
				}
				else vertexBinary[j] = false;

				averageScalar += vertexScalars[fVerts[j].getId()];
			}

			averageScalar /= fVerts.size();

			int MS_case = getIsolineCase(vertexBinary);


			vector<zVector> newPositions;
			zColorArray newColors;

			vector<zVector> newPositions2;
			zColorArray newColors2;


			// CASE 0
			if (MS_case == 0)
			{
				// No Veritices to be added 
			}

			// CASE 1
			if (MS_case == 1)
			{
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos1 = (getContourPosition(threshold, v1, v0, s1, s0));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[0].getId(), fVerts[1].getId(), he1);


				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[0].getId(), fVerts[3].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 2
			if (MS_case == 2)
			{
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[0].getId(), fVerts[1].getId(), he2);

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[2].getId(), fVerts[1].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 3
			if (MS_case == 3)
			{
				zVector v0 = fVerts[3].getPosition();
				float s0 = vertexScalars[fVerts[3].getId()];

				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[3].getId(), fVerts[0].getId(), he2);

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];

				v1 = fVerts[1].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[2].getId(), fVerts[1].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

			}

			// CASE 4
			if (MS_case == 4)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[1].getId(), fVerts[2].getId(), he2);

				v0 = fVerts[3].getPosition();
				s0 = vertexScalars[fVerts[3].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[3].getId(), fVerts[2].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 5
			if (MS_case == 5)
			{
				// SADDLE CASE 

				// tri 1
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[1].getId(), fVerts[0].getId(), he1);

				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];
				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[1].getId(), fVerts[2].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

				// tri 2
				v0 = fVerts[3].getPosition();
				s0 = vertexScalars[fVerts[3].getId()];
				zVector pos4 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he4;
				halfEdgeExists(fVerts[3].getId(), fVerts[2].getId(), he4);

				v1 = fVerts[0].getPosition();
				s1 = vertexScalars[fVerts[0].getId()];
				zVector pos3 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he3;
				halfEdgeExists(fVerts[3].getId(), fVerts[0].getId(), he3);

				newPositions2.push_back(pos3);
				newPositions2.push_back(pos4);

				newColors2.push_back(he3.getColor());
				newColors2.push_back(he4.getColor());

			}

			// CASE 6
			if (MS_case == 6)
			{
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[0].getId(), fVerts[1].getId(), he2);

				v0 = fVerts[3].getPosition();
				s0 = vertexScalars[fVerts[3].getId()];
				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[3].getId(), fVerts[2].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

			}


			// CASE 7
			if (MS_case == 7)
			{
				zVector v0 = fVerts[3].getPosition();
				float s0 = vertexScalars[fVerts[3].getId()];
				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[3].getId(), fVerts[2].getId(), he1);

				v1 = fVerts[0].getPosition();
				s1 = vertexScalars[fVerts[0].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[3].getId(), fVerts[0].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 8
			if (MS_case == 8)
			{
				zVector v0 = fVerts[2].getPosition();
				float s0 = vertexScalars[fVerts[2].getId()];
				zVector v1 = fVerts[3].getPosition();
				float s1 = vertexScalars[fVerts[3].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[2].getId(), fVerts[3].getId(), he2);

				v0 = fVerts[0].getPosition();
				s0 = vertexScalars[fVerts[0].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[0].getId(), fVerts[3].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

			}

			// CASE 9
			if (MS_case == 9)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];
				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[1].getId(), fVerts[0].getId(), he1);

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];
				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[2].getId(), fVerts[3].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

			}

			// CASE 10
			if (MS_case == 10)
			{
				//printf("\n case 10");

				// tri 1
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[0].getId(), fVerts[1].getId(), he2);

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];
				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[0].getId(), fVerts[3].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

				// tri 2

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];
				v1 = fVerts[1].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];

				zVector pos3 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he3;
				halfEdgeExists(fVerts[2].getId(), fVerts[1].getId(), he3);

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];
				zVector pos4 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he4;
				halfEdgeExists(fVerts[2].getId(), fVerts[3].getId(), he4);

				newPositions2.push_back(pos3);
				newPositions2.push_back(pos4);

				newColors2.push_back(he3.getColor());
				newColors2.push_back(he4.getColor());

			}

			// CASE 11
			if (MS_case == 11)
			{
				zVector v0 = fVerts[2].getPosition();
				float s0 = vertexScalars[fVerts[2].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[2].getId(), fVerts[1].getId(), he1);

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];
				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[2].getId(), fVerts[3].getId(), he2);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);


				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

			}

			// CASE 12
			if (MS_case == 12)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];
				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[1].getId(), fVerts[2].getId(), he2);

				v0 = fVerts[0].getPosition();
				s0 = vertexScalars[fVerts[0].getId()];
				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[0].getId(), fVerts[3].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());

			}

			// CASE 13
			if (MS_case == 13)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];
				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[1].getId(), fVerts[0].getId(), he1);

				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[1].getId(), fVerts[2].getId(), he2);


				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 14
			if (MS_case == 14)
			{
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he2;
				halfEdgeExists(fVerts[0].getId(), fVerts[1].getId(), he2);

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));

				zItMeshHalfEdge he1;
				halfEdgeExists(fVerts[0].getId(), fVerts[3].getId(), he1);

				newPositions.push_back(pos1);
				newPositions.push_back(pos2);

				newColors.push_back(he1.getColor());
				newColors.push_back(he2.getColor());
			}

			// CASE 15
			if (MS_case == 15)
			{
				// No Veritices to be added 

			}


			// check for edge lengths

			if (newPositions.size() == 2)
			{
				zPoint p0 = coreUtils.factorise(newPositions[0], precision);
				zPoint p1 = coreUtils.factorise(newPositions[1], precision);

				//zPoint p0 = newPositions[0];
				//zPoint p1 = newPositions[1];

				if (p0.distanceTo(p1) < distTolerance)
					newPositions.clear();
			}



			// compute edge 
			if (newPositions.size() == 2)
			{
				for (int i = 0; i < newPositions.size(); i++)
				{
					zVector p0 = newPositions[i];
					int v0;

					//bool vExists = coreUtils.vertexExists(positionVertex, p0, PRECISION, v0);
					bool vExists = coreUtils.checkRepeatVector(p0, positions, v0, precision);


					if (!vExists)
					{
						v0 = positions.size();
						positions.push_back(p0);
						cVertexColor.push_back(newColors[i]);

						//string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
						//positionVertex[hashKey] = v0;

						//coreUtils.addToPositionMap(positionVertex, p0, v0, PRECISION);
					}

					edgeConnects.push_back(v0);
				}

			}


			// only if there are 2 tris Case : 5,10

			// Edge Length Check


			if (newPositions2.size() == 2)
			{
				zPoint p0 = coreUtils.factorise(newPositions2[0], precision);
				zPoint p1 = coreUtils.factorise(newPositions2[1], precision);

				//zPoint p0 = newPositions[0];
				//zPoint p1 = newPositions[1];

				if (p0.distanceTo(p1) < distTolerance)
					newPositions2.clear();
			}


			// compute edge 
			if (newPositions2.size() == 2)
			{
				for (int i = 0; i < newPositions2.size(); i++)
				{
					zVector p0 = newPositions2[i];
					int v0;

					//bool vExists = coreUtils.vertexExists(positionVertex, p0, PRECISION, v0);
					bool vExists = coreUtils.checkRepeatVector(p0, positions, v0, precision);

					if (!vExists)
					{
						v0 = positions.size();
						positions.push_back(p0);
						cVertexColor.push_back(newColors2[i]);

						//coreUtils.addToPositionMap(positionVertex, p0, v0, PRECISION);
					}

					edgeConnects.push_back(v0);
				}

			}
		}

		
				

	}


	ZSPACE_INLINE void zFnMesh::getIsolinePoly_mixed(zScalarArray& vertexScalars, zItMeshFace& f, zPointArray& positions, zIntArray& polyConnects, zIntArray& polyCounts, unordered_map <string, int>& positionVertex, float& threshold, bool invertMesh)
	{
		vector<zItMeshVertex> fVerts;
		f.getVertices(fVerts);

		// TRIANGLES
		if (fVerts.size() == 3)
		{
			// chk if all the face vertices are below the threshold
			bool vertexBinary[3];
			float averageScalar = 0;

			for (int j = 0; j < fVerts.size(); j++)
			{
				if (vertexScalars[fVerts[j].getId()] < threshold)
				{
					vertexBinary[j] = (invertMesh) ? false : true;
				}
				else vertexBinary[j] = (invertMesh) ? true : false;

				averageScalar += vertexScalars[fVerts[j].getId()];
			}

			averageScalar /= fVerts.size();

			int MS_case = getIsolineCase_triangle(vertexBinary);

			vector<zVector> newPositions;
			vector<zVector> newPositions2;


			// CASE 0
			if (MS_case == 0)
			{
				for (int j = 0; j < fVerts.size(); j++)
				{
					newPositions.push_back(fVerts[j].getPosition());
				}

			}

			// CASE 1 100
			if (MS_case == 1)
			{
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[1].getPosition());
				newPositions.push_back(fVerts[2].getPosition());

				// tri 2

				newPositions2.push_back(pos);
				newPositions2.push_back(fVerts[2].getPosition());

				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions2.push_back(pos);


			}

			// CASE 2 010
			if (MS_case == 2)
			{
				newPositions.push_back(fVerts[0].getPosition());

				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[2].getPosition());

				// tri 2

				newPositions2.push_back(pos);

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos);

				newPositions2.push_back(fVerts[2].getPosition());

			}

			// CASE 3 001
			if (MS_case == 3)
			{
				newPositions.push_back(fVerts[0].getPosition());
				newPositions.push_back(fVerts[1].getPosition());

				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				// tri 2

				newPositions2.push_back(fVerts[0].getPosition());
				newPositions2.push_back(pos);

				v0 = fVerts[0].getPosition();
				s0 = vertexScalars[fVerts[0].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos);


			}

			// CASE 4 110
			if (MS_case == 4)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[2].getPosition());

				v0 = fVerts[0].getPosition();
				s0 = vertexScalars[fVerts[0].getId()];

				pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions.push_back(pos);
			}

			// CASE 5 011
			if (MS_case == 5)
			{
				newPositions.push_back(fVerts[0].getPosition());

				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions.push_back(pos);

				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions.push_back(pos);
			}

			// CASE 6 101
			if (MS_case == 6)
			{
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[1].getPosition());

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];

				pos = (getContourPosition(threshold, v1, v0, s1, s0));
				newPositions.push_back(pos);
			}


			// CASE 7 111
			if (MS_case == 15)
			{
				// No Veritices to be added 

			}


			// check for edge lengths

			bool check = false;
			if (newPositions.size() == 3)
			{
				for (int i = 0; i < newPositions.size(); i++)
				{
					int next = (i + 1) % newPositions.size();

					if (newPositions[i].distanceTo(newPositions[next]) < distanceTolerance)
					{
						//newPositions.erase(newPositions.begin() + i);

						check = true;
					}
				}
			}


			// compute poly 
			if (newPositions.size() == 3 && !check)
			{
				for (int i = 0; i < newPositions.size(); i++)
				{
					zVector p0 = newPositions[i];
					int v0;

					bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);

					if (!vExists)
					{
						v0 = positions.size();
						positions.push_back(p0);

						/*string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
						positionVertex[hashKey] = v0;*/

						coreUtils.addToPositionMap(positionVertex, p0, v0, 3);
					}

					polyConnects.push_back(v0);
				}

				polyCounts.push_back(newPositions.size());
			}


			// only if there are 2 tris Case : 5,10

			// Edge Length Check
			check = false;

			if (newPositions2.size() == 3)
			{
				for (int i = 0; i < newPositions2.size(); i++)
				{
					int next = (i + 1) % newPositions2.size();


					if (newPositions2[i].distanceTo(newPositions2[next]) < distanceTolerance)
					{
						//newPositions2.erase(newPositions.begin() + i);
						check = true;
					}

				}
			}


			// compute poly 
			if (newPositions2.size() == 3 && !check)
			{
				for (int i = 0; i < newPositions2.size(); i++)
				{
					zVector p0 = newPositions2[i];
					int v0;

					bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);

					if (!vExists)
					{
						v0 = positions.size();
						positions.push_back(p0);

						/*string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
						positionVertex[hashKey] = v0;*/

						coreUtils.addToPositionMap(positionVertex, p0, v0, 3);
					}

					polyConnects.push_back(v0);
				}

				polyCounts.push_back(newPositions2.size());
			}
		}

		
		//QUADS
		else if (fVerts.size() == 4)
		{

			// chk if all the face vertices are below the threshold
			bool vertexBinary[4];
			float averageScalar = 0;

			for (int j = 0; j < fVerts.size(); j++)
			{
				if (vertexScalars[fVerts[j].getId()] < threshold)
				{
					vertexBinary[j] = (invertMesh) ? false : true;
				}
				else vertexBinary[j] = (invertMesh) ? true : false;

				averageScalar += vertexScalars[fVerts[j].getId()];
			}

			averageScalar /= fVerts.size();

			int MS_case = getIsolineCase(vertexBinary);

			

			vector<zVector> newPositions;
			vector<zVector> newPositions2;


			// CASE 0
			if (MS_case == 0)
			{
				for (int j = 0; j < fVerts.size(); j++)
				{
					newPositions.push_back(fVerts[j].getPosition());
				}

			}

			// CASE 1
			if (MS_case == 1)
			{
				// tri
				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos0 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos0);

				newPositions.push_back(fVerts[1].getPosition());

				newPositions.push_back(fVerts[2].getPosition());

				// quad
				newPositions2.push_back(pos0);
				newPositions2.push_back(fVerts[2].getPosition());

				newPositions2.push_back(fVerts[3].getPosition());

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos1);


			}

			// CASE 2
			if (MS_case == 2)
			{
				//tri
				newPositions.push_back(fVerts[0].getPosition());

				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];

				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos0 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos0);

				newPositions.push_back(fVerts[3].getPosition());

				//quad

				newPositions2.push_back(pos0);

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos1);

				newPositions2.push_back(fVerts[2].getPosition());

				newPositions2.push_back(fVerts[3].getPosition());

			}

			// CASE 3
			if (MS_case == 3)
			{
				zVector v0 = fVerts[3].getPosition();
				float s0 = vertexScalars[fVerts[3].getId()];

				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];

				v1 = fVerts[1].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[2].getPosition());

				newPositions.push_back(fVerts[3].getPosition());


			}

			// CASE 4
			if (MS_case == 4)
			{
				//tri
				newPositions.push_back(fVerts[0].getPosition());

				newPositions.push_back(fVerts[1].getPosition());

				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos0 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos0);

				// quad
				newPositions2.push_back(fVerts[0].getPosition());

				newPositions2.push_back(pos0);

				v0 = fVerts[3].getPosition();
				s0 = vertexScalars[fVerts[3].getId()];

				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos1);

				newPositions2.push_back(fVerts[3].getPosition());

			}

			// CASE 5
			if (MS_case == 5)
			{
				// SADDLE CASE 

				//printf("\n case 5");

				int SaddleCase = 1/*(averageScalar < threshold) ? 0 : 1*/;
				if (invertMesh) SaddleCase = 0/*(averageScalar < threshold) ? 1 : 0*/;

				if (SaddleCase == 0)
				{
					// quad 1

					zVector v0 = fVerts[1].getPosition();
					float s0 = vertexScalars[fVerts[1].getId()];

					zVector v1 = fVerts[0].getPosition();
					float s1 = vertexScalars[fVerts[0].getId()];

					zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos1);

					newPositions.push_back(fVerts[1].getPosition());

					v1 = fVerts[2].getPosition();
					s1 = vertexScalars[fVerts[2].getId()];

					zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos2);

					v0 = fVerts[3].getPosition();
					s0 = vertexScalars[fVerts[3].getId()];

					zVector pos3 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos3);

					// quad 2
					newPositions2.push_back(pos1);
					newPositions2.push_back(pos3);

					newPositions2.push_back(fVerts[3].getPosition());

					v1 = fVerts[0].getPosition();
					s1 = vertexScalars[fVerts[0].getId()];

					zVector pos4 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions2.push_back(pos4);

				}
				if (SaddleCase == 1)
				{
					// tri 1
					zVector v0 = fVerts[1].getPosition();
					float s0 = vertexScalars[fVerts[1].getId()];

					zVector v1 = fVerts[0].getPosition();
					float s1 = vertexScalars[fVerts[0].getId()];

					zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos);

					newPositions.push_back(fVerts[1].getPosition());


					v1 = fVerts[2].getPosition();
					s1 = vertexScalars[fVerts[2].getId()];
					pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos);

					// tri 2
					v0 = fVerts[3].getPosition();
					s0 = vertexScalars[fVerts[3].getId()];
					pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions2.push_back(pos);

					newPositions2.push_back(fVerts[3].getPosition());

					v1 = fVerts[0].getPosition();
					s1 = vertexScalars[fVerts[0].getId()];
					pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions2.push_back(pos);
				}


			}

			// CASE 6
			if (MS_case == 6)
			{
				newPositions.push_back(fVerts[0].getPosition());

				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v0 = fVerts[3].getPosition();
				s0 = fVerts[3].getColor().r;
				s0 = vertexScalars[fVerts[3].getId()];
				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[3].getPosition());

			}


			// CASE 7
			if (MS_case == 7)
			{
				// tri
				zVector v0 = fVerts[3].getPosition();
				float s0 = vertexScalars[fVerts[3].getId()];
				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[3].getPosition());

				v1 = fVerts[0].getPosition();
				s1 = vertexScalars[fVerts[0].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

			}

			// CASE 8
			if (MS_case == 8)
			{
				// tri 1
				newPositions.push_back(fVerts[0].getPosition());

				newPositions.push_back(fVerts[1].getPosition());

				//quad
				newPositions2.push_back(fVerts[1].getPosition());

				newPositions2.push_back(fVerts[2].getPosition());

				zVector v0 = fVerts[2].getPosition();
				float s0 = vertexScalars[fVerts[2].getId()];
				zVector v1 = fVerts[3].getPosition();
				float s1 = vertexScalars[fVerts[3].getId()];

				zVector pos0 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos0);

				v0 = fVerts[0].getPosition();
				s0 = vertexScalars[fVerts[0].getId()];
				zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos1);

				// tri1
				newPositions.push_back(pos1);
			}

			// CASE 9
			if (MS_case == 9)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];
				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[1].getPosition());

				newPositions.push_back(fVerts[2].getPosition());

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];
				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

			}

			// CASE 10
			if (MS_case == 10)
			{
				//printf("\n case 10");

				int SaddleCase = 1 /*(averageScalar < threshold) ? 0 : 1*/;
				if (invertMesh) SaddleCase = 0 /*(averageScalar < threshold) ? 1 : 0*/;

				if (SaddleCase == 0)
				{
					// quad1
					newPositions.push_back(fVerts[0].getPosition());

					zVector v0 = fVerts[0].getPosition();
					float s0 = vertexScalars[fVerts[0].getId()];
					zVector v1 = fVerts[1].getPosition();
					float s1 = vertexScalars[fVerts[1].getId()];

					zVector pos1 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos1);

					// quad 2
					newPositions2.push_back(pos1);

					v0 = fVerts[2].getPosition();
					s0 = vertexScalars[fVerts[2].getId()];
					v1 = fVerts[1].getPosition();
					s1 = vertexScalars[fVerts[1].getId()];

					zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions2.push_back(pos2);

					newPositions2.push_back(fVerts[2].getPosition());

					v1 = fVerts[3].getPosition();
					s1 = vertexScalars[fVerts[3].getId()];

					zVector pos3 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions2.push_back(pos3);

					// quad1
					newPositions.push_back(pos3);

					v0 = fVerts[0].getPosition();
					s0 = vertexScalars[fVerts[0].getId()];
					v1 = fVerts[3].getPosition();
					s1 = vertexScalars[fVerts[3].getId()];

					zVector pos4 = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos4);
				}

				if (SaddleCase == 1)
				{
					// tri 1

					newPositions.push_back(fVerts[0].getPosition());

					zVector v0 = fVerts[0].getPosition();
					float s0 = vertexScalars[fVerts[0].getId()];
					zVector v1 = fVerts[1].getPosition();
					float s1 = vertexScalars[fVerts[1].getId()];

					zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos);

					v1 = fVerts[3].getPosition();
					s1 = vertexScalars[fVerts[1].getId()];
					pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions.push_back(pos);

					// tri 2

					v0 = fVerts[2].getPosition();
					s0 = vertexScalars[fVerts[2].getId()];
					v1 = fVerts[1].getPosition();
					s1 = vertexScalars[fVerts[1].getId()];

					pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions2.push_back(pos);

					newPositions2.push_back(fVerts[2].getPosition());

					v1 = fVerts[3].getPosition();
					s1 = vertexScalars[fVerts[3].getId()];
					pos = (getContourPosition(threshold, v0, v1, s0, s1));
					newPositions2.push_back(pos);
				}


			}

			// CASE 11
			if (MS_case == 11)
			{
				zVector v0 = fVerts[2].getPosition();
				float s0 = vertexScalars[fVerts[2].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[2].getPosition());

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];
				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);



			}

			// CASE 12
			if (MS_case == 12)
			{
				newPositions.push_back(fVerts[0].getPosition());

				newPositions.push_back(fVerts[1].getPosition());

				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];
				zVector v1 = fVerts[2].getPosition();
				float s1 = vertexScalars[fVerts[2].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v0 = fVerts[0].getPosition();
				s0 = vertexScalars[fVerts[0].getId()];
				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

			}

			// CASE 13
			if (MS_case == 13)
			{
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];
				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[1].getPosition());

				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

			}

			// CASE 14
			if (MS_case == 14)
			{
				newPositions.push_back(fVerts[0].getPosition());

				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

			}

			// CASE 15
			if (MS_case == 15)
			{
				// No Veritices to be added 

			}


			// check for edge lengths


			if (newPositions.size() >= 3)
			{
				for (int i = 0; i < newPositions.size(); i++)
				{
					int next = (i + 1) % newPositions.size();

					if (newPositions[i].distanceTo(newPositions[next]) < distanceTolerance)
					{
						newPositions.erase(newPositions.begin() + i);
					}
				}
			}


			// compute poly 
			if (newPositions.size() >= 3)
			{
				for (int i = 0; i < newPositions.size(); i++)
				{
					zVector p0 = newPositions[i];
					int v0;

					bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);

					if (!vExists)
					{
						v0 = positions.size();
						positions.push_back(p0);

						/*string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
						positionVertex[hashKey] = v0;*/

						coreUtils.addToPositionMap(positionVertex, p0, v0, 3);
					}

					polyConnects.push_back(v0);
				}

				polyCounts.push_back(newPositions.size());
			}


			// only if there are 2 tris Case : 5,10

			// Edge Length Check

			if (newPositions2.size() >= 3)
			{
				for (int i = 0; i < newPositions2.size(); i++)
				{
					int next = (i + 1) % newPositions2.size();


					if (newPositions2[i].distanceTo(newPositions2[next]) < distanceTolerance)
					{
						newPositions2.erase(newPositions.begin() + i);
					}

				}
			}


			// compute poly 
			if (newPositions2.size() >= 3)
			{
				for (int i = 0; i < newPositions2.size(); i++)
				{
					zVector p0 = newPositions2[i];
					int v0;

					bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);

					if (!vExists)
					{
						v0 = positions.size();
						positions.push_back(p0);

						/*string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
						positionVertex[hashKey] = v0;*/

						coreUtils.addToPositionMap(positionVertex, p0, v0, 3);
					}

					polyConnects.push_back(v0);
				}

				polyCounts.push_back(newPositions2.size());
			}
		}

		else throw std::invalid_argument(" error: doesnt work with ngon meshes "); ;



	}

	ZSPACE_INLINE void zFnMesh::getIsolinePoly(zScalarArray &vertexScalars , zItMeshFace& f, zPointArray& positions, zIntArray& polyConnects, zIntArray& polyCounts, unordered_map <string, int>& positionVertex, float& threshold, bool invertMesh)
	{
		vector<zItMeshVertex> fVerts;
		f.getVertices(fVerts);

		if (fVerts.size() != 4) return;

		// chk if all the face vertices are below the threshold
		bool vertexBinary[4];
		float averageScalar = 0;

		for (int j = 0; j < fVerts.size(); j++)
		{
			if (vertexScalars[fVerts[j].getId()] < threshold)
			{
				vertexBinary[j] = (invertMesh) ? false : true;
			}
			else vertexBinary[j] = (invertMesh) ? true : false;

			averageScalar += vertexScalars[fVerts[j].getId()];
		}

		averageScalar /= fVerts.size();

		int MS_case = getIsolineCase(vertexBinary);

		vector<zVector> newPositions;
		vector<zVector> newPositions2;


		// CASE 0
		if (MS_case == 0)
		{
			for (int j = 0; j < fVerts.size(); j++)
			{
				newPositions.push_back(fVerts[j].getPosition());
			}

		}

		// CASE 1
		if (MS_case == 1)
		{
			zVector v0 = fVerts[0].getPosition();
			float s0 = vertexScalars[fVerts[0].getId()];

			zVector v1 = fVerts[1].getPosition();
			float s1 = vertexScalars[fVerts[1].getId()];

			zVector pos = (getContourPosition(threshold, v1, v0, s1, s0));
			newPositions.push_back(pos);		

			newPositions.push_back(fVerts[1].getPosition());

			newPositions.push_back(fVerts[2].getPosition());

			newPositions.push_back(fVerts[3].getPosition());

			v1 = fVerts[3].getPosition();
			s1 = vertexScalars[fVerts[3].getId()];

			pos = (getContourPosition(threshold, v1, v0, s1, s0));
			newPositions.push_back(pos);

			
		}

		// CASE 2
		if (MS_case == 2)
		{
			newPositions.push_back(fVerts[0].getPosition());

			zVector v0 = fVerts[0].getPosition();
			float s0 = vertexScalars[fVerts[0].getId()];

			zVector v1 = fVerts[1].getPosition();
			float s1 = vertexScalars[fVerts[1].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			v0 = fVerts[2].getPosition();
			s0 = vertexScalars[fVerts[2].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[2].getPosition());

			newPositions.push_back(fVerts[3].getPosition());

		}

		// CASE 3
		if (MS_case == 3)
		{
			zVector v0 = fVerts[3].getPosition();
			float s0 = vertexScalars[fVerts[3].getId()];

			zVector v1 = fVerts[0].getPosition();
			float s1 = vertexScalars[fVerts[0].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			v0 = fVerts[2].getPosition();
			s0 = vertexScalars[fVerts[2].getId()];

			v1 = fVerts[1].getPosition();
			s1 = vertexScalars[fVerts[1].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[2].getPosition());

			newPositions.push_back(fVerts[3].getPosition());


		}

		// CASE 4
		if (MS_case == 4)
		{
			newPositions.push_back(fVerts[0].getPosition());

			newPositions.push_back(fVerts[1].getPosition());

			zVector v0 = fVerts[1].getPosition();
			float s0 = vertexScalars[fVerts[1].getId()];

			zVector v1 = fVerts[2].getPosition();
			float s1 = vertexScalars[fVerts[2].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			v0 = fVerts[3].getPosition();
			s0 = vertexScalars[fVerts[3].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[3].getPosition());

		}

		// CASE 5
		if (MS_case == 5)
		{
			// SADDLE CASE 

			//printf("\n case 5");

			int SaddleCase = 1/*(averageScalar < threshold) ? 0 : 1*/;
			if (invertMesh) SaddleCase = 0/*(averageScalar < threshold) ? 1 : 0*/;

			if (SaddleCase == 0)
			{
				// hex

				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[1].getPosition());

				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v0 = fVerts[3].getPosition();
				s0 = vertexScalars[fVerts[3].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[3].getPosition());

				v1 = fVerts[0].getPosition();
				s1 = vertexScalars[fVerts[0].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

			}
			if (SaddleCase == 1)
			{
				// tri 1
				zVector v0 = fVerts[1].getPosition();
				float s0 = vertexScalars[fVerts[1].getId()];

				zVector v1 = fVerts[0].getPosition();
				float s1 = vertexScalars[fVerts[0].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[1].getPosition());


				v1 = fVerts[2].getPosition();
				s1 = vertexScalars[fVerts[2].getId()];
				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				// tri 2
				v0 = fVerts[3].getPosition();
				s0 = vertexScalars[fVerts[3].getId()];
				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos);

				newPositions2.push_back(fVerts[3].getPosition());

				v1 = fVerts[0].getPosition();
				s1 = vertexScalars[fVerts[0].getId()];
				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos);
			}


		}

		// CASE 6
		if (MS_case == 6)
		{
			newPositions.push_back(fVerts[0].getPosition());

			zVector v0 = fVerts[0].getPosition();
			float s0 = vertexScalars[fVerts[0].getId()];
			zVector v1 = fVerts[1].getPosition();
			float s1 = vertexScalars[fVerts[1].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			v0 = fVerts[3].getPosition();
			s0 = fVerts[3].getColor().r;
			s0 = vertexScalars[fVerts[3].getId()];
			v1 = fVerts[2].getPosition();
			s1 = vertexScalars[fVerts[2].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[3].getPosition());

		}


		// CASE 7
		if (MS_case == 7)
		{
			zVector v0 = fVerts[3].getPosition();
			float s0 = vertexScalars[fVerts[3].getId()];
			zVector v1 = fVerts[2].getPosition();
			float s1 = vertexScalars[fVerts[2].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[3].getPosition());

			v1 = fVerts[0].getPosition();
			s1 = vertexScalars[fVerts[0].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

		}

		// CASE 8
		if (MS_case == 8)
		{
			newPositions.push_back(fVerts[0].getPosition());

			newPositions.push_back(fVerts[1].getPosition());

			newPositions.push_back(fVerts[2].getPosition());

			zVector v0 = fVerts[2].getPosition();
			float s0 = vertexScalars[fVerts[2].getId()];
			zVector v1 = fVerts[3].getPosition();
			float s1 = vertexScalars[fVerts[3].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			v0 = fVerts[0].getPosition();
			s0 = vertexScalars[fVerts[0].getId()];
			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

		}

		// CASE 9
		if (MS_case == 9)
		{
			zVector v0 = fVerts[1].getPosition();
			float s0 = vertexScalars[fVerts[1].getId()];
			zVector v1 = fVerts[0].getPosition();
			float s1 = vertexScalars[fVerts[0].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[1].getPosition());

			newPositions.push_back(fVerts[2].getPosition());

			v0 = fVerts[2].getPosition();
			s0 = vertexScalars[fVerts[2].getId()];
			v1 = fVerts[3].getPosition();
			s1 = vertexScalars[fVerts[3].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

		}

		// CASE 10
		if (MS_case == 10)
		{
			//printf("\n case 10");

			int SaddleCase = 1 /*(averageScalar < threshold) ? 0 : 1*/;
			if (invertMesh) SaddleCase = 0 /*(averageScalar < threshold) ? 1 : 0*/;

			if (SaddleCase == 0)
			{
				// hex
				newPositions.push_back(fVerts[0].getPosition());

				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];
				v1 = fVerts[1].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				newPositions.push_back(fVerts[2].getPosition());

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v0 = fVerts[0].getPosition();
				s0 = vertexScalars[fVerts[0].getId()];
				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);
			}

			if (SaddleCase == 1)
			{
				// tri 1

				newPositions.push_back(fVerts[0].getPosition());

				zVector v0 = fVerts[0].getPosition();
				float s0 = vertexScalars[fVerts[0].getId()];
				zVector v1 = fVerts[1].getPosition();
				float s1 = vertexScalars[fVerts[1].getId()];

				zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];
				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions.push_back(pos);

				// tri 2

				v0 = fVerts[2].getPosition();
				s0 = vertexScalars[fVerts[2].getId()];
				v1 = fVerts[1].getPosition();
				s1 = vertexScalars[fVerts[1].getId()];

				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos);

				newPositions2.push_back(fVerts[2].getPosition());

				v1 = fVerts[3].getPosition();
				s1 = vertexScalars[fVerts[3].getId()];
				pos = (getContourPosition(threshold, v0, v1, s0, s1));
				newPositions2.push_back(pos);
			}


		}

		// CASE 11
		if (MS_case == 11)
		{
			zVector v0 = fVerts[2].getPosition();
			float s0 = vertexScalars[fVerts[2].getId()];
			zVector v1 = fVerts[1].getPosition();
			float s1 = vertexScalars[fVerts[1].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[2].getPosition());

			v1 = fVerts[3].getPosition();
			s1 = vertexScalars[fVerts[3].getId()];
			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);



		}

		// CASE 12
		if (MS_case == 12)
		{
			newPositions.push_back(fVerts[0].getPosition());

			newPositions.push_back(fVerts[1].getPosition());

			zVector v0 = fVerts[1].getPosition();
			float s0 = vertexScalars[fVerts[1].getId()];
			zVector v1 = fVerts[2].getPosition();
			float s1 = vertexScalars[fVerts[2].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			v0 = fVerts[0].getPosition();
			s0 = vertexScalars[fVerts[0].getId()];
			v1 = fVerts[3].getPosition();
			s1 = vertexScalars[fVerts[3].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

		}

		// CASE 13
		if (MS_case == 13)
		{
			zVector v0 = fVerts[1].getPosition();
			float s0 = vertexScalars[fVerts[1].getId()];
			zVector v1 = fVerts[0].getPosition();
			float s1 = vertexScalars[fVerts[0].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			newPositions.push_back(fVerts[1].getPosition());

			v1 = fVerts[2].getPosition();
			s1 = vertexScalars[fVerts[2].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

		}

		// CASE 14
		if (MS_case == 14)
		{
			newPositions.push_back(fVerts[0].getPosition());

			zVector v0 = fVerts[0].getPosition();
			float s0 = vertexScalars[fVerts[0].getId()];
			zVector v1 = fVerts[1].getPosition();
			float s1 = vertexScalars[fVerts[1].getId()];

			zVector pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

			v1 = fVerts[3].getPosition();
			s1 = vertexScalars[fVerts[3].getId()];

			pos = (getContourPosition(threshold, v0, v1, s0, s1));
			newPositions.push_back(pos);

		}

		// CASE 15
		if (MS_case == 15)
		{
			// No Veritices to be added 

		}


		// check for edge lengths


		if (newPositions.size() >= 3)
		{
			for (int i = 0; i < newPositions.size(); i++)
			{
				int next = (i + 1) % newPositions.size();

				if (newPositions[i].distanceTo(newPositions[next]) < distanceTolerance)
				{
					newPositions.erase(newPositions.begin() + i);
				}
			}
		}


		// compute poly 
		if (newPositions.size() >= 3)
		{
			for (int i = 0; i < newPositions.size(); i++)
			{
				zVector p0 = newPositions[i];
				int v0;

				bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);
				//if (vExists) printf("\n working");

				if (!vExists)
				{
					v0 = positions.size();
					positions.push_back(p0);

					//string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
					//positionVertex[hashKey] = v0;

					coreUtils.addToPositionMap(positionVertex, p0, v0,3);
				}

				polyConnects.push_back(v0);
			}

			polyCounts.push_back(newPositions.size());
		}


		// only if there are 2 tris Case : 5,10

		// Edge Length Check

		if (newPositions2.size() >= 3)
		{
			for (int i = 0; i < newPositions2.size(); i++)
			{
				int next = (i + 1) % newPositions2.size();


				if (newPositions2[i].distanceTo(newPositions2[next]) < distanceTolerance)
				{
					newPositions2.erase(newPositions.begin() + i);
				}

			}
		}


		// compute poly 
		if (newPositions2.size() >= 3)
		{
			for (int i = 0; i < newPositions2.size(); i++)
			{
				zVector p0 = newPositions2[i];
				int v0;

				bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);

				if (!vExists)
				{
					v0 = positions.size();
					positions.push_back(p0);

					//string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
					//positionVertex[hashKey] = v0;
					coreUtils.addToPositionMap(positionVertex, p0, v0, 3);
				}

				polyConnects.push_back(v0);
			}

			polyCounts.push_back(newPositions2.size());
		}



	}


	ZSPACE_INLINE void zFnMesh::getIsobandPoly(zScalarArray& vertexScalars, zItMeshFace & f, zPointArray & positions, zIntArray & polyConnects, zIntArray & polyCounts, unordered_map<string, int>& positionVertex, float& thresholdLow, float& thresholdHigh)

	{
		vector<zItMeshVertex> fVerts;
		f.getVertices(fVerts);

		//printf("\n fVs: %i ", fVerts.size());

		if (fVerts.size() != 4) return;

		// chk if all the face vertices are below the threshold
		int vertexTernary[4];
		double averageScalar = 0;

		for (int j = 0; j < fVerts.size(); j++)
		{
			if (vertexScalars[fVerts[j].getId()] <= thresholdLow)
			{
				vertexTernary[j] = 0;
			}

			else if (vertexScalars[fVerts[j].getId()] >= thresholdHigh)
			{
				vertexTernary[j] = 2;
			}
			else vertexTernary[j] = 1;

			averageScalar += vertexScalars[fVerts[j].getId()];
		}

		averageScalar /= fVerts.size();

		
		int MS_case = getIsobandCase(vertexTernary);

		//if (MS_case != 0 && MS_case != 1) printf("\n f %i | case %i ",f.getId(), MS_case);
				

		vector<zVector> newPositions;
		vector<zVector> newPositions2;

		// No Contours CASE 0 & 1
		if (MS_case == 0 || MS_case == 1)
		{
			// NO Vertices to be added
		}

		// Single Triangle CASE 2 to 9 
		if (MS_case >= 2 && MS_case <= 9)
		{
			int startID = -1;
			float threshold = thresholdHigh;

			if (MS_case == 2 || MS_case == 6)startID = 0;
			if (MS_case == 3 || MS_case == 7)startID = 1;
			if (MS_case == 4 || MS_case == 8)startID = 2;
			if (MS_case == 5 || MS_case == 9)startID = 3;

			if (MS_case > 5) threshold = thresholdLow;

			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];

			zVector v1 = fVerts[prevID].getPosition();
			float s1 = vertexScalars[fVerts[prevID].getId()];


			zVector pos0 = (getContourPosition(threshold, v0, v1, s0, s1));

			zVector pos1 = fVerts[startID].getPosition();


			v1 = fVerts[nextID].getPosition();
			s1 = vertexScalars[fVerts[nextID].getId()];

			zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));


			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);

		}


		// Single Trapezoid CASE 10 to 17

		if (MS_case >= 10 && MS_case <= 17)
		{
			int startID = -1;

			float threshold0 = thresholdLow;
			float threshold1 = thresholdHigh;

			if (MS_case == 10 || MS_case == 14) startID = 0;
			if (MS_case == 11 || MS_case == 15) startID = 1;
			if (MS_case == 12 || MS_case == 16) startID = 2;
			if (MS_case == 13 || MS_case == 17) startID = 3;

			if (MS_case > 13)
			{
				threshold0 = thresholdHigh;
				threshold1 = thresholdLow;
			}

			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];

			zVector v1 = fVerts[nextID].getPosition();
			float s1 = vertexScalars[fVerts[nextID].getId()];


			zVector pos0 = (getContourPosition(threshold0, v0, v1, s0, s1));

			zVector pos1 = (getContourPosition(threshold1, v0, v1, s0, s1));

			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];


			zVector pos2 = (getContourPosition(threshold1, v0, v1, s0, s1));

			zVector pos3 = (getContourPosition(threshold0, v0, v1, s0, s1));


			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);

		}


		// Single Rectangle CASE 18 to 29
		if (MS_case >= 18 && MS_case <= 25)
		{
			int startID = -1;
			float threshold = thresholdLow;
			if (MS_case > 21) threshold = thresholdHigh;

			if (MS_case == 18 || MS_case == 22) startID = 0;
			if (MS_case == 19 || MS_case == 23) startID = 1;
			if (MS_case == 20 || MS_case == 24) startID = 2;
			if (MS_case == 21 || MS_case == 25) startID = 3;


			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();

			int next_nextID = (nextID + 1) % fVerts.size();

			zVector pos0 = fVerts[startID].getPosition();
			zVector pos1 = fVerts[nextID].getPosition();

			zVector v0 = fVerts[nextID].getPosition();
			float s0 = vertexScalars[fVerts[nextID].getId()];

			zVector v1 = fVerts[next_nextID].getPosition();
			float s1 = vertexScalars[fVerts[next_nextID].getId()];

			zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

			v0 = fVerts[startID].getPosition();
			s0 = vertexScalars[fVerts[startID].getId()];
			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos3 = (getContourPosition(threshold, v0, v1, s0, s1));

			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);


		}

		// Single Rectangle CASE 26 to 29
		if (MS_case >= 26 && MS_case <= 29)
		{
			int startID = -1;

			if (MS_case == 26) startID = 0;
			if (MS_case == 27) startID = 1;
			if (MS_case == 28) startID = 2;
			if (MS_case == 29) startID = 3;


			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];

			zVector v1 = fVerts[prevID].getPosition();
			float s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos0 = (getContourPosition(thresholdLow, v0, v1, s0, s1));
			zVector pos3 = (getContourPosition(thresholdHigh, v0, v1, s0, s1));

			v0 = fVerts[nextID].getPosition();
			s0 = vertexScalars[fVerts[nextID].getId()];
			v1 = fVerts[next_nextID].getPosition();
			s1 = vertexScalars[fVerts[next_nextID].getId()];


			zVector pos1 = (getContourPosition(thresholdLow, v0, v1, s0, s1));
			zVector pos2 = (getContourPosition(thresholdHigh, v0, v1, s0, s1));

			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);
		}

		// Single Square CASE 30
		if (MS_case == 30)
		{
			for (int j = 0; j < fVerts.size(); j++)
				newPositions.push_back(fVerts[j].getPosition());
		}

		// Single Pentagon CASE 31 to 38
		if (MS_case >= 31 && MS_case <= 38)
		{
			int startID = -1;

			float threshold = thresholdHigh;
			if (MS_case > 34)threshold = thresholdLow;

			if (MS_case == 31 || MS_case == 35) startID = 0;
			if (MS_case == 32 || MS_case == 36) startID = 1;
			if (MS_case == 33 || MS_case == 37) startID = 2;
			if (MS_case == 34 || MS_case == 38) startID = 3;


			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector pos0 = fVerts[startID].getPosition();
			zVector pos1 = fVerts[nextID].getPosition();

			zVector v0 = fVerts[nextID].getPosition();
			float s0 = vertexScalars[fVerts[nextID].getId()];

			zVector v1 = fVerts[next_nextID].getPosition();
			float s1 = vertexScalars[fVerts[next_nextID].getId()];


      zVector pos2 = getContourPosition(threshold, v0, v1, s0, s1);


			v0 = fVerts[prevID].getPosition();
			s0 = vertexScalars[fVerts[prevID].getId()];

			zVector pos3 = (getContourPosition(threshold, v0, v1, s0, s1));

			zVector pos4 = fVerts[prevID].getPosition();;

			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);
			newPositions.push_back(pos4);

		}

		// Single Pentagon CASE 39 to 46
		if (MS_case >= 39 && MS_case <= 46)
		{
			int startID = -1;

			float threshold0 = thresholdLow;
			float threshold1 = thresholdHigh;

			if (MS_case > 42)
			{
				threshold0 = thresholdHigh;
				threshold1 = thresholdLow;

			}

			if (MS_case == 39 || MS_case == 43) startID = 3;
			if (MS_case == 40 || MS_case == 44) startID = 2;
			if (MS_case == 41 || MS_case == 45) startID = 1;
			if (MS_case == 42 || MS_case == 46) startID = 0;


			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector pos0 = fVerts[startID].getPosition();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];

			zVector v1 = fVerts[nextID].getPosition();
			float s1 = vertexScalars[fVerts[nextID].getId()];

			zVector pos1 = getContourPosition(threshold0, v0, v1, s0, s1);

			v0 = fVerts[next_nextID].getPosition();
			s0 = vertexScalars[fVerts[next_nextID].getId()];
			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos2 = (getContourPosition(threshold0, v0, v1, s0, s1));
			zVector pos3 = (getContourPosition(threshold1, v0, v1, s0, s1));

			v0 = fVerts[startID].getPosition();
			s0 = vertexScalars[fVerts[startID].getId()];

			zVector pos4 = (getContourPosition(threshold1, v0, v1, s0, s1));

			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);
			newPositions.push_back(pos4);

		}

		// Single Pentagon CASE 47 to54
		if (MS_case >= 47 && MS_case <= 54)
		{
			int startID = -1;

			float threshold0 = thresholdLow;
			float threshold1 = thresholdHigh;

			if (MS_case > 50)
			{
				threshold0 = thresholdHigh;
				threshold1 = thresholdLow;

			}

			if (MS_case == 47 || MS_case == 51) startID = 3;
			if (MS_case == 48 || MS_case == 52) startID = 2;
			if (MS_case == 49 || MS_case == 53) startID = 1;
			if (MS_case == 50 || MS_case == 54) startID = 0;


			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector pos0 = fVerts[startID].getPosition();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];
			zVector v1 = fVerts[nextID].getPosition();
			float s1 = vertexScalars[fVerts[nextID].getId()];

			zVector pos1 = getContourPosition(threshold1, v0, v1, s0, s1);

			v0 = fVerts[nextID].getPosition();
			s0 = vertexScalars[fVerts[nextID].getId()];
			v1 = fVerts[next_nextID].getPosition();
			s1 = vertexScalars[fVerts[next_nextID].getId()];

			zVector pos2 = getContourPosition(threshold1, v0, v1, s0, s1);
			zVector pos3 = getContourPosition(threshold0, v0, v1, s0, s1);

			v0 = fVerts[startID].getPosition();
			s0 = vertexScalars[fVerts[startID].getId()];
			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos4 = getContourPosition(threshold0, v0, v1, s0, s1);

			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);
			newPositions.push_back(pos4);
		
		}

		// Single Hexagon CASE 55 to 62
		if (MS_case >= 55 && MS_case <= 62)
		{
			int startID = -1;

			float threshold0 = thresholdLow;
			float threshold1 = thresholdHigh;

			if (MS_case > 58)
			{
				threshold0 = thresholdHigh;
				threshold1 = thresholdLow;

			}

			if (MS_case == 55 || MS_case == 59) startID = 0;
			if (MS_case == 56 || MS_case == 60) startID = 1;
			if (MS_case == 57 || MS_case == 61) startID = 2;
			if (MS_case == 58 || MS_case == 62) startID = 3;


			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector pos0 = fVerts[startID].getPosition();
			zVector pos1 = fVerts[nextID].getPosition();

			zVector v0 = fVerts[nextID].getPosition();
			float s0 = vertexScalars[fVerts[nextID].getId()];
			zVector v1 = fVerts[next_nextID].getPosition();
			float s1 = vertexScalars[fVerts[next_nextID].getId()];

			zVector pos2 = getContourPosition(threshold1, v0, v1, s0, s1);

			v0 = fVerts[prevID].getPosition();
			s0 = vertexScalars[fVerts[prevID].getId()];

			zVector pos3 = (getContourPosition(threshold1, v0, v1, s0, s1));
			zVector pos4 = (getContourPosition(threshold0, v0, v1, s0, s1));

			v1 = fVerts[startID].getPosition();
			s1 = vertexScalars[fVerts[startID].getId()];

			zVector pos5 = getContourPosition(threshold0, v0, v1, s0, s1);

			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);
			newPositions.push_back(pos4);
			newPositions.push_back(pos5);
		}

		// Single Hexagon CASE 63 to 66
		if (MS_case >= 63 && MS_case <= 66)
		{
			int startID = -1;


			float threshold0 = thresholdLow;
			float threshold1 = thresholdHigh;

			if (MS_case % 2 == 0)
			{
				threshold0 = thresholdHigh;
				threshold1 = thresholdLow;
			}

			if (MS_case == 63 || MS_case == 64) startID = 0;
			if (MS_case == 65 || MS_case == 66) startID = 1;



			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector pos0 = fVerts[startID].getPosition();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];
			zVector v1 = fVerts[nextID].getPosition();
			float s1 = vertexScalars[fVerts[nextID].getId()];

			zVector pos1 = getContourPosition(threshold0, v0, v1, s0, s1);

			v0 = fVerts[next_nextID].getPosition();
			s0 = vertexScalars[fVerts[next_nextID].getId()];

			zVector pos2 = (getContourPosition(threshold0, v0, v1, s0, s1));

			zVector pos3 = fVerts[next_nextID].getPosition();

			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos4 = (getContourPosition(threshold1, v0, v1, s0, s1));

			v0 = fVerts[startID].getPosition();
			s0 = vertexScalars[fVerts[startID].getId()];

			zVector pos5 = getContourPosition(threshold1, v0, v1, s0, s1);

			newPositions.push_back(pos0);
			newPositions.push_back(pos1);
			newPositions.push_back(pos2);
			newPositions.push_back(pos3);
			newPositions.push_back(pos4);
			newPositions.push_back(pos5);
		}

		// SADDLE CASE 67 to 68 : 8 Sides
		if (MS_case >= 67 && MS_case <= 68)
		{

			float threshold0 = thresholdLow;
			float threshold1 = thresholdHigh;

			if (MS_case % 2 == 0)
			{
				threshold0 = thresholdHigh;
				threshold1 = thresholdLow;
			}

			int SaddleCase = -1;;

			if (averageScalar < thresholdLow) SaddleCase = 0;
			else if (averageScalar > thresholdHigh) SaddleCase = 2;
			else SaddleCase = 1;

			int startID = 0;

			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];
			zVector v1 = fVerts[nextID].getPosition();
			float s1 = vertexScalars[fVerts[nextID].getId()];

			zVector pos0 = getContourPosition(threshold0, v0, v1, s0, s1);
			zVector pos1 = getContourPosition(threshold1, v0, v1, s0, s1);

			v0 = fVerts[next_nextID].getPosition();
			s0 = vertexScalars[fVerts[next_nextID].getId()];

			zVector pos2 = (getContourPosition(threshold1, v0, v1, s0, s1));
			zVector pos3 = getContourPosition(threshold0, v0, v1, s0, s1);

			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos4 = (getContourPosition(threshold0, v0, v1, s0, s1));
			zVector pos5 = (getContourPosition(threshold1, v0, v1, s0, s1));

			v0 = fVerts[startID].getPosition();
			s0 = vertexScalars[fVerts[startID].getId()];

			zVector pos6 = getContourPosition(threshold1, v0, v1, s0, s1);
			zVector pos7 = (getContourPosition(threshold0, v0, v1, s0, s1));


			if (SaddleCase == 0)
			{
				// quad 1
				newPositions.push_back(pos0);
				newPositions.push_back(pos1);
				newPositions.push_back(pos2);
				newPositions.push_back(pos3);

				// quad 2
				newPositions2.push_back(pos4);
				newPositions2.push_back(pos5);
				newPositions2.push_back(pos6);
				newPositions2.push_back(pos7);
			}


			if (SaddleCase == 2)
			{
				// quad 1
				newPositions.push_back(pos0);
				newPositions.push_back(pos1);
				newPositions.push_back(pos6);
				newPositions.push_back(pos7);

				// quad 2
				newPositions2.push_back(pos2);
				newPositions2.push_back(pos3);
				newPositions2.push_back(pos4);
				newPositions2.push_back(pos5);
			}


			if (SaddleCase == 1)
			{
				// octagon
				newPositions.push_back(pos0);
				newPositions.push_back(pos1);
				newPositions.push_back(pos2);
				newPositions.push_back(pos3);

				newPositions.push_back(pos4);
				newPositions.push_back(pos5);
				newPositions.push_back(pos6);
				newPositions.push_back(pos7);
			}

		}

		// SADDLE CASE 69 to 72 : 6 Sides
		if (MS_case >= 69 && MS_case <= 72)
		{

			float threshold = thresholdLow;

			if (MS_case > 70)
			{
				threshold = thresholdHigh;
			}

			int SaddleCase = -1;;

			if (averageScalar > thresholdLow && averageScalar < thresholdHigh) SaddleCase = 1;
			else SaddleCase = 0;

			int startID = (MS_case % 2 == 0) ? 1 : 0;

			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector pos0 = fVerts[startID].getPosition();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];
			zVector v1 = fVerts[nextID].getPosition();
			float s1 = vertexScalars[fVerts[nextID].getId()];

			zVector pos1 = getContourPosition(threshold, v0, v1, s0, s1);

			v0 = fVerts[next_nextID].getPosition();
			s0 = vertexScalars[fVerts[next_nextID].getId()];

			zVector pos2 = (getContourPosition(threshold, v0, v1, s0, s1));

			zVector pos3 = fVerts[next_nextID].getPosition();

			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos4 = (getContourPosition(threshold, v0, v1, s0, s1));

			v0 = fVerts[startID].getPosition();
			s0 = vertexScalars[fVerts[startID].getId()];

			zVector pos5 = getContourPosition(threshold, v0, v1, s0, s1);


			if (SaddleCase == 0)
			{
				// tri 1
				newPositions.push_back(pos0);
				newPositions.push_back(pos1);
				newPositions.push_back(pos5);

				// tri 2
				newPositions2.push_back(pos2);
				newPositions2.push_back(pos3);
				newPositions2.push_back(pos4);
			}


			if (SaddleCase == 1)
			{
				// hexagon
				newPositions.push_back(pos0);
				newPositions.push_back(pos1);
				newPositions.push_back(pos2);
				newPositions.push_back(pos3);
				newPositions.push_back(pos4);
				newPositions.push_back(pos5);

			}

		}

		// SADDLE CASE 73 to 80 : 7 Sides
		if (MS_case >= 73 && MS_case <= 80)
		{

			float threshold0 = thresholdLow;
			float threshold1 = thresholdHigh;

			if (MS_case > 76)
			{
				threshold0 = thresholdHigh;
				threshold1 = thresholdLow;
			}

			int SaddleCase = -1;;

			if (averageScalar > thresholdLow && averageScalar < thresholdHigh) SaddleCase = 1;
			else SaddleCase = 0;

			int startID = -1;

			if (MS_case == 73 || MS_case == 77) startID = 0;
			if (MS_case == 74 || MS_case == 78) startID = 2;
			if (MS_case == 75 || MS_case == 79) startID = 1;
			if (MS_case == 76 || MS_case == 80) startID = 3;


			int nextID = (startID + 1) % fVerts.size();
			int prevID = (startID - 1 + fVerts.size()) % fVerts.size();
			int next_nextID = (nextID + 1) % fVerts.size();

			zVector v0 = fVerts[startID].getPosition();
			float s0 = vertexScalars[fVerts[startID].getId()];
			zVector v1 = fVerts[nextID].getPosition();
			float s1 = vertexScalars[fVerts[nextID].getId()];

			zVector pos0 = getContourPosition(threshold0, v0, v1, s0, s1);
			zVector pos1 = getContourPosition(threshold1, v0, v1, s0, s1);

			v0 = fVerts[next_nextID].getPosition();
			s0 = vertexScalars[fVerts[next_nextID].getId()];

			zVector pos2 = (getContourPosition(threshold1, v0, v1, s0, s1));

			zVector pos3 = fVerts[next_nextID].getPosition();

			v1 = fVerts[prevID].getPosition();
			s1 = vertexScalars[fVerts[prevID].getId()];

			zVector pos4 = (getContourPosition(threshold1, v0, v1, s0, s1));

			v0 = fVerts[startID].getPosition();
			s0 = vertexScalars[fVerts[startID].getId()];

			zVector pos5 = getContourPosition(threshold1, v0, v1, s0, s1);
			zVector pos6 = getContourPosition(threshold0, v0, v1, s0, s1);

			if (SaddleCase == 0)
			{
				// quad 1
				newPositions.push_back(pos0);
				newPositions.push_back(pos1);
				newPositions.push_back(pos5);
				newPositions.push_back(pos6);

				// tri 2
				newPositions2.push_back(pos2);
				newPositions2.push_back(pos3);
				newPositions2.push_back(pos4);
			}


			if (SaddleCase == 1)
			{
				// heptagon
				newPositions.push_back(pos0);
				newPositions.push_back(pos1);
				newPositions.push_back(pos2);
				newPositions.push_back(pos3);
				newPositions.push_back(pos4);
				newPositions.push_back(pos5);
				newPositions.push_back(pos6);

			}

		}

		// check for edge lengths

		if (newPositions.size() >= 3)
		{
			for (int i = 0; i < newPositions.size(); i++)
			{
				int next = (i + 1) % newPositions.size();

				if (newPositions[i].distanceTo(newPositions[next]) < distanceTolerance)
				{
					newPositions.erase(newPositions.begin() + i);

				}
			}
		}


		// compute poly 
		if (newPositions.size() >= 3)
		{
			for (int i = 0; i < newPositions.size(); i++)
			{
				zVector p0 = newPositions[i];
				int v0;

				bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);

				if (!vExists)
				{
					v0 = positions.size();
					positions.push_back(p0);

					string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
					positionVertex[hashKey] = v0;
				}

				polyConnects.push_back(v0);
			}

			polyCounts.push_back(newPositions.size());
		}


		// only if there are 2 tris Case : 5,10

		// Edge Length Check


		if (newPositions2.size() >= 3)
		{
			for (int i = 0; i < newPositions2.size(); i++)
			{
				int next = (i + 1) % newPositions2.size();


				if (newPositions2[i].distanceTo(newPositions2[next]) < distanceTolerance)
				{
					newPositions2.erase(newPositions.begin() + i);


				}

			}
		}


		// compute poly 
		if (newPositions2.size() >= 3)
		{
			for (int i = 0; i < newPositions2.size(); i++)
			{
				zVector p0 = newPositions2[i];
				int v0;

				bool vExists = coreUtils.vertexExists(positionVertex, p0, 3, v0);

				if (!vExists)
				{
					v0 = positions.size();
					positions.push_back(p0);

					string hashKey = (to_string(p0.x) + "," + to_string(p0.y) + "," + to_string(p0.z));
					positionVertex[hashKey] = v0;
				}

				polyConnects.push_back(v0);
			}

			polyCounts.push_back(newPositions2.size());
		}

	}
	   
	//---- PRIVATE METHODS
	   
	ZSPACE_INLINE void zFnMesh::setStaticContainers()
	{
		zMeshObjectStorage::get(*meshObj).staticGeometry = true;

		vector<vector<int>> edgeVerts;

		for (zItMeshEdge e(*meshObj); !e.end(); e++)
		{
			vector<int> verts;
			e.getVertices(verts);

			edgeVerts.push_back(verts);
		}

		zMeshObjectStorage::get(*meshObj).setStaticEdgeVertices(edgeVerts);

		vector<vector<int>> faceVerts;

		for (zItMeshFace f(*meshObj); !f.end(); f++)
		{
			vector<int> verts;
			f.getVertices(verts);

			faceVerts.push_back(verts);
		}

		zMeshObjectStorage::get(*meshObj).setStaticFaceVertices(faceVerts);
	}

	//---- PRIVATE DEACTIVATE AND REMOVE METHODS

	ZSPACE_INLINE void zFnMesh::addToHalfEdgesMap(zItMeshHalfEdge &he)
	{
		zMeshObjectStorage::get(*meshObj).addToHalfEdgesMap(he.getStartVertex().getId(), he.getVertex().getId(), he.getId());
	}

	ZSPACE_INLINE void zFnMesh::removeFromHalfEdgesMap(zItMeshHalfEdge &he)
	{
		zMeshObjectStorage::get(*meshObj).removeFromHalfEdgesMap(he.getStartVertex().getId(), he.getVertex().getId());
	}

	ZSPACE_INLINE void zFnMesh::removeInactive(zHEData type)
	{
		//  Vertex		
		if (type == zVertexData)
		{
			bool reindex = false;

			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vHandles.size(); i++)
			{
				bool active = (zMeshObjectStorage::get(*meshObj).vHandles[i].id == -1) ? false : true;

				if (!active)
				{
					reindex = true;
					while (zMeshObjectStorage::get(*meshObj).vHandles[i].id == -1 && i < zMeshObjectStorage::get(*meshObj).vHandles.size())
					{
						zMeshObjectStorage::get(*meshObj).vertexColors.erase(zMeshObjectStorage::get(*meshObj).vertexColors.begin() + i);

						zMeshObjectStorage::get(*meshObj).vertexNormals.erase(zMeshObjectStorage::get(*meshObj).vertexNormals.begin() + i);

						zMeshObjectStorage::get(*meshObj).vHandles.erase(zMeshObjectStorage::get(*meshObj).vHandles.begin() + i);

						zMeshObjectStorage::get(*meshObj).n_v--;
					}
				}
			}

			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).vHandles.size(); i++) zMeshObjectStorage::get(*meshObj).vHandles[i].id = i;

			zMeshObjectStorage::get(*meshObj).resizeArray(zVertexData, numVertices());

			printf("\n removed inactive vertices. ");
		}

		//  Edge
		else if (type == zEdgeData || type == zHalfEdgeData)
		{
			bool reindex = false;

			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).heHandles.size(); i++)
			{
				bool active = (zMeshObjectStorage::get(*meshObj).heHandles[i].id == -1) ? false : true;

				if (!active)
				{
					reindex = true;
					while (zMeshObjectStorage::get(*meshObj).heHandles[i].id == -1 &&  i < zMeshObjectStorage::get(*meshObj).heHandles.size())
					{
						zMeshObjectStorage::get(*meshObj).heHandles.erase(zMeshObjectStorage::get(*meshObj).heHandles.begin() + i);
						zMeshObjectStorage::get(*meshObj).n_he--;

					}
				}
			}

			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).heHandles.size(); i++) zMeshObjectStorage::get(*meshObj).heHandles[i].id = i;

			zMeshObjectStorage::get(*meshObj).resizeArray(zHalfEdgeData, numHalfEdges());

			printf("\n removed inactive halfedges. ");

			
			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).eHandles.size(); i++)
			{
				bool active = (zMeshObjectStorage::get(*meshObj).eHandles[i].id == -1) ? false : true;

				if (!active)
				{
					reindex = true;
					while (zMeshObjectStorage::get(*meshObj).eHandles[i].id == -1 && i < zMeshObjectStorage::get(*meshObj).eHandles.size())
					{
						zMeshObjectStorage::get(*meshObj).edgeColors.erase(zMeshObjectStorage::get(*meshObj).edgeColors.begin() + i);

						zMeshObjectStorage::get(*meshObj).eHandles.erase(zMeshObjectStorage::get(*meshObj).eHandles.begin() + i);

						zMeshObjectStorage::get(*meshObj).n_e--;
					}
				}
			}

			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).eHandles.size(); i++)
			{
				zMeshObjectStorage::get(*meshObj).eHandles[i].id = i;

				zMeshObjectStorage::get(*meshObj).eHandles[i].he0 = (i * 2);
				zMeshObjectStorage::get(*meshObj).eHandles[i].he1 = (i * 2) +1;
			}

			zMeshObjectStorage::get(*meshObj).resizeArray(zEdgeData, numEdges());

			printf("\n removed inactive edges. ");
		}

		// Mesh Face
		else if (type == zFaceData)
		{			
			bool reindex = false;

			//printf("\n fhendles %i ", zMeshObjectStorage::get(*meshObj).fHandles.size());

			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).fHandles.size(); i++)
			{
				bool active = (zMeshObjectStorage::get(*meshObj).fHandles[i].id == -1) ? false : true;

				if (!active)
				{
					reindex = true;
					while (zMeshObjectStorage::get(*meshObj).fHandles[i].id == -1 && i < zMeshObjectStorage::get(*meshObj).fHandles.size())
					{
						zMeshObjectStorage::get(*meshObj).faceColors.erase(zMeshObjectStorage::get(*meshObj).faceColors.begin() + i);
						zMeshObjectStorage::get(*meshObj).faceNormals.erase(zMeshObjectStorage::get(*meshObj).faceNormals.begin() + i);
						zMeshObjectStorage::get(*meshObj).fHandles.erase(zMeshObjectStorage::get(*meshObj).fHandles.begin() + i);
						zMeshObjectStorage::get(*meshObj).n_f--;							
					}
				}
			}		
	
			for (int i = 0; i < zMeshObjectStorage::get(*meshObj).fHandles.size(); i++) zMeshObjectStorage::get(*meshObj).fHandles[i].id = i;
		

			zMeshObjectStorage::get(*meshObj).resizeArray(zFaceData, zMeshObjectStorage::get(*meshObj).fHandles.size());

			printf("\n removed inactive faces. ");
		}

		else throw std::invalid_argument(" error: invalid zHEData type");
	}
}
