#include <zspace/zIO/zIO.h>

#include <src/zIO/codecs/zCodecGraphTXT.h>
#include <src/zIO/codecs/zCodecJSON.h>
#include <src/zIO/codecs/zCodecOBJ.h>
#include <src/zIO/codecs/zCodecPointCloudCSV.h>
#include <src/zIO/codecs/zCodecUSD.h>
#include <src/zIO/internal/zIOData.h>

#include <zspace/zInterface/functionsets/zFnGraph.h>
#include <zspace/zInterface/functionsets/zFnMesh.h>
#include <zspace/zInterface/functionsets/zFnPointCloud.h>
#include <zspace/zInterface/iterators/zItGraph.h>
#include <zspace/zInterface/iterators/zItMesh.h>

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace zSpace
{
	namespace
	{
		std::string extensionOf(const std::string& path)
		{
			std::string extension = std::filesystem::path(path).extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
			return extension;
		}

		bool isUSD(const std::string& extension)
		{
			return extension == ".usd" || extension == ".usda" ||
				extension == ".usdc" || extension == ".usdz";
		}

		zIOResult applyMeshData(io_detail::MeshData& data, zObjectMesh& mesh, bool staticGeometry)
		{
			if (data.polygonCounts.empty() || data.polygonConnects.empty())
				return zIOResult::error("Mesh data contains no polygons.");

			zFnMesh functionSet(mesh);
			functionSet.create(data.positions, data.polygonCounts, data.polygonConnects, staticGeometry);

			if (data.vertexColors.size() == data.positions.size())
				functionSet.setVertexColors(data.vertexColors);
			if (data.edgeColors.size() == static_cast<std::size_t>(functionSet.numEdges()))
				functionSet.setEdgeColors(data.edgeColors, false);
			if (data.edgeWeights.size() == static_cast<std::size_t>(functionSet.numEdges()))
				functionSet.setEdgeWeights(data.edgeWeights);
			if (data.faceColors.size() == data.polygonCounts.size())
				functionSet.setFaceColors(data.faceColors);
			if (data.faceNormals.size() == data.polygonCounts.size())
				functionSet.setFaceNormals(data.faceNormals);

			return zIOResult::ok();
		}

		zIOResult extractMeshData(zObjectMesh& mesh, io_detail::MeshData& data)
		{
			zFnMesh functionSet(mesh);
			if (!functionSet.hasObject()) return zIOResult::error("No mesh object was provided.");

			data = {};
			functionSet.getVertexPositions(data.positions);
			functionSet.getFaceNormals(data.faceNormals);
			functionSet.getVertexColors(data.vertexColors);
			functionSet.getEdgeColors(data.edgeColors);
			functionSet.getEdgeWeights(data.edgeWeights);
			functionSet.getFaceColors(data.faceColors);

			for (std::size_t i = 0; i < data.edgeColors.size(); ++i)
			{
				const auto& color = data.edgeColors[i];
				const double weight = i < data.edgeWeights.size() ? data.edgeWeights[i] : 1.0;
				data.edgeAttributes.push_back({ color.r, color.g, color.b, color.a, weight });
			}

			for (zItMeshFace face(mesh); !face.end(); face++)
			{
				if (!face.isActive()) continue;
				zIntArray vertices;
				face.getVertices(vertices);
				data.polygonCounts.push_back(static_cast<int>(vertices.size()));
				data.polygonConnects.insert(data.polygonConnects.end(), vertices.begin(), vertices.end());
			}

			if (data.positions.empty() || data.polygonCounts.empty())
				return zIOResult::error("Mesh contains no writable geometry.");
			return zIOResult::ok();
		}

		zIOResult applyGraphData(io_detail::GraphData& data, zObjectGraph& graph, bool staticGeometry)
		{
			if (data.edgeConnects.empty())
				return zIOResult::error("Graph data contains no edges.");

			zFnGraph functionSet(graph);
			functionSet.create(data.positions, data.edgeConnects, staticGeometry);

			if (data.vertexColors.size() == data.positions.size())
				functionSet.setVertexColors(data.vertexColors);
			if (data.edgeColors.size() * 2 == data.edgeConnects.size())
				functionSet.setEdgeColors(data.edgeColors, false);

			return zIOResult::ok();
		}

		zIOResult extractGraphData(zObjectGraph& graph, io_detail::GraphData& data)
		{
			zFnGraph functionSet(graph);
			if (!functionSet.hasObject()) return zIOResult::error("No graph object was provided.");

			data = {};
			functionSet.getVertexPositions(data.positions);
			functionSet.getVertexColors(data.vertexColors);
			functionSet.getEdgeColors(data.edgeColors);

			for (zItGraphEdge edge(graph); !edge.end(); edge++)
			{
				if (!edge.isActive()) continue;
				zIntArray vertices;
				edge.getVertices(vertices);
				if (vertices.size() != 2)
					return zIOResult::error("Graph contains an edge without two vertices.");
				data.edgeConnects.push_back(vertices[0]);
				data.edgeConnects.push_back(vertices[1]);
			}

			if (data.positions.empty() || data.edgeConnects.empty())
				return zIOResult::error("Graph contains no writable geometry.");
			return zIOResult::ok();
		}
	}

	zIOResult zIO::readMesh(const std::string& path, zObjectMesh& mesh, bool staticGeometry)
	{
		io_detail::MeshData data;
		const std::string extension = extensionOf(path);

		zIOResult result;
		if (extension == ".obj") result = io_detail::readOBJ(path, data);
		else if (extension == ".json") result = io_detail::readMeshJSON(path, data);
		else if (isUSD(extension)) return io_detail::usdUnavailable();
		else return zIOResult::error("Unsupported mesh file extension: " + extension);

		if (!result) return result;
		return applyMeshData(data, mesh, staticGeometry);
	}

	zIOResult zIO::writeMesh(const std::string& path, zObjectMesh& mesh)
	{
		io_detail::MeshData data;
		auto result = extractMeshData(mesh, data);
		if (!result) return result;

		const std::string extension = extensionOf(path);
		if (extension == ".obj") return io_detail::writeOBJ(path, data);
		if (extension == ".json") return io_detail::writeMeshJSON(path, data);
		if (isUSD(extension)) return io_detail::usdUnavailable();
		return zIOResult::error("Unsupported mesh file extension: " + extension);
	}

	zIOResult zIO::readGraph(const std::string& path, zObjectGraph& graph, bool staticGeometry)
	{
		io_detail::GraphData data;
		const std::string extension = extensionOf(path);

		zIOResult result;
		if (extension == ".txt") result = io_detail::readGraphTXT(path, data);
		else if (extension == ".json") result = io_detail::readGraphJSON(path, data);
		else if (isUSD(extension)) return io_detail::usdUnavailable();
		else return zIOResult::error("Unsupported graph file extension: " + extension);

		if (!result) return result;
		return applyGraphData(data, graph, staticGeometry);
	}

	zIOResult zIO::writeGraph(const std::string& path, zObjectGraph& graph)
	{
		io_detail::GraphData data;
		auto result = extractGraphData(graph, data);
		if (!result) return result;

		const std::string extension = extensionOf(path);
		if (extension == ".txt") return io_detail::writeGraphTXT(path, data);
		if (extension == ".json") return io_detail::writeGraphJSON(path, data);
		if (isUSD(extension)) return io_detail::usdUnavailable();
		return zIOResult::error("Unsupported graph file extension: " + extension);
	}

	zIOResult zIO::readPointCloud(const std::string& path, zObjectPointCloud& points)
	{
		const std::string extension = extensionOf(path);
		if (extension != ".csv")
			return zIOResult::error("Unsupported point-cloud file extension: " + extension);

		io_detail::PointCloudData data;
		auto result = io_detail::readPointCloudCSV(path, data);
		if (!result) return result;

		zFnPointCloud functionSet(points);
		functionSet.create(data.positions);
		if (data.colors.size() == data.positions.size())
			functionSet.setVertexColors(data.colors);
		return zIOResult::ok();
	}

	zIOResult zIO::writePointCloud(const std::string& path, zObjectPointCloud& points)
	{
		const std::string extension = extensionOf(path);
		if (extension != ".csv")
			return zIOResult::error("Unsupported point-cloud file extension: " + extension);

		io_detail::PointCloudData data;
		zFnPointCloud functionSet(points);
		functionSet.getVertexPositions(data.positions);
		functionSet.getVertexColors(data.colors);
		if (data.positions.empty())
			return zIOResult::error("Point cloud contains no writable geometry.");
		return io_detail::writePointCloudCSV(path, data);
	}
}
