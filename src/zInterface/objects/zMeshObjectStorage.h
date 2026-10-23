#ifndef ZSPACE_MESH_OBJECT_STORAGE_H
#define ZSPACE_MESH_OBJECT_STORAGE_H

#pragma once

#include <src/zCore/geometry/detail/zMeshFaceListStorage.h>
#include <src/zCore/geometry/detail/zMeshStorage.h>
#include <zspace/zInterface/objects/zObjectMesh.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>
#include <unordered_map>

namespace zSpace
{
	class zObjectMesh::Impl
	{
	public:
		detail::zMeshFaceListStorage faceList;
		mutable std::unique_ptr<zMesh> topology;
		mutable bool topologyDirty = true;
		mutable bool faceListDirty = false;
		mutable std::unordered_map<std::string, int> positionLookup;
		mutable bool positionLookupDirty = true;

		Impl() = default;
		Impl(const Impl& other)
			: faceList(other.faceList),
			  topology(other.topology ? std::make_unique<zMesh>(*other.topology) : nullptr),
			  topologyDirty(other.topologyDirty),
			  faceListDirty(other.faceListDirty)
		{
		}
	};

	class zMeshObjectStorage
	{
		static std::uint64_t edgeKey(int a, int b)
		{
			const auto endpoints = std::minmax(a, b);
			return (std::uint64_t(std::uint32_t(endpoints.first)) << 32) | std::uint32_t(endpoints.second);
		}

		static void requireManifold(const detail::zMeshFaceListStorage& data)
		{
			struct EdgeUse
			{
				int count = 0;
				int direction = 0;
			};
			std::unordered_map<std::uint64_t, EdgeUse> uses;
			uses.reserve(static_cast<std::size_t>(data.numEdges()));

			for (int faceId = 0; faceId < data.numFaces(); ++faceId)
			{
				const int begin = data.faceOffsets[faceId];
				const int end = data.faceOffsets[faceId + 1];
				for (int i = begin; i < end; ++i)
				{
					const int next = (i + 1 < end) ? i + 1 : begin;
					const int a = data.faceVertexIndices[i];
					const int b = data.faceVertexIndices[next];
					const auto endpoints = std::minmax(a, b);
					auto& use = uses[edgeKey(endpoints.first, endpoints.second)];
					const int direction = (a == endpoints.first) ? 1 : -1;
					if (use.count >= 2)
						throw std::runtime_error("This topology query requires a manifold mesh; an edge has more than two incident faces.");
					if (use.count == 1 && use.direction == direction)
						throw std::runtime_error("This topology query requires consistently oriented manifold faces.");
					use.count++;
					use.direction = direction;
				}
			}
		}

		static void buildTopology(const zObjectMesh& object)
		{
			auto& impl = *object.impl;
			if (impl.faceListDirty) syncFaceList(object);
			if (impl.topology && !impl.topologyDirty) return;

			requireManifold(impl.faceList);
			zIntArray counts;
			zIntArray connects;
			impl.faceList.polygonData(connects, counts);

			auto topology = std::make_unique<zMesh>();
			zPointArray positions = impl.faceList.positions;
			topology->create(positions, counts, connects);
			topology->vertexNormals = impl.faceList.vertexNormals;
			topology->faceNormals = impl.faceList.faceNormals;
			topology->vertexColors = impl.faceList.vertexColors;
			topology->faceColors = impl.faceList.faceColors;
			topology->vertexWeights = impl.faceList.vertexWeights;

			std::unordered_map<std::uint64_t, int> sourceEdges;
			sourceEdges.reserve(static_cast<std::size_t>(impl.faceList.numEdges()));
			for (std::size_t i = 0; i + 1 < impl.faceList.edgeVertexIndices.size(); i += 2)
			{
				sourceEdges[edgeKey(impl.faceList.edgeVertexIndices[i], impl.faceList.edgeVertexIndices[i + 1])] = static_cast<int>(i / 2);
			}
			for (int edgeId = 0; edgeId < topology->n_e; ++edgeId)
			{
				const int halfEdge0 = topology->edges[edgeId].getHalfEdge(0);
				const int halfEdge1 = topology->edges[edgeId].getHalfEdge(1);
				const auto source = sourceEdges.find(edgeKey(
					topology->halfEdges[halfEdge0].getVertex(),
					topology->halfEdges[halfEdge1].getVertex()));
				if (source == sourceEdges.end()) continue;
				if (source->second < static_cast<int>(impl.faceList.edgeColors.size()))
					topology->edgeColors[edgeId] = impl.faceList.edgeColors[source->second];
				if (source->second < static_cast<int>(impl.faceList.edgeWeights.size()))
					topology->edgeWeights[edgeId] = impl.faceList.edgeWeights[source->second];
			}

			impl.topology = std::move(topology);
			impl.topologyDirty = false;
			impl.faceListDirty = false;
		}

		static void syncFaceList(const zObjectMesh& object)
		{
			auto& impl = *object.impl;
			if (!impl.faceListDirty || !impl.topology) return;
			zMesh& topology = *impl.topology;

			zPointArray positions;
			zColorArray vertexColors;
			zDoubleArray vertexWeights;
			zVectorArray vertexNormals;
			std::vector<int> vertexMap(topology.vertices.size(), -1);
			for (std::size_t i = 0; i < topology.vertices.size(); ++i)
			{
				if (!topology.vertices[i].isActive()) continue;
				vertexMap[i] = static_cast<int>(positions.size());
				positions.push_back(topology.vertexPositions[i]);
				if (i < topology.vertexColors.size()) vertexColors.push_back(topology.vertexColors[i]);
				if (i < topology.vertexWeights.size()) vertexWeights.push_back(topology.vertexWeights[i]);
				if (i < topology.vertexNormals.size()) vertexNormals.push_back(topology.vertexNormals[i]);
			}

			zIntArray counts;
			zIntArray connects;
			zColorArray faceColors;
			zVectorArray faceNormals;
			for (std::size_t i = 0; i < topology.faces.size(); ++i)
			{
				if (!topology.faces[i].isActive()) continue;
				zIntArray vertices;
				topology.getFaceVertices(static_cast<int>(i), vertices);
				bool valid = vertices.size() >= 3;
				for (int vertex : vertices)
					valid = valid && vertex >= 0 && vertex < static_cast<int>(vertexMap.size()) && vertexMap[vertex] >= 0;
				if (!valid) continue;
				counts.push_back(static_cast<int>(vertices.size()));
				for (int vertex : vertices) connects.push_back(vertexMap[vertex]);
				if (i < topology.faceColors.size()) faceColors.push_back(topology.faceColors[i]);
				if (i < topology.faceNormals.size()) faceNormals.push_back(topology.faceNormals[i]);
			}

			detail::zMeshFaceListStorage rebuilt;
			rebuilt.set(positions, counts, connects);
			if (vertexColors.size() == positions.size()) rebuilt.vertexColors = std::move(vertexColors);
			if (vertexWeights.size() == positions.size()) rebuilt.vertexWeights = std::move(vertexWeights);
			if (vertexNormals.size() == positions.size()) rebuilt.vertexNormals = std::move(vertexNormals);
			if (faceColors.size() == counts.size()) rebuilt.faceColors = std::move(faceColors);
			if (faceNormals.size() == counts.size()) rebuilt.faceNormals = std::move(faceNormals);

			std::unordered_map<std::uint64_t, int> rebuiltEdges;
			rebuiltEdges.reserve(static_cast<std::size_t>(rebuilt.numEdges()));
			for (std::size_t i = 0; i + 1 < rebuilt.edgeVertexIndices.size(); i += 2)
				rebuiltEdges[edgeKey(rebuilt.edgeVertexIndices[i], rebuilt.edgeVertexIndices[i + 1])] = static_cast<int>(i / 2);
			for (std::size_t edgeId = 0; edgeId < topology.edges.size(); ++edgeId)
			{
				if (!topology.edges[edgeId].isActive()) continue;
				const int he0 = topology.edges[edgeId].getHalfEdge(0);
				const int he1 = topology.edges[edgeId].getHalfEdge(1);
				const int oldA = topology.halfEdges[he0].getVertex();
				const int oldB = topology.halfEdges[he1].getVertex();
				if (oldA < 0 || oldB < 0 || oldA >= static_cast<int>(vertexMap.size()) ||
					oldB >= static_cast<int>(vertexMap.size())) continue;
				const auto target = rebuiltEdges.find(edgeKey(vertexMap[oldA], vertexMap[oldB]));
				if (target == rebuiltEdges.end()) continue;
				if (edgeId < topology.edgeColors.size()) rebuilt.edgeColors[target->second] = topology.edgeColors[edgeId];
				if (edgeId < topology.edgeWeights.size()) rebuilt.edgeWeights[target->second] = topology.edgeWeights[edgeId];
			}

			impl.faceList = std::move(rebuilt);
			impl.positionLookupDirty = true;
			impl.faceListDirty = false;
			impl.topologyDirty = false;
		}

	public:
		// Internal cache diagnostic; observing state must not synchronize it.
		static bool needsFaceListSync(const zObjectMesh& object)
		{
			return object.impl->faceListDirty;
		}

		static bool hasTopology(const zObjectMesh& object)
		{
			return object.impl->topology && !object.impl->topologyDirty;
		}

		static const zMesh* readTopologyIfPresent(const zObjectMesh& object)
		{
			if (!hasTopology(object)) return nullptr;
			syncFaceList(object);
			return object.impl->topology.get();
		}

		static bool vertexExists(const zObjectMesh& object, zPoint position, int& id, int precision)
		{
			// A live edited topology owns the legacy position hash. Reading it
			// preserves its existing lookup semantics without creating topology.
			if (hasTopology(object)) return object.impl->topology->vertexExists(position, id, precision);
			auto& impl = *object.impl;
			const auto& data = read(object);
			auto key = [](zPoint p, int digits)
			{
				const double factor = std::pow(10, digits);
				return std::to_string(std::round(p.x * factor) / factor) + "," +
					std::to_string(std::round(p.y * factor) / factor) + "," +
					std::to_string(std::round(p.z * factor) / factor);
			};
			if (impl.positionLookupDirty)
			{
				impl.positionLookup.clear();
				impl.positionLookup.reserve(data.positions.size());
				// zMesh::create inserts at precision 6; duplicate keys select the
				// last vertex. Query precision changes only the queried key.
				for (int i = 0; i < data.numVertices(); ++i) impl.positionLookup[key(data.positions[i], 6)] = i;
				impl.positionLookupDirty = false;
			}
			const auto match = impl.positionLookup.find(key(position, precision));
			id = match == impl.positionLookup.end() ? -1 : match->second;
			return match != impl.positionLookup.end();
		}

		static zMesh& get(zObjectMesh& object)
		{
			buildTopology(object);
			object.impl->faceListDirty = true;
			return *object.impl->topology;
		}

		static const zMesh& get(const zObjectMesh& object)
		{
			buildTopology(object);
			return *object.impl->topology;
		}

		static const detail::zMeshFaceListStorage& read(const zObjectMesh& object)
		{
			syncFaceList(object);
			return object.impl->faceList;
		}

		static detail::zMeshFaceListStorage& edit(zObjectMesh& object)
		{
			syncFaceList(object);
			object.impl->topologyDirty = true;
			object.impl->topology.reset();
			object.impl->positionLookupDirty = true;
			return object.impl->faceList;
		}

		static void set(zObjectMesh& object, const zPointArray& positions,
			const zIntArray& polygonCounts, const zIntArray& polygonConnects)
		{
			object.impl->faceList.set(positions, polygonCounts, polygonConnects);
			object.impl->topology.reset();
			object.impl->topologyDirty = true;
			object.impl->faceListDirty = false;
			object.impl->positionLookupDirty = true;
		}

		static void setMoved(zObjectMesh& object, zPointArray&& positions,
			const zIntArray& polygonCounts, zIntArray&& polygonConnects)
		{
			object.impl->faceList.setMoved(std::move(positions), polygonCounts, std::move(polygonConnects));
			object.impl->topology.reset();
			object.impl->topologyDirty = true;
			object.impl->faceListDirty = false;
			object.impl->positionLookupDirty = true;
		}
	};
}

#endif
