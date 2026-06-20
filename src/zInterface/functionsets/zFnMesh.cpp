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
#include <map>
#include <queue>
#include <utility>

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

		bool solve5x5(double A[5][5], double b[5], double x[5])
		{
			double tempA[5][5];
			double tempB[5];

			for (int i = 0; i < 5; ++i)
			{
				tempB[i] = b[i];
				x[i] = 0.0;
				for (int j = 0; j < 5; ++j) tempA[i][j] = A[i][j];
			}

			for (int i = 0; i < 5; ++i)
			{
				int pivotRow = i;
				double maxValue = std::abs(tempA[i][i]);
				for (int r = i + 1; r < 5; ++r)
				{
					double value = std::abs(tempA[r][i]);
					if (value > maxValue)
					{
						maxValue = value;
						pivotRow = r;
					}
				}

				if (maxValue < 1.0e-12) return false;

				if (pivotRow != i)
				{
					for (int col = i; col < 5; ++col) std::swap(tempA[i][col], tempA[pivotRow][col]);
					std::swap(tempB[i], tempB[pivotRow]);
				}

				for (int r = i + 1; r < 5; ++r)
				{
					double factor = tempA[r][i] / tempA[i][i];
					for (int col = i; col < 5; ++col) tempA[r][col] -= factor * tempA[i][col];
					tempB[r] -= factor * tempB[i];
				}
			}

			for (int i = 4; i >= 0; --i)
			{
				double sum = 0.0;
				for (int col = i + 1; col < 5; ++col) sum += tempA[i][col] * x[col];
				x[i] = (tempB[i] - sum) / tempA[i][i];
			}

			return true;
		}

		void solveSymmetric2x2(
			double A,
			double B,
			double C,
			double& L1,
			double& L2,
			double& v1x,
			double& v1y,
			double& v2x,
			double& v2y)
		{
			double disc = ((A - C) * (A - C)) + (4.0 * B * B);
			double sqrtDisc = std::sqrt(std::max(0.0, disc));
			L1 = 0.5 * (A + C + sqrtDisc);
			L2 = 0.5 * (A + C - sqrtDisc);

			double u1 = B;
			double u2 = L1 - A;
			double len = std::sqrt((u1 * u1) + (u2 * u2));
			if (len > 1.0e-9)
			{
				v1x = u1 / len;
				v1y = u2 / len;
			}
			else
			{
				v1x = 1.0;
				v1y = 0.0;
			}

			v2x = -v1y;
			v2y = v1x;
		}

		void appendUnique(zIntArray& values, int value)
		{
			if (std::find(values.begin(), values.end(), value) == values.end()) values.push_back(value);
		}

		void getKRingVertices(int start, int kRing, zIntArray& out, const vector<zIntArray>& vertexToVertices)
		{
			out.clear();
			if (start < 0 || start >= (int)vertexToVertices.size()) return;

			vector<bool> visited(vertexToVertices.size(), false);
			std::queue<std::pair<int, int>> queue;

			queue.push(std::make_pair(start, 0));
			visited[start] = true;

			while (!queue.empty())
			{
				std::pair<int, int> current = queue.front();
				queue.pop();
				out.push_back(current.first);

				if (current.second >= kRing) continue;

				for (int neighbour : vertexToVertices[current.first])
				{
					if (neighbour < 0 || neighbour >= (int)visited.size() || visited[neighbour]) continue;

					visited[neighbour] = true;
					queue.push(std::make_pair(neighbour, current.second + 1));
				}
			}
		}

		void getSphereVertices(
			int start,
			double radius,
			zIntArray& out,
			int minCount,
			const vector<zIntArray>& vertexToVertices,
			const zVectorArray& positions)
		{
			out.clear();
			if (start < 0 || start >= (int)vertexToVertices.size()) return;

			vector<bool> visited(vertexToVertices.size(), false);
			std::queue<int> queue;
			vector<std::pair<int, double>> extraCandidates;

			queue.push(start);
			visited[start] = true;
			zVector center = positions[start];

			while (!queue.empty())
			{
				int current = queue.front();
				queue.pop();
				out.push_back(current);

				for (int neighbour : vertexToVertices[current])
				{
					if (neighbour < 0 || neighbour >= (int)visited.size() || visited[neighbour]) continue;

					zVector neighbourPosition = positions[neighbour];
					zVector delta = neighbourPosition - center;
					double distance = vectorLength(delta);
					if (distance < radius)
					{
						queue.push(neighbour);
					}
					else if ((int)out.size() < minCount)
					{
						extraCandidates.push_back(std::make_pair(neighbour, distance));
					}

					visited[neighbour] = true;
				}
			}

			while (!extraCandidates.empty() && (int)out.size() < minCount)
			{
				std::sort(extraCandidates.begin(), extraCandidates.end(),
					[](const std::pair<int, double>& a, const std::pair<int, double>& b)
					{
						return a.second < b.second;
					});

				std::pair<int, double> candidate = extraCandidates.front();
				extraCandidates.erase(extraCandidates.begin());
				out.push_back(candidate.first);

				for (int neighbour : vertexToVertices[candidate.first])
				{
					if (neighbour < 0 || neighbour >= (int)visited.size() || visited[neighbour]) continue;

					zVector neighbourPosition = positions[neighbour];
					zVector delta = neighbourPosition - center;
					double distance = vectorLength(delta);
					extraCandidates.push_back(std::make_pair(neighbour, distance));
					visited[neighbour] = true;
				}
			}
		}

		zVector computeFaceNormalAndArea(const zIntArray& faceVertices, const zVectorArray& positions, double& area)
		{
			area = 0.0;
			if (faceVertices.size() < 3) return zVector();

			zVector centroid;
			for (int vertexId : faceVertices)
			{
				zVector position = positions[vertexId];
				centroid += position;
			}
			centroid /= (float)faceVertices.size();

			zVector normalSum;
			for (int i = 0; i < (int)faceVertices.size(); ++i)
			{
				zVector p1 = positions[faceVertices[i]];
				zVector p2 = positions[faceVertices[(i + 1) % faceVertices.size()]];
				zVector v1 = p1 - centroid;
				zVector v2 = p2 - centroid;
				normalSum += (v1 ^ v2);
			}

			double length = vectorLength(normalSum);
			if (length <= 1.0e-9) return zVector();

			area = 0.5 * length;
			return normalized(normalSum);
		}

		struct Point2
		{
			double x;
			double y;
		};

		Point2 projectPoint(const zPoint& point, int axis)
		{
			if (axis == 0) return { point.y, point.z };
			if (axis == 1) return { point.x, point.z };
			return { point.x, point.y };
		}

		double signedArea2D(const std::vector<Point2>& points)
		{
			double area = 0.0;
			for (std::size_t i = 0; i < points.size(); ++i)
			{
				const auto& a = points[i];
				const auto& b = points[(i + 1) % points.size()];
				area += a.x * b.y - b.x * a.y;
			}
			return area * 0.5;
		}

		double cross2D(const Point2& a, const Point2& b, const Point2& c)
		{
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}

		bool pointInTriangle2D(const Point2& point, const Point2& a,
			const Point2& b, const Point2& c, bool ccw)
		{
			const double ab = cross2D(a, b, point);
			const double bc = cross2D(b, c, point);
			const double ca = cross2D(c, a, point);
			const double epsilon = 1.0e-10;
			return ccw ? (ab >= -epsilon && bc >= -epsilon && ca >= -epsilon)
				: (ab <= epsilon && bc <= epsilon && ca <= epsilon);
		}

		zIntArray triangulatePolygon(const zIntArray& polygon, const zPointArray& positions)
		{
			zIntArray triangles;
			if (polygon.size() < 3) return triangles;
			if (polygon.size() == 3) return polygon;

			double area = 0.0;
			const zVector normal = computeFaceNormalAndArea(polygon, positions, area);
			const double ax = std::abs(normal.x);
			const double ay = std::abs(normal.y);
			const double az = std::abs(normal.z);
			const int axis = (ax >= ay && ax >= az) ? 0 : ((ay >= az) ? 1 : 2);

			std::vector<Point2> projected;
			projected.reserve(polygon.size());
			for (int id : polygon) projected.push_back(projectPoint(positions[id], axis));
			const bool ccw = signedArea2D(projected) >= 0.0;

			std::vector<int> remaining(polygon.size());
			for (int i = 0; i < static_cast<int>(remaining.size()); ++i) remaining[i] = i;
			while (remaining.size() > 3)
			{
				bool found = false;
				for (int i = 0; i < static_cast<int>(remaining.size()); ++i)
				{
					const int previous = remaining[(i + remaining.size() - 1) % remaining.size()];
					const int current = remaining[i];
					const int next = remaining[(i + 1) % remaining.size()];
					const double corner = cross2D(projected[previous], projected[current], projected[next]);
					if ((ccw && corner <= 1.0e-10) || (!ccw && corner >= -1.0e-10)) continue;

					bool containsPoint = false;
					for (int candidate : remaining)
					{
						if (candidate == previous || candidate == current || candidate == next) continue;
						if (pointInTriangle2D(projected[candidate], projected[previous],
							projected[current], projected[next], ccw))
						{
							containsPoint = true;
							break;
						}
					}
					if (containsPoint) continue;

					triangles.push_back(polygon[previous]);
					triangles.push_back(polygon[current]);
					triangles.push_back(polygon[next]);
					remaining.erase(remaining.begin() + i);
					found = true;
					break;
				}
				if (!found) throw std::invalid_argument("Polygon triangulation failed; polygon may be degenerate or self-intersecting.");
			}

			for (int index : remaining) triangles.push_back(polygon[index]);
			return triangles;
		}

		vector<std::pair<int, int>> getBoundaryEdges(const detail::zMeshFaceListStorage& data)
		{
			std::map<std::pair<int, int>, int> edgeUseCount;
			std::map<std::pair<int, int>, std::pair<int, int>> orientedEdges;

			for (int faceId = 0; faceId < data.numFaces(); ++faceId)
			{
				const int begin = data.faceOffsets[faceId];
				const int end = data.faceOffsets[faceId + 1];
				for (int i = begin; i < end; ++i)
				{
					const int next = (i + 1 < end) ? i + 1 : begin;
					const int v0 = data.faceVertexIndices[i];
					const int v1 = data.faceVertexIndices[next];
					const auto key = std::minmax(v0, v1);

					edgeUseCount[key]++;
					if (orientedEdges.find(key) == orientedEdges.end()) orientedEdges[key] = std::make_pair(v0, v1);
				}
			}

			vector<std::pair<int, int>> out;
			for (const auto& item : edgeUseCount)
			{
				if (item.second == 1) out.push_back(orientedEdges[item.first]);
			}

			return out;
		}

		vector<zIntArray> getEdgeIncidentFaces(const detail::zMeshFaceListStorage& data)
		{
			std::map<std::pair<int, int>, int> edgeIds;
			for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
			{
				const int v0 = data.edgeVertexIndices[edgeId * 2];
				const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
				const auto endpoints = std::minmax(v0, v1);
				edgeIds[{ endpoints.first, endpoints.second }] = edgeId;
			}

			vector<zIntArray> incidentFaces(data.numEdges());
			for (int faceId = 0; faceId < data.numFaces(); ++faceId)
			{
				const int begin = data.faceOffsets[faceId];
				const int end = data.faceOffsets[faceId + 1];
				for (int i = begin; i < end; ++i)
				{
					const int next = (i + 1 < end) ? i + 1 : begin;
					const int v0 = data.faceVertexIndices[i];
					const int v1 = data.faceVertexIndices[next];
					const auto endpoints = std::minmax(v0, v1);
					const auto edge = edgeIds.find({ endpoints.first, endpoints.second });
					if (edge != edgeIds.end()) incidentFaces[edge->second].push_back(faceId);
				}
			}

			return incidentFaces;
		}

		void appendExtrudeSideFace(
			const std::pair<int, int>& edge,
			int offset,
			bool thicknessTris,
			zIntArray& polyCounts,
			zIntArray& polyConnects)
		{
			const int v0 = edge.first;
			const int v1 = edge.second;

			if (thicknessTris)
			{
				polyConnects.push_back(v1);
				polyConnects.push_back(v0);
				polyConnects.push_back(v0 + offset);

				polyConnects.push_back(v0 + offset);
				polyConnects.push_back(v1 + offset);
				polyConnects.push_back(v1);

				polyCounts.push_back(3);
				polyCounts.push_back(3);
			}
			else
			{
				polyConnects.push_back(v1);
				polyConnects.push_back(v0);
				polyConnects.push_back(v0 + offset);
				polyConnects.push_back(v1 + offset);

				polyCounts.push_back(4);
			}
		}

		struct ScalarCorner
		{
			zPoint position;
			float scalar;
			zColor color;
		};

		ScalarCorner interpolateCorner(const ScalarCorner& a, const ScalarCorner& b, float threshold)
		{
			const float denominator = b.scalar - a.scalar;
			const float t = std::abs(denominator) <= 1.0e-12f ? 0.0f : (threshold - a.scalar) / denominator;
			ScalarCorner out;
			zPoint aPosition = a.position;
			zPoint bPosition = b.position;
			out.position = aPosition + (bPosition - aPosition) * t;
			out.scalar = threshold;
			out.color.r = a.color.r + (b.color.r - a.color.r) * t;
			out.color.g = a.color.g + (b.color.g - a.color.g) * t;
			out.color.b = a.color.b + (b.color.b - a.color.b) * t;
			out.color.a = a.color.a + (b.color.a - a.color.a) * t;
			return out;
		}

		std::vector<ScalarCorner> clipScalarPolygon(
			const std::vector<ScalarCorner>& input, float threshold, bool keepAbove)
		{
			std::vector<ScalarCorner> output;
			if (input.empty()) return output;
			auto inside = [&](const ScalarCorner& corner)
			{
				return keepAbove ? corner.scalar >= threshold : corner.scalar <= threshold;
			};

			ScalarCorner previous = input.back();
			bool previousInside = inside(previous);
			for (const auto& current : input)
			{
				const bool currentInside = inside(current);
				if (currentInside != previousInside)
					output.push_back(interpolateCorner(previous, current, threshold));
				if (currentInside) output.push_back(current);
				previous = current;
				previousInside = currentInside;
			}
			return output;
		}
	}

	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnMesh::zFnMesh()
	{
		fnType = zFnType::zMeshFn; 
		meshObj = nullptr;
	}

	ZSPACE_INLINE zFnMesh::zFnMesh(zObjectMesh &_meshObj)
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
		zPointArray positions = zMeshObjectStorage::read(*meshObj).positions;
		coreUtils.getBounds(positions, minBB, maxBB);
	}

	ZSPACE_INLINE void zFnMesh::clear()
	{
		zMeshObjectStorage::edit(*meshObj).clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnMesh::reserve(int _n_v, int  _n_e, int _n_f)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.clear();
		data.positions.reserve(_n_v);
		data.faceOffsets.reserve(_n_f + 1);
		data.edgeVertexIndices.reserve(_n_e * 2);
	}

	ZSPACE_INLINE void zFnMesh::create(zPointArray& _positions, zIntArray& polyCounts, zIntArray& polyConnects)
	{

		zMeshObjectStorage::set(*meshObj, _positions, polyCounts, polyConnects);
			
		// compute mesh normals
		computeMeshNormals();
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
		return zMeshObjectStorage::read(*meshObj).numVertices();
	}

	ZSPACE_INLINE int zFnMesh::numEdges()
	{
		return zMeshObjectStorage::read(*meshObj).numEdges();
	}

	ZSPACE_INLINE bool zFnMesh::edgeExists(int v1, int v2, int &outEdgeId)
	{
		const auto& data = zMeshObjectStorage::read(*meshObj);
		const int a = std::min(v1, v2);
		const int b = std::max(v1, v2);

		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			if (data.edgeVertexIndices[edgeId * 2] == a &&
				data.edgeVertexIndices[edgeId * 2 + 1] == b)
			{
				outEdgeId = edgeId;
				return true;
			}
		}

		outEdgeId = -1;
		return false;
	}

	ZSPACE_INLINE int zFnMesh::numHalfEdges()
	{
		return zMeshObjectStorage::get(*meshObj).n_he;
	}

	ZSPACE_INLINE int zFnMesh::numPolygons()
	{
		return zMeshObjectStorage::read(*meshObj).numFaces();
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
		{
			const auto& existingData = zMeshObjectStorage::read(*meshObj);
			if (existingData.faceNormals.size() != existingData.numFaces()) computeMeshNormals();
		}

		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int begin = data.faceOffsets[faceId];
			const int end = data.faceOffsets[faceId + 1];
			if (begin >= end) continue;

			zVector normal = data.faceNormals[faceId];
			zPoint pointOnFace = data.positions[data.faceVertexIndices[begin]];
			const double normalLength = sqrt(normal * normal);
			if (normalLength <= 1.0e-9) continue;

			const double d = (normal * pointOnFace) * -1.0;
			const double distance = (pt * normal) + d;
			if ((distance / normalLength) >= 0.0) return false;
		}

		return true;
	}

	//--- COMPUTE METHODS 

	ZSPACE_INLINE void zFnMesh::computeEdgeColorfromVertexColor()
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.edgeColors.assign(data.numEdges(), zColor());
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			zColor& color = data.edgeColors[edgeId];
			color.r = (data.vertexColors[v0].r + data.vertexColors[v1].r) * 0.5f;
			color.g = (data.vertexColors[v0].g + data.vertexColors[v1].g) * 0.5f;
			color.b = (data.vertexColors[v0].b + data.vertexColors[v1].b) * 0.5f;
			color.a = (data.vertexColors[v0].a + data.vertexColors[v1].a) * 0.5f;
		}
	}

	ZSPACE_INLINE void zFnMesh::computeVertexColorfromEdgeColor()
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		zColorArray colors(data.positions.size(), zColor());
		zIntArray counts(data.positions.size(), 0);
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			for (int endpoint = 0; endpoint < 2; ++endpoint)
			{
				const int vertexId = data.edgeVertexIndices[edgeId * 2 + endpoint];
				colors[vertexId].r += data.edgeColors[edgeId].r;
				colors[vertexId].g += data.edgeColors[edgeId].g;
				colors[vertexId].b += data.edgeColors[edgeId].b;
				colors[vertexId].a += data.edgeColors[edgeId].a;
				counts[vertexId]++;
			}
		}
		for (std::size_t i = 0; i < colors.size(); ++i)
		{
			if (counts[i] == 0) continue;
			colors[i].r /= counts[i]; colors[i].g /= counts[i];
			colors[i].b /= counts[i]; colors[i].a /= counts[i];
		}
		data.vertexColors = std::move(colors);
	}

	ZSPACE_INLINE void zFnMesh::computeFaceColorfromVertexColor()
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.faceColors.assign(data.numFaces(), zColor());
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int count = data.faceOffsets[faceId + 1] - data.faceOffsets[faceId];
			for (int i = data.faceOffsets[faceId]; i < data.faceOffsets[faceId + 1]; ++i)
			{
				const zColor& source = data.vertexColors[data.faceVertexIndices[i]];
				data.faceColors[faceId].r += source.r;
				data.faceColors[faceId].g += source.g;
				data.faceColors[faceId].b += source.b;
				data.faceColors[faceId].a += source.a;
			}
			data.faceColors[faceId].r /= count; data.faceColors[faceId].g /= count;
			data.faceColors[faceId].b /= count; data.faceColors[faceId].a /= count;
		}
	}

	ZSPACE_INLINE void zFnMesh::computeVertexColorfromFaceColor()
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		zColorArray colors(data.positions.size(), zColor());
		zIntArray counts(data.positions.size(), 0);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			for (int i = data.faceOffsets[faceId]; i < data.faceOffsets[faceId + 1]; ++i)
			{
				const int vertexId = data.faceVertexIndices[i];
				colors[vertexId].r += data.faceColors[faceId].r;
				colors[vertexId].g += data.faceColors[faceId].g;
				colors[vertexId].b += data.faceColors[faceId].b;
				colors[vertexId].a += data.faceColors[faceId].a;
				counts[vertexId]++;
			}
		}
		for (std::size_t i = 0; i < colors.size(); ++i)
		{
			if (counts[i] == 0) continue;
			colors[i].r /= counts[i]; colors[i].g /= counts[i];
			colors[i].b /= counts[i]; colors[i].a /= counts[i];
		}
		data.vertexColors = std::move(colors);
	}

	ZSPACE_INLINE void zFnMesh::smoothColors(int smoothVal, zHEData type)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (type == zVertexData && data.vertexColors.size() != data.numVertices())
			data.vertexColors.assign(data.numVertices(), zColor(1, 0, 0, 1));
		if (type == zFaceData && data.faceColors.size() != data.numFaces())
			data.faceColors.assign(data.numFaces(), zColor(1, 0, 0, 1));

		vector<zIntArray> vertexAdjacency(data.numVertices());
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			vertexAdjacency[v0].push_back(v1);
			vertexAdjacency[v1].push_back(v0);
		}

		vector<zIntArray> faceAdjacency(data.numFaces());
		if (type == zFaceData)
		{
			std::map<std::pair<int, int>, int> edgeFace;
			for (int faceId = 0; faceId < data.numFaces(); ++faceId)
			{
				const int begin = data.faceOffsets[faceId];
				const int end = data.faceOffsets[faceId + 1];
				for (int i = begin; i < end; ++i)
				{
					const int next = (i + 1 < end) ? i + 1 : begin;
					const int a = data.faceVertexIndices[i];
					const int b = data.faceVertexIndices[next];
					const auto key = std::minmax(a, b);
					auto it = edgeFace.find(key);
					if (it == edgeFace.end()) edgeFace[key] = faceId;
					else
					{
						faceAdjacency[faceId].push_back(it->second);
						faceAdjacency[it->second].push_back(faceId);
					}
				}
			}
		}

		for (int j = 0; j < smoothVal; j++)
		{
			if (type == zVertexData)
			{
				zColorArray tempColors = data.vertexColors;

				for (int vertexId = 0; vertexId < data.numVertices(); ++vertexId)
				{
					zColor col;
					col.r = data.vertexColors[vertexId].r;
					col.g = data.vertexColors[vertexId].g;
					col.b = data.vertexColors[vertexId].b;
					col.a = data.vertexColors[vertexId].a;

					for (int connectedVertex : vertexAdjacency[vertexId])
					{
						col.r += data.vertexColors[connectedVertex].r;
						col.g += data.vertexColors[connectedVertex].g;
						col.b += data.vertexColors[connectedVertex].b;
						col.a += data.vertexColors[connectedVertex].a;
					}

					const float denominator = static_cast<float>(vertexAdjacency[vertexId].size() + 1);
					col.r /= denominator; col.g /= denominator;
					col.b /= denominator; col.a /= denominator;
					tempColors[vertexId] = col;
				}

				data.vertexColors = tempColors;
			}

			else if (type == zFaceData)
			{
				zColorArray tempColors = data.faceColors;

				for (int faceId = 0; faceId < data.numFaces(); ++faceId)
				{
					zColor col;
					col.r = data.faceColors[faceId].r;
					col.g = data.faceColors[faceId].g;
					col.b = data.faceColors[faceId].b;
					col.a = data.faceColors[faceId].a;

					for (int connectedFace : faceAdjacency[faceId])
					{
						col.r += data.faceColors[connectedFace].r;
						col.g += data.faceColors[connectedFace].g;
						col.b += data.faceColors[connectedFace].b;
						col.a += data.faceColors[connectedFace].a;
					}

					const float denominator = static_cast<float>(faceAdjacency[faceId].size() + 1);
					col.r /= denominator; col.g /= denominator;
					col.b /= denominator; col.a /= denominator;
					tempColors[faceId] = col;
				}

				data.faceColors = tempColors;
			}

			else throw std::invalid_argument(" error: invalid zHEData type");
		}
	}

	ZSPACE_INLINE void zFnMesh::computeVertexNormalfromFaceNormal()
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.vertexNormals.assign(data.positions.size(), zVector());
		zIntArray contributions(data.positions.size(), 0);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const zVector normal = faceId < static_cast<int>(data.faceNormals.size())
				? data.faceNormals[faceId] : zVector();
			for (int i = data.faceOffsets[faceId]; i < data.faceOffsets[faceId + 1]; ++i)
			{
				const int vertexId = data.faceVertexIndices[i];
				data.vertexNormals[vertexId] += normal;
				contributions[vertexId]++;
			}
		}
		for (std::size_t i = 0; i < data.vertexNormals.size(); ++i)
		{
			if (contributions[i] > 0) data.vertexNormals[i] /= static_cast<float>(contributions[i]);
			data.vertexNormals[i].normalize();
		}
	}

	ZSPACE_INLINE void zFnMesh::computeMeshNormals()
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.faceNormals.assign(data.numFaces(), zVector());
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int begin = data.faceOffsets[faceId];
			const int end = data.faceOffsets[faceId + 1];
			zVector center;
			for (int i = begin; i < end; ++i) center += data.positions[data.faceVertexIndices[i]];
			center /= static_cast<float>(end - begin);

			zVector normal;
			for (int i = begin; i < end; ++i)
			{
				const int next = (i + 1 < end) ? i + 1 : begin;
				normal += (data.positions[data.faceVertexIndices[i]] - center) ^
					(data.positions[data.faceVertexIndices[next]] - center);
			}
			normal.normalize();
			data.faceNormals[faceId] = normal;
		}
		// compute vertex normal
		computeVertexNormalfromFaceNormal();

		
	}

	ZSPACE_INLINE void zFnMesh::averageVertices(int numSteps)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
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
				for (int connectedVertex : adjacency[vertexId])
				{
					avg += data.positions[connectedVertex];
				}

				avg /= static_cast<float>(adjacency[vertexId].size() + 1);
				tempVertPos[vertexId] = avg;
			}

			data.positions = tempVertPos;
		}
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
		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			if ((data.faceOffsets[faceId + 1] - data.faceOffsets[faceId]) != 3) return false;
		}

		return true;
	}

	ZSPACE_INLINE bool zFnMesh::isQuadMesh()
	{
		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			if ((data.faceOffsets[faceId + 1] - data.faceOffsets[faceId]) != 4) return false;
		}

		return true;
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
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (pos.size() != data.positions.size()) throw std::invalid_argument("size of position container is not equal to number of mesh vertices.");
		data.positions = pos;
	}

	ZSPACE_INLINE void zFnMesh::setVertexColor(zColor col, bool setFaceColor)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.vertexColors.assign(data.positions.size(), col);

		if (setFaceColor) computeFaceColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnMesh::setVertexColorsfromScalars(zScalarArray& vertexScalars, bool setFaceColor)
	{
		if (vertexScalars.size() != numVertices())
		{
			throw std::invalid_argument(" error: scalars values to match number of vertices");
			return;
		}

		auto& data = zMeshObjectStorage::edit(*meshObj);
		for (int i = 0; i < vertexScalars.size(); i++)
		{

			zColor col;

			if (vertexScalars[i] < 0) col = zColor(0.941, 0, 0.157, 1);
			if (vertexScalars[i] > 0.0) col = zColor(0, 0.941, 0.157, 1);

			data.vertexColors[i] = col;

		}

		if (setFaceColor) computeFaceColorfromVertexColor();

	}

	ZSPACE_INLINE void zFnMesh::setVertexColors(zColorArray& col, bool setFaceColor)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (col.size() != data.positions.size()) throw std::invalid_argument("size of color container is not equal to number of mesh vertices.");
		data.vertexColors = col;

		if (setFaceColor) computeFaceColorfromVertexColor();
	}

	ZSPACE_INLINE void zFnMesh::setVertexWeight(double wt)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.vertexWeights.assign(data.positions.size(), wt);
	}

	ZSPACE_INLINE void zFnMesh::setVertexWeights(zDoubleArray& wt)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (wt.size() != data.positions.size()) throw std::invalid_argument("size of weight container is not equal to number of mesh vertices.");
		data.vertexWeights = wt;
	}

	ZSPACE_INLINE void zFnMesh::setFaceColor(zColor col, bool setVertexColor)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.faceColors.assign(data.numFaces(), col);

		if (setVertexColor) computeVertexColorfromFaceColor();
	}

	ZSPACE_INLINE void zFnMesh::setFaceColors(zColorArray& col, bool setVertexColor)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (col.size() != static_cast<std::size_t>(data.numFaces())) throw std::invalid_argument("size of color container is not equal to number of mesh faces.");
		data.faceColors = col;

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
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.faceNormals.assign(data.numFaces(), fNormal);

		// compute normals per face based on vertex normals and store it in faceNormals
		computeVertexNormalfromFaceNormal();
	}

	ZSPACE_INLINE void zFnMesh::setFaceNormals(zVectorArray &fNormals)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (static_cast<std::size_t>(data.numFaces()) != fNormals.size()) throw std::invalid_argument("size of normal container is not equal to number of mesh faces.");
		data.faceNormals = fNormals;

		// compute normals per face based on vertex normals and store it in faceNormals
		computeVertexNormalfromFaceNormal();
	}

	ZSPACE_INLINE void zFnMesh::setEdgeColor(zColor col, bool setVertexColor)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.edgeColors.assign(data.numEdges(), col);

		if (setVertexColor) computeVertexColorfromEdgeColor();
	}

	ZSPACE_INLINE void zFnMesh::setEdgeColors(zColorArray& col, bool setVertexColor)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (col.size() != static_cast<std::size_t>(data.numEdges())) throw std::invalid_argument("size of color container is not equal to number of mesh edges.");
		data.edgeColors = col;

		if (setVertexColor) computeVertexColorfromEdgeColor();
	}

	ZSPACE_INLINE void zFnMesh::setEdgeWeight(double wt)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		data.edgeWeights.assign(data.numEdges(), wt);
	}

	ZSPACE_INLINE void zFnMesh::setEdgeWeight(int index, double wt)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (index < 0 || index >= data.numEdges()) throw std::invalid_argument("edge index out of bounds.");
		data.edgeWeights[index] = wt;
	}

	ZSPACE_INLINE void zFnMesh::setEdgeWeights(zDoubleArray& wt)
	{
		auto& data = zMeshObjectStorage::edit(*meshObj);
		if (wt.size() != static_cast<std::size_t>(data.numEdges())) throw std::invalid_argument("size of weight container is not equal to number of mesh edges.");
		data.edgeWeights = wt;
	}

	//--- GET METHODS 
	ZSPACE_INLINE void zFnMesh::getVertexPositions(zPointArray &pos, bool exludeCornerVertices)
	{
		pos = zMeshObjectStorage::read(*meshObj).positions;

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

		return zMeshObjectStorage::edit(*meshObj).positions.data();
	}

	ZSPACE_INLINE void zFnMesh::getRawVertexPositions(float** points)
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");
			
		

		auto& positions = zMeshObjectStorage::edit(*meshObj).positions;
		for (int i = 0; i < static_cast<int>(positions.size()); i++)
		{
			points[(i * 3) + 0] = &positions[i].x;
			points[(i * 3) + 1] = &positions[i].y;
			points[(i * 3) + 2] = &positions[i].z;
		}

	}

	ZSPACE_INLINE void zFnMesh::getVertexNormals(zVectorArray& norm)
	{
		norm = zMeshObjectStorage::read(*meshObj).vertexNormals;
	}

	ZSPACE_INLINE zVector* zFnMesh::getRawVertexNormals()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return zMeshObjectStorage::edit(*meshObj).vertexNormals.data();
	}

	ZSPACE_INLINE void zFnMesh::getRawVertexNormals(float** normals)
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		auto& vertexNormals = zMeshObjectStorage::edit(*meshObj).vertexNormals;
		for (int i = 0; i < static_cast<int>(vertexNormals.size()); i++)
		{
			normals[(i * 3) + 0] = &vertexNormals[i].x;
			normals[(i * 3) + 1] = &vertexNormals[i].y;
			normals[(i * 3) + 2] = &vertexNormals[i].z;
		}

	}

	ZSPACE_INLINE void zFnMesh::getVertexColors(zColorArray& col)
	{
		col = zMeshObjectStorage::read(*meshObj).vertexColors;
	}

	ZSPACE_INLINE void zFnMesh::getVertexWeights(zDoubleArray& weights)
	{
		weights = zMeshObjectStorage::read(*meshObj).vertexWeights;
	}

	ZSPACE_INLINE zColor* zFnMesh::getRawVertexColors()
	{
		if (numVertices() == 0) throw std::invalid_argument(" error: null pointer.");

		return zMeshObjectStorage::edit(*meshObj).vertexColors.data();
	}

	ZSPACE_INLINE void zFnMesh::getEdgeColors(zColorArray& col)
	{
		col = zMeshObjectStorage::read(*meshObj).edgeColors;
	}

	ZSPACE_INLINE void zFnMesh::getEdgeWeights(zDoubleArray& weights)
	{
		weights = zMeshObjectStorage::read(*meshObj).edgeWeights;
	}

	ZSPACE_INLINE zColor* zFnMesh::getRawEdgeColors()
	{
		if (numEdges() == 0) throw std::invalid_argument(" error: null pointer.");

		return zMeshObjectStorage::edit(*meshObj).edgeColors.data();
	}

	ZSPACE_INLINE void zFnMesh::getFaceNormals(zVectorArray& norm)
	{
		norm = zMeshObjectStorage::read(*meshObj).faceNormals;
	}

	ZSPACE_INLINE zVector* zFnMesh::getRawFaceNormals()
	{
		if (numPolygons() == 0) throw std::invalid_argument(" error: null pointer.");

		return zMeshObjectStorage::edit(*meshObj).faceNormals.data();
	}

	ZSPACE_INLINE void zFnMesh::getFaceColors(zColorArray& col)
	{
		col = zMeshObjectStorage::read(*meshObj).faceColors;
	}

	ZSPACE_INLINE zColor* zFnMesh::getRawFaceColors()
	{
		if (numEdges() == 0) throw std::invalid_argument(" error: null pointer.");

		return zMeshObjectStorage::edit(*meshObj).faceColors.data();
	}

	ZSPACE_INLINE zPoint zFnMesh::getCenter()
	{
		zPoint out;
		const auto& positions = zMeshObjectStorage::read(*meshObj).positions;
		for (const auto& position : positions) out += position;
		if (!positions.empty()) out /= static_cast<float>(positions.size());

		return out;
	}

	ZSPACE_INLINE void zFnMesh::getCenters(zHEData type, zPointArray &centers)
	{
		const auto& data = zMeshObjectStorage::read(*meshObj);
		centers.clear();

		// Mesh Edge 
		if (type == zHalfEdgeData)
		{
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
			centers.reserve(data.numEdges());
			for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
			{
				const int v0 = data.edgeVertexIndices[edgeId * 2];
				const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
				zPoint p0 = data.positions[v0];
				zPoint p1 = data.positions[v1];
				centers.push_back((p0 + p1) * 0.5f);
			}
		}

		// Mesh Face 
		else if (type == zFaceData)
		{
			centers.reserve(data.numFaces());
			for (int faceId = 0; faceId < data.numFaces(); ++faceId)
			{
				zPoint center;
				const int begin = data.faceOffsets[faceId];
				const int end = data.faceOffsets[faceId + 1];
				for (int i = begin; i < end; ++i) center += data.positions[data.faceVertexIndices[i]];
				if (end > begin) center /= static_cast<float>(end - begin);
				centers.push_back(center);
			}
		}
		else throw std::invalid_argument(" error: invalid zHEData type");
	}

	ZSPACE_INLINE void zFnMesh::getDualMesh(zObjectMesh &dualMeshObj, zIntArray &inEdge_dualEdge, zIntArray &dualEdge_inEdge, bool excludeBoundary, bool keepExistingBoundary, bool rotate90)
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

	ZSPACE_INLINE void zFnMesh::getDualGraph(zObjectGraph &dualGraphObj, zIntArray &inEdge_dualEdge, zIntArray &dualEdge_inEdge, bool excludeBoundary , bool PlanarMesh , bool rotate90)
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
		zItGraphHalfEdge dualHalfEdgeIt(dualGraphObj);
		dualEdge_inEdge.assign(dualHalfEdgeIt.size(), -1);

		// compute inEdge to dualEdge	
		inEdge_dualEdge.clear();
		zIntArray dualGraphEdgeConnects;
		fnDualGraph.getEdgeData(dualGraphEdgeConnects);

		for (int i = 0; i < numHalfEdges(); i++)
		{
			int v1 = inEdge_dualVertex[i];
			int v2 = (i % 2 == 0) ? inEdge_dualVertex[i + 1] : inEdge_dualVertex[i - 1];

			int eId;
			bool chk = fnDualGraph.edgeExists(v1, v2, eId);

			if (chk)
			{
				int heId = (dualGraphEdgeConnects[eId * 2] == v1) ? eId * 2 : eId * 2 + 1;
				inEdge_dualEdge.push_back(heId);
			}
			else inEdge_dualEdge.push_back(-1);

			if (inEdge_dualEdge[i] != -1)
			{
				dualEdge_inEdge[inEdge_dualEdge[i]] = i;
			}
		}
	}

	ZSPACE_INLINE void zFnMesh::getRainflowGraph(zObjectGraph &rainflowGraphObj, bool excludeBoundary)
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
		faceTris.clear();
		const auto& data = zMeshObjectStorage::read(*meshObj);
		faceTris.reserve(data.numFaces());
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			zIntArray polygon(data.faceVertexIndices.begin() + data.faceOffsets[faceId],
				data.faceVertexIndices.begin() + data.faceOffsets[faceId + 1]);
			faceTris.push_back(triangulatePolygon(polygon, data.positions));
		}
	}

	ZSPACE_INLINE double zFnMesh::getMeshVolume()
	{
		double out = 0;

		vector<vector<int>> faceTris;
		getMeshTriangles(faceTris);

		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (const auto& triangles : faceTris)
		{
			for (int j = 0; j < triangles.size(); j += 3)
			{
				zPoint a = data.positions[triangles[j]];
				zPoint b = data.positions[triangles[j + 1]];
				zPoint c = data.positions[triangles[j + 2]];
				double vol = coreUtils.getSignedTriangleVolume(a, b, c);

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

		const auto& data = zMeshObjectStorage::read(*meshObj);
		faceVolumes.reserve(data.numFaces());
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			double vol = 0.0;
			for (int j = 0; j < faceTris[faceId].size(); j += 3)
			{
				zPoint a = data.positions[faceTris[faceId][j]];
				zPoint b = data.positions[faceTris[faceId][j + 1]];
				zPoint c = data.positions[faceTris[faceId][j + 2]];
				vol += coreUtils.getSignedTriangleVolume(a, b, c);
			}
			const int begin = data.faceOffsets[faceId];
			const int end = data.faceOffsets[faceId + 1];
			for (int i = begin; i < end; ++i)
			{
				const int previous = (i == begin) ? end - 1 : i - 1;
				zPoint a = data.positions[data.faceVertexIndices[i]];
				zPoint b = data.positions[data.faceVertexIndices[previous]];
				zPoint center = fCenters[faceId];
				vol += coreUtils.getSignedTriangleVolume(a, b, center);
			}
			if (absoluteVolumes) vol = std::abs(vol);
			faceVolumes.push_back(vol);
		}
	}

	ZSPACE_INLINE void zFnMesh::getPrincipalCurvatures(zCurvatureArray &vertexCurvatures, zVectorArray& pVector1, zVectorArray& pVector2)
	{
		vertexCurvatures.clear();
		pVector1.clear();
		pVector2.clear();

		const int radius = 5;
		const bool useKring = true;
		const int nV = numVertices();
		const int nF = numPolygons();

		vertexCurvatures.assign(nV, zCurvature());
		pVector1.assign(numVertices(), zVector());
		pVector2.assign(numVertices(), zVector());

		if (nV == 0) return;

		zVectorArray positions(nV);
		positions = zMeshObjectStorage::read(*meshObj).positions;

		vector<zIntArray> vertexToVertices(nV);
		vector<zIntArray> vertexToFaces(nV);
		vector<zIntArray> faceVertexIds(nF);

		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int begin = data.faceOffsets[faceId];
			const int end = data.faceOffsets[faceId + 1];
			faceVertexIds[faceId].assign(data.faceVertexIndices.begin() + begin, data.faceVertexIndices.begin() + end);

			const zIntArray& fVerts = faceVertexIds[faceId];
			for (int i = 0; i < (int)fVerts.size(); ++i)
			{
				int u = fVerts[i];
				int v = fVerts[(i + 1) % fVerts.size()];

				appendUnique(vertexToVertices[u], v);
				appendUnique(vertexToVertices[v], u);
				appendUnique(vertexToFaces[u], faceId);
			}
		}

		zVectorArray faceNormals(nF);
		zDoubleArray faceAreas(nF, 0.0);
		for (int i = 0; i < nF; ++i)
		{
			faceNormals[i] = computeFaceNormalAndArea(faceVertexIds[i], positions, faceAreas[i]);
		}

		zVectorArray vertexNormals(nV);
		for (int i = 0; i < nV; ++i)
		{
			zVector normalSum;
			for (int faceId : vertexToFaces[i])
			{
				normalSum += faceNormals[faceId] * (float)faceAreas[faceId];
			}

			if (vectorLength(normalSum) > 1.0e-9) vertexNormals[i] = normalized(normalSum);
			else vertexNormals[i] = zVector(0, 0, 1);
		}

		double averageEdgeLength = 1.0;
		double edgeLengthSum = 0.0;
		int edgeLengthCount = 0;
		for (const zIntArray& fVerts : faceVertexIds)
		{
			for (int i = 0; i < (int)fVerts.size(); ++i)
			{
				zVector edge = positions[fVerts[(i + 1) % fVerts.size()]] - positions[fVerts[i]];
				edgeLengthSum += vectorLength(edge);
				edgeLengthCount++;
			}
		}
		if (edgeLengthCount > 0) averageEdgeLength = edgeLengthSum / (double)edgeLengthCount;

		double scaledRadius = averageEdgeLength * (double)radius;

		for (zItMeshVertex vertex(*meshObj); !vertex.end(); vertex++)
		{
			int id = vertex.getId();

			if (!vertex.isActive() || vertexToVertices[id].empty())
			{
				vertexCurvatures[id].k1 = 0;
				vertexCurvatures[id].k2 = 0;
				continue;
			}

			zIntArray neighbourhood;
			if (useKring) getKRingVertices(id, radius, neighbourhood, vertexToVertices);
			else getSphereVertices(id, scaledRadius, neighbourhood, 6, vertexToVertices, positions);

			if (neighbourhood.size() < 6) continue;

			zIntArray filteredNeighbourhood;
			zVector ppn = vertexNormals[id];
			for (int neighbour : neighbourhood)
			{
				zVector nNormal = vertexNormals[neighbour];
				if ((nNormal * ppn) > 0.0) filteredNeighbourhood.push_back(neighbour);
			}

			if (filteredNeighbourhood.size() >= 6 && filteredNeighbourhood.size() < neighbourhood.size())
			{
				neighbourhood = filteredNeighbourhood;
			}

			if (neighbourhood.size() < 6) continue;

			zVector normal = vertexNormals[id];
			zVector diff = positions[vertexToVertices[id][0]] - positions[id];
			zVector xAxis = diff - (normal * (float)(diff * normal));
			if (vectorLength(xAxis) < 1.0e-9)
			{
				for (int j = 1; j < (int)vertexToVertices[id].size(); ++j)
				{
					diff = positions[vertexToVertices[id][j]] - positions[id];
					xAxis = diff - (normal * (float)(diff * normal));
					if (vectorLength(xAxis) > 1.0e-9) break;
				}
			}

			if (vectorLength(xAxis) < 1.0e-9)
			{
				if (std::abs(normal.x) < 0.9 && std::abs(normal.y) < 0.9)
				{
					xAxis = normal ^ zVector(0, 0, 1);
				}
				else
				{
					xAxis = normal ^ zVector(1, 0, 0);
				}
			}

			xAxis = normalized(xAxis);
			zVector yAxis = normalized(normal ^ xAxis);

			double normalEquations[5][5] = {};
			double rhs[5] = {};
			zVector center = positions[id];

			for (int neighbour : neighbourhood)
			{
				zVector tangent = positions[neighbour] - center;
				double u = tangent * xAxis;
				double v = tangent * yAxis;
				double n = tangent * normal;

				double row[5] = { u * u, u * v, v * v, u, v };
				for (int r = 0; r < 5; ++r)
				{
					for (int c = 0; c < 5; ++c) normalEquations[r][c] += row[r] * row[c];
					rhs[r] += row[r] * n;
				}
			}

			double coefficients[5] = {};
			if (!solve5x5(normalEquations, rhs, coefficients)) continue;

			double aVal = coefficients[0];
			double bVal = coefficients[1];
			double cVal = coefficients[2];
			double dVal = coefficients[3];
			double eVal = coefficients[4];

			double E = 1.0 + (dVal * dVal);
			double F = dVal * eVal;
			double G = 1.0 + (eVal * eVal);

			double nLength = std::sqrt((dVal * dVal) + (eVal * eVal) + 1.0);
			if (nLength <= 1.0e-9) continue;

			double n2 = 1.0 / nLength;
			double L = 2.0 * aVal * n2;
			double M = bVal * n2;
			double N = 2.0 * cVal * n2;

			double det = (E * G) - (F * F);
			if (std::abs(det) < 1.0e-9) continue;

			double m00 = ((L * G) - (M * F)) / det;
			double m01 = ((M * E) - (L * F)) / det;
			double m11 = ((N * E) - (M * F)) / det;

			double l1, l2, v1x, v1y, v2x, v2y;
			solveSymmetric2x2(m00, m01, m11, l1, l2, v1x, v1y, v2x, v2y);

			l1 = -l1;
			l2 = -l2;

			zVector v1Global = (xAxis * (float)v1x) + (yAxis * (float)v1y);
			zVector v2Global = (xAxis * (float)v2x) + (yAxis * (float)v2y);

			v1Global = normalized(v1Global);
			v2Global = normalized(v2Global);

			if (!std::isfinite(v1Global.x) || !std::isfinite(v1Global.y) || !std::isfinite(v1Global.z) ||
				!std::isfinite(v2Global.x) || !std::isfinite(v2Global.y) || !std::isfinite(v2Global.z))
			{
				continue;
			}

			double k1, k2;
			zVector pd1;
			zVector pd2;
			if (l1 > l2)
			{
				k1 = l1;
				k2 = l2;
				pd1 = v1Global;
				pd2 = v2Global;
			}
			else
			{
				k1 = l2;
				k2 = l1;
				pd1 = v2Global;
				pd2 = v1Global;
			}

			if ((pd1 * pd2) > 1.0e-5)
			{
				pd1 = zVector();
				pd2 = zVector();
			}

			vertexCurvatures[id].k1 = k1;
			vertexCurvatures[id].k2 = k2;
			pVector1[id] = pd1;
			pVector2[id] = pd2;
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
		const auto& data = zMeshObjectStorage::read(*meshObj);
		if (planarityDevs.size() != data.numFaces())
		{
			planarityDevs.clear();
			planarityDevs.assign(data.numFaces(), -1);
		}

		zPointArray faceCenters;
		vector<zIntArray> faceTriangles;
		zDoubleArray faceVolumes;
		if (type == zVolumePlanar)
		{
			getCenters(zFaceData, faceCenters);
			getMeshTriangles(faceTriangles);
			getMeshFaceVolumes(faceTriangles, faceCenters, faceVolumes, false);
		}

		auto& editableData = zMeshObjectStorage::edit(*meshObj);
		for (int faceId = 0; faceId < editableData.numFaces(); ++faceId)
		{
			zPointArray fVerts;
			const int begin = editableData.faceOffsets[faceId];
			const int end = editableData.faceOffsets[faceId + 1];
			fVerts.reserve(end - begin);
			for (int i = begin; i < end; ++i)
				fVerts.push_back(editableData.positions[editableData.faceVertexIndices[i]]);

			if (type == zQuadPlanar)
			{
				if (fVerts.size() == 3)planarityDevs[faceId] = 0.0;

				if (fVerts.size() == 4)
				{
					double uA, uB;
					zPoint pA, pB;

					coreUtils.line_lineClosestPoints(fVerts[0], fVerts[2], fVerts[1], fVerts[3], uA, uB, pA, pB);
					planarityDevs[faceId] = pA.distanceTo(pB);
				}
				
			}

			if (type == zVolumePlanar)
			{
				planarityDevs[faceId] = abs(faceVolumes[faceId]);
			}

			if (colorFaces && faceId < static_cast<int>(editableData.faceColors.size()))
			{
				if (planarityDevs[faceId] == -1) continue;
				editableData.faceColors[faceId] = (planarityDevs[faceId] < tolerance) ? zGREEN : zMAGENTA;
			}
		}
			
		

		
	}

	ZSPACE_INLINE void zFnMesh::getEdgeDihedralAngles(zDoubleArray &dihedralAngles)
	{
		{
			const auto& existingData = zMeshObjectStorage::read(*meshObj);
			if (existingData.faceNormals.size() != existingData.numFaces()) computeMeshNormals();
		}

		const auto& data = zMeshObjectStorage::read(*meshObj);
		vector<zIntArray> edgeIncidentFaces = getEdgeIncidentFaces(data);

		vector<double> out;
		out.reserve(data.numEdges());
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			if (edgeIncidentFaces[edgeId].size() != 2)
			{
				out.push_back(-1.0);
				continue;
			}

			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			zPoint p0 = data.positions[v0];
			zPoint p1 = data.positions[v1];
			zVector edgeVector = p0 - p1;
			zVector n0 = data.faceNormals[edgeIncidentFaces[edgeId][0]];
			zVector n1 = data.faceNormals[edgeIncidentFaces[edgeId][1]];

			out.push_back(edgeVector.dihedralAngle(n0, n1));
		}

		dihedralAngles = out;
	}

	ZSPACE_INLINE double zFnMesh::getHalfEdgeLengths(zDoubleArray &halfEdgeLengths)
	{
		double total = 0.0;

		halfEdgeLengths.clear();
		const auto& data = zMeshObjectStorage::read(*meshObj);
		halfEdgeLengths.reserve(data.numEdges() * 2);

		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			zPoint p0 = data.positions[v0];
			zPoint p1 = data.positions[v1];
			const double edgeLength = p0.distanceTo(p1);

			halfEdgeLengths.push_back(edgeLength);
			halfEdgeLengths.push_back(edgeLength);

			total += edgeLength;
		}

		return total;
	}

	ZSPACE_INLINE double zFnMesh::getEdgeLengths(zDoubleArray &edgeLengths)
	{
		double total = 0.0;


		edgeLengths.clear();
		const auto& data = zMeshObjectStorage::read(*meshObj);
		edgeLengths.reserve(data.numEdges());

		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			const int v0 = data.edgeVertexIndices[edgeId * 2];
			const int v1 = data.edgeVertexIndices[edgeId * 2 + 1];
			zPoint p0 = data.positions[v0];
			zPoint p1 = data.positions[v1];
			const double edgeLength = p0.distanceTo(p1);

			edgeLengths.push_back(edgeLength);
			total += edgeLength;
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
		faceAreas.clear();
		double totalArea = 0;
		const auto& data = zMeshObjectStorage::read(*meshObj);
		faceAreas.reserve(data.numFaces());
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			zIntArray polygon(data.faceVertexIndices.begin() + data.faceOffsets[faceId],
				data.faceVertexIndices.begin() + data.faceOffsets[faceId + 1]);
			double fArea = 0.0;
			computeFaceNormalAndArea(polygon, data.positions, fArea);
			faceAreas.push_back(fArea);

			totalArea += fArea;
		}

		return totalArea;
	}

	ZSPACE_INLINE void zFnMesh::getPolygonData(zIntArray(&polyConnects), zIntArray(&polyCounts))
	{
		zMeshObjectStorage::read(*meshObj).polygonData(polyConnects, polyCounts);
	}

	ZSPACE_INLINE void zFnMesh::getMatrices_trimesh(MatrixXd& V, MatrixXi& F)
	{
		const auto& data = zMeshObjectStorage::read(*meshObj);
		MatrixXd triMesh_V(data.numVertices(), 3);

		// fill vertex matrix
		for (int i = 0; i < data.numVertices(); i++)
		{
			triMesh_V(i, 0) = data.positions[i].x;
			triMesh_V(i, 1) = data.positions[i].y;
			triMesh_V(i, 2) = data.positions[i].z;
		}

		V = triMesh_V;

		// fill triangle matrix
		MatrixXi FTris(data.numFaces(), 3);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int begin = data.faceOffsets[faceId];
			if (data.faceOffsets[faceId + 1] - begin != 3)
				throw std::invalid_argument("getMatrices_trimesh requires triangular faces.");
			for (int corner = 0; corner < 3; ++corner)
				FTris(faceId, corner) = data.faceVertexIndices[begin + corner];
		}

		F = FTris;
	}

	ZSPACE_INLINE void zFnMesh::getMatrices_quadmesh(MatrixXd& V, MatrixXi& F)
	{
		const auto& data = zMeshObjectStorage::read(*meshObj);
		MatrixXd quadMesh_V(data.numVertices(), 3);

		// fill vertex matrix
		for (int i = 0; i < data.numVertices(); i++)
		{
			quadMesh_V(i, 0) = data.positions[i].x;
			quadMesh_V(i, 1) = data.positions[i].y;
			quadMesh_V(i, 2) = data.positions[i].z;
		}

		V = quadMesh_V;

		// fill triangle matrix
		MatrixXi FQuads(data.numFaces(), 4);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int begin = data.faceOffsets[faceId];
			if (data.faceOffsets[faceId + 1] - begin != 4)
				throw std::invalid_argument("getMatrices_quadmesh requires quadrilateral faces.");
			for (int corner = 0; corner < 4; ++corner)
				FQuads(faceId, corner) = data.faceVertexIndices[begin + corner];
		}

		F = FQuads;
	}

	ZSPACE_INLINE void zFnMesh::getEdgeData(zIntArray &edgeConnects, bool excludeBoundary)
	{
		if (!excludeBoundary)
		{
			edgeConnects = zMeshObjectStorage::read(*meshObj).edgeVertexIndices;
			return;
		}
		edgeConnects.clear();
		const auto& data = zMeshObjectStorage::read(*meshObj);
		vector<zIntArray> edgeIncidentFaces = getEdgeIncidentFaces(data);
		for (int edgeId = 0; edgeId < data.numEdges(); ++edgeId)
		{
			if (edgeIncidentFaces[edgeId].size() < 2) continue;
			edgeConnects.push_back(data.edgeVertexIndices[edgeId * 2]);
			edgeConnects.push_back(data.edgeVertexIndices[edgeId * 2 + 1]);
		}
	}

	ZSPACE_INLINE void zFnMesh::getDuplicate(zObjectMesh &out)
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

		out = zObjectMesh(*meshObj);
	}

	

	//---- CONTOUR METHODS

	ZSPACE_INLINE void zFnMesh::splitMesh_Mixed(zPointArray& splitPlanes_origins, zVectorArray& splitPlanes_normals, zObjectMesh& resultMeshObj)
	{
		zObjectMesh tempObj;
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

	ZSPACE_INLINE void zFnMesh::splitMesh_Quad(vector<zPlane>& splitPlanes, bool invertMesh, zObjectMesh& resultMeshObj)
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

	ZSPACE_INLINE void zFnMesh::getIsoMesh_mixed(zScalarArray& vertexScalars, float threshold, bool invertMesh, zObjectMesh& coutourMeshObj)
	{
		getIsoMesh(vertexScalars, threshold, invertMesh, coutourMeshObj);
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
		unordered_map<string, int> positionVertex;
		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			if (selectedFaces)
			{
				if (faceId >= data.faceColors.size()) continue;
				zColor faceColor = data.faceColors[faceId];
				if (!(faceColor == selectedFaceColor)) continue;
			}
			std::vector<ScalarCorner> intersections;
			const int begin = data.faceOffsets[faceId];
			const int end = data.faceOffsets[faceId + 1];
			for (int i = begin; i < end; ++i)
			{
				const int next = (i + 1 == end) ? begin : i + 1;
				const int aId = data.faceVertexIndices[i];
				const int bId = data.faceVertexIndices[next];
				ScalarCorner a{ data.positions[aId], vertexScalars[aId],
					aId < data.vertexColors.size() ? data.vertexColors[aId] : zColor() };
				ScalarCorner b{ data.positions[bId], vertexScalars[bId],
					bId < data.vertexColors.size() ? data.vertexColors[bId] : zColor() };
				const float da = a.scalar - threshold;
				const float db = b.scalar - threshold;
				if (std::abs(da) <= 1.0e-12f) intersections.push_back(a);
				if (da * db < 0.0f) intersections.push_back(interpolateCorner(a, b, threshold));
			}
			for (std::size_t i = 0; i + 1 < intersections.size(); i += 2)
			{
				zPoint segmentStart = intersections[i].position;
				zPoint segmentEnd = intersections[i + 1].position;
				if (vectorLength(segmentEnd - segmentStart) <= distTolerance) continue;
				for (int endpoint = 0; endpoint < 2; ++endpoint)
				{
					const auto& corner = intersections[i + endpoint];
					int vertexId = -1;
					zPoint position = corner.position;
					if (!coreUtils.vertexExists(positionVertex, position, precision, vertexId))
					{
						vertexId = static_cast<int>(positions.size());
						positions.push_back(position);
						cVertexColor.push_back(corner.color);
						coreUtils.addToPositionMap(positionVertex, position, vertexId, precision);
					}
					edgeConnects.push_back(vertexId);
				}
			}
		}
	}


	ZSPACE_INLINE void zFnMesh::getIsoMesh(zScalarArray& vertexScalars, float threshold, bool invertMesh, zObjectMesh& coutourMeshObj)
	{

		if (vertexScalars.size() != numVertices())
		{
			throw std::invalid_argument(" error: scalars values to match number of vertices");
			return;
		}

		zPointArray positions;
		zIntArray polyConnects;
		zIntArray polyCounts;
		zColorArray colors;
		unordered_map<string, int> positionVertex;
		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			std::vector<ScalarCorner> polygon;
			for (int i = data.faceOffsets[faceId]; i < data.faceOffsets[faceId + 1]; ++i)
			{
				const int vertexId = data.faceVertexIndices[i];
				polygon.push_back({ data.positions[vertexId], vertexScalars[vertexId],
					vertexId < data.vertexColors.size() ? data.vertexColors[vertexId] : zColor() });
			}
			polygon = clipScalarPolygon(polygon, threshold, !invertMesh);
			if (polygon.size() < 3) continue;
			polyCounts.push_back(static_cast<int>(polygon.size()));
			for (const auto& corner : polygon)
			{
				int vertexId = -1;
				zPoint position = corner.position;
				if (!coreUtils.vertexExists(positionVertex, position, PRECISION, vertexId))
				{
					vertexId = static_cast<int>(positions.size());
					positions.push_back(position);
					colors.push_back(corner.color);
					coreUtils.addToPositionMap(positionVertex, position, vertexId, PRECISION);
				}
				polyConnects.push_back(vertexId);
			}
		}
		zFnMesh tempFn(coutourMeshObj);
		tempFn.create(positions, polyCounts, polyConnects);
		if (colors.size() == positions.size()) tempFn.setVertexColors(colors, false);

	}
	
	ZSPACE_INLINE void zFnMesh::getIsobandMesh(zScalarArray& vertexScalars, float inThresholdLow, float inThresholdHigh, zObjectMesh& coutourMeshObj)
	{
		if (vertexScalars.size() != numVertices())
			throw std::invalid_argument(" error: scalars values to match number of vertices");
		const float low = std::min(inThresholdLow, inThresholdHigh);
		const float high = std::max(inThresholdLow, inThresholdHigh);
		zPointArray positions;
		zIntArray polyConnects;
		zIntArray polyCounts;
		zColorArray colors;
		unordered_map<string, int> positionVertex;
		const auto& data = zMeshObjectStorage::read(*meshObj);
		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			std::vector<ScalarCorner> polygon;
			for (int i = data.faceOffsets[faceId]; i < data.faceOffsets[faceId + 1]; ++i)
			{
				const int vertexId = data.faceVertexIndices[i];
				polygon.push_back({ data.positions[vertexId], vertexScalars[vertexId],
					vertexId < data.vertexColors.size() ? data.vertexColors[vertexId] : zColor() });
			}
			polygon = clipScalarPolygon(polygon, low, true);
			polygon = clipScalarPolygon(polygon, high, false);
			if (polygon.size() < 3) continue;
			polyCounts.push_back(static_cast<int>(polygon.size()));
			for (const auto& corner : polygon)
			{
				int vertexId = -1;
				zPoint position = corner.position;
				if (!coreUtils.vertexExists(positionVertex, position, PRECISION, vertexId))
				{
					vertexId = static_cast<int>(positions.size());
					positions.push_back(position);
					colors.push_back(corner.color);
					coreUtils.addToPositionMap(positionVertex, position, vertexId, PRECISION);
				}
				polyConnects.push_back(vertexId);
			}
		}
		zFnMesh tempFn(coutourMeshObj);
		tempFn.create(positions, polyCounts, polyConnects);
		if (colors.size() == positions.size()) tempFn.setVertexColors(colors, false);
	}

	//---- TRI-MESH MODIFIER METHODS

	ZSPACE_INLINE void zFnMesh::faceTriangulate(zItMeshFace &face)
	{
		const auto source = zMeshObjectStorage::read(*meshObj);
		const int targetFace = face.getId();
		if (targetFace < 0 || targetFace >= source.numFaces())
			throw std::out_of_range("faceTriangulate face index is out of range.");

		zIntArray polygonCounts;
		zIntArray polygonConnects;
		zColorArray faceColors;
		for (int faceId = 0; faceId < source.numFaces(); ++faceId)
		{
			zIntArray polygon(source.faceVertexIndices.begin() + source.faceOffsets[faceId],
				source.faceVertexIndices.begin() + source.faceOffsets[faceId + 1]);
			if (faceId == targetFace && polygon.size() > 3)
			{
				const zIntArray triangles = triangulatePolygon(polygon, source.positions);
				for (int i = 0; i < triangles.size(); i += 3)
				{
					polygonCounts.push_back(3);
					polygonConnects.insert(polygonConnects.end(), triangles.begin() + i, triangles.begin() + i + 3);
					if (faceId < source.faceColors.size()) faceColors.push_back(source.faceColors[faceId]);
				}
			}
			else
			{
				polygonCounts.push_back(static_cast<int>(polygon.size()));
				polygonConnects.insert(polygonConnects.end(), polygon.begin(), polygon.end());
				if (faceId < source.faceColors.size()) faceColors.push_back(source.faceColors[faceId]);
			}
		}
		zMeshObjectStorage::set(*meshObj, source.positions, polygonCounts, polygonConnects);
		auto& result = zMeshObjectStorage::edit(*meshObj);
		result.vertexColors = source.vertexColors;
		result.vertexWeights = source.vertexWeights;
		result.vertexNormals = source.vertexNormals;
		result.faceColors = faceColors;
		computeMeshNormals();
	}

	ZSPACE_INLINE void zFnMesh::triangulate()
	{
		const auto source = zMeshObjectStorage::read(*meshObj);
		zIntArray polygonCounts;
		zIntArray polygonConnects;
		zColorArray faceColors;
		for (int faceId = 0; faceId < source.numFaces(); ++faceId)
		{
			zIntArray polygon(source.faceVertexIndices.begin() + source.faceOffsets[faceId],
				source.faceVertexIndices.begin() + source.faceOffsets[faceId + 1]);
			const zIntArray triangles = triangulatePolygon(polygon, source.positions);
			for (int i = 0; i < triangles.size(); i += 3)
			{
				polygonCounts.push_back(3);
				polygonConnects.insert(polygonConnects.end(), triangles.begin() + i, triangles.begin() + i + 3);
				if (faceId < source.faceColors.size()) faceColors.push_back(source.faceColors[faceId]);
			}
		}
		zMeshObjectStorage::set(*meshObj, source.positions, polygonCounts, polygonConnects);
		auto& result = zMeshObjectStorage::edit(*meshObj);
		result.vertexColors = source.vertexColors;
		result.vertexWeights = source.vertexWeights;
		result.vertexNormals = source.vertexNormals;
		result.faceColors = faceColors;
		computeMeshNormals();
	}

	//---- PRIVATE TOPOLOGY HELPERS

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


	ZSPACE_INLINE void zFnMesh::extrudeMesh(float extrudeThickness,zObjectMesh &out, bool thicknessTris)
	{
		{
			const auto& existingData = zMeshObjectStorage::read(*meshObj);
			if (existingData.vertexNormals.size() != existingData.numVertices()) computeMeshNormals();
		}

		const auto& data = zMeshObjectStorage::read(*meshObj);

		vector<zVector> positions;
		vector<int> polyCounts;
		vector<int> polyConnects;

		for (int i = 0; i < data.numVertices(); i++)
		{
			positions.push_back(data.positions[i]);
		}

		for (int i = 0; i < data.numVertices(); i++)
		{
			zVector normal = data.vertexNormals[i];
			zPoint position = data.positions[i];
			positions.push_back(position + (normal * extrudeThickness));
		}

		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int begin = data.faceOffsets[faceId];
			const int end = data.faceOffsets[faceId + 1];

			for (int i = begin; i < end; ++i)
			{
				polyConnects.push_back(data.faceVertexIndices[i]);
			}

			polyCounts.push_back(end - begin);

			for (int i = end - 1; i >= begin; --i)
			{
				polyConnects.push_back(data.faceVertexIndices[i] + data.numVertices());
			}

			polyCounts.push_back(end - begin);
		}

		vector<std::pair<int, int>> boundaryEdges = getBoundaryEdges(data);
		for (const auto& edge : boundaryEdges)
		{
			appendExtrudeSideFace(edge, data.numVertices(), thicknessTris, polyCounts, polyConnects);
		}

		zFnMesh tempFn(out);

		tempFn.clear();
		tempFn.create(positions, polyCounts, polyConnects);		
		
	}

	ZSPACE_INLINE void zFnMesh::extrudeVariableMesh(zFloatArray extrudeThickness, zObjectMesh& out, bool bothSides, bool thicknessTris)
	{
		{
			const auto& existingData = zMeshObjectStorage::read(*meshObj);
			if (existingData.vertexNormals.size() != existingData.numVertices()) computeMeshNormals();
		}

		if (extrudeThickness.size() != numVertices()) return;

		const auto& data = zMeshObjectStorage::read(*meshObj);

		vector<zVector> positions;
		vector<int> polyCounts;
		vector<int> polyConnects;

		for (int i = 0; i < data.numVertices(); i++)
		{
			zVector normal = data.vertexNormals[i];
			if(!bothSides) positions.push_back(data.positions[i]);
			else
			{
				zPoint position = data.positions[i];
				positions.push_back(position + (normal * extrudeThickness[i] * -1));
			}
		}

		for (int i = 0; i < data.numVertices(); i++)
		{
			zVector normal = data.vertexNormals[i];
			zPoint position = data.positions[i];
			positions.push_back(position + (normal * extrudeThickness[i]));
		}

		for (int faceId = 0; faceId < data.numFaces(); ++faceId)
		{
			const int begin = data.faceOffsets[faceId];
			const int end = data.faceOffsets[faceId + 1];

			for (int i = begin; i < end; ++i)
			{
				polyConnects.push_back(data.faceVertexIndices[i]);
			}

			polyCounts.push_back(end - begin);

			for (int i = end - 1; i >= begin; --i)
			{
				polyConnects.push_back(data.faceVertexIndices[i] + data.numVertices());
			}

			polyCounts.push_back(end - begin);
		}

		vector<std::pair<int, int>> boundaryEdges = getBoundaryEdges(data);
		for (const auto& edge : boundaryEdges)
		{
			appendExtrudeSideFace(edge, data.numVertices(), thicknessTris, polyCounts, polyConnects);
		}

		zFnMesh tempFn(out);

		tempFn.clear();
		tempFn.create(positions, polyCounts, polyConnects);

	}

	ZSPACE_INLINE void zFnMesh::extrudeBoundaryEdge(float extrudeThickness, zObjectMesh &out, bool thicknessTris)
	{
		{
			const auto& existingData = zMeshObjectStorage::read(*meshObj);
			if (existingData.vertexNormals.size() != existingData.numVertices()) computeMeshNormals();
		}

		const auto& data = zMeshObjectStorage::read(*meshObj);

		vector<zVector> positions;
		vector<int> polyCounts;
		vector<int> polyConnects;


		for (int i = 0; i < data.numVertices(); i++)
		{
			positions.push_back(data.positions[i]);
		}

		for (int i = 0; i < data.numVertices(); i++)
		{
			zVector normal = data.vertexNormals[i];
			zPoint position = data.positions[i];
			positions.push_back(position + (normal * extrudeThickness));
		}	


		vector<std::pair<int, int>> boundaryEdges = getBoundaryEdges(data);
		for (const auto& edge : boundaryEdges)
		{
			appendExtrudeSideFace(edge, data.numVertices(), thicknessTris, polyCounts, polyConnects);
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

	//---- PRIVATE DEACTIVATE AND REMOVE METHODS

	ZSPACE_INLINE void zFnMesh::addToHalfEdgesMap(zItMeshHalfEdge &he)
	{
		zMeshObjectStorage::get(*meshObj).addToHalfEdgesMap(he.getStartVertex().getId(), he.getVertex().getId(), he.getId());
	}

	ZSPACE_INLINE void zFnMesh::removeFromHalfEdgesMap(zItMeshHalfEdge &he)
	{
		zMeshObjectStorage::get(*meshObj).removeFromHalfEdgesMap(he.getStartVertex().getId(), he.getVertex().getId());
	}

}
