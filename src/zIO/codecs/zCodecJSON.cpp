#include <src/zIO/codecs/zCodecJSON.h>

#include <depends/nlohmann/json.hpp>

#include <fstream>
#include <unordered_set>
#include <utility>

namespace zSpace::io_detail
{
	using Json = nlohmann::json;

	namespace
	{
		zIOResult readDocument(const std::string& path, Json& document)
		{
			std::ifstream input(path);
			if (!input) return zIOResult::error("Could not open JSON file: " + path);

			try
			{
				input >> document;
				return zIOResult::ok();
			}
			catch (const std::exception& error)
			{
				return zIOResult::error("Invalid JSON in " + path + ": " + error.what());
			}
		}

		zIOResult writeDocument(const std::string& path, const Json& document)
		{
			std::ofstream output(path);
			if (!output) return zIOResult::error("Could not create JSON file: " + path);
			output << document.dump(2);
			return output ? zIOResult::ok() : zIOResult::error("Failed while writing JSON file: " + path);
		}

		Json pointsToJson(const zPointArray& points)
		{
			Json values = Json::array();
			for (const auto& point : points) values.push_back({ point.x, point.y, point.z });
			return values;
		}

		Json colorsToJson(const zColorArray& colors)
		{
			Json values = Json::array();
			for (const auto& color : colors) values.push_back({ color.r, color.g, color.b, color.a });
			return values;
		}

		void meshEdgesFromAttributes(
			const std::vector<zDoubleArray>& attributes,
			zColorArray& colors,
			zDoubleArray& weights)
		{
			colors.clear();
			weights.clear();
			for (const auto& value : attributes)
			{
				if (value.size() < 3) continue;
				const float alpha = value.size() > 3 ? static_cast<float>(value[3]) : 1.0f;
				colors.emplace_back(
					static_cast<float>(value[0]),
					static_cast<float>(value[1]),
					static_cast<float>(value[2]),
					alpha);
				weights.push_back(value.size() > 4 ? value[4] : 1.0);
			}
		}

		void meshEdgesFromLegacyHalfedgeAttributes(
			const std::vector<zDoubleArray>& attributes,
			zColorArray& colors,
			zDoubleArray& weights)
		{
			std::vector<zDoubleArray> edgeAttributes;
			for (std::size_t i = 0; i < attributes.size(); i += 2)
			{
				zDoubleArray value = attributes[i];
				if (value.size() < 4) value.push_back(1.0);
				if (value.size() < 5) value.push_back(1.0);
				edgeAttributes.push_back(std::move(value));
			}
			meshEdgesFromAttributes(edgeAttributes, colors, weights);
		}

		bool readPoints(const Json& values, zPointArray& points)
		{
			if (!values.is_array()) return false;
			points.clear();
			for (const auto& value : values)
			{
				if (!value.is_array() || value.size() < 3) return false;
				points.emplace_back(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
			}
			return true;
		}

		bool readColors(const Json& values, zColorArray& colors)
		{
			if (!values.is_array()) return false;
			colors.clear();
			for (const auto& value : values)
			{
				if (!value.is_array() || value.size() < 3) return false;
				const float alpha = value.size() > 3 ? value[3].get<float>() : 1.0f;
				colors.emplace_back(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), alpha);
			}
			return true;
		}

		bool readLegacyPositions(const Json& document, zPointArray& positions, zColorArray& colors)
		{
			auto attribute = document.find("VertexAttributes");
			if (attribute == document.end() || !attribute->is_array()) return false;

			positions.clear();
			colors.clear();
			for (const auto& value : *attribute)
			{
				if (!value.is_array() || value.size() < 3) return false;
				positions.emplace_back(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());

				if (value.size() >= 9)
					colors.emplace_back(value[6].get<float>(), value[7].get<float>(), value[8].get<float>(), 1.0f);
				else if (value.size() >= 6)
					colors.emplace_back(value[3].get<float>(), value[4].get<float>(), value[5].get<float>(), 1.0f);
			}
			return true;
		}

		zIOResult readLegacyMesh(const Json& document, MeshData& data)
		{
			if (!readLegacyPositions(document, data.positions, data.vertexColors))
				return zIOResult::error("Legacy mesh JSON has invalid VertexAttributes.");

			if (!document.contains("Halfedges") || !document.contains("Faces"))
				return zIOResult::error("Legacy mesh JSON is missing Halfedges or Faces.");

			const auto halfedges = document["Halfedges"].get<std::vector<std::vector<int>>>();
			const auto faces = document["Faces"].get<std::vector<int>>();

			for (int start : faces)
			{
				if (start < 0 || start >= static_cast<int>(halfedges.size()))
					return zIOResult::error("Legacy mesh JSON contains an invalid face halfedge.");

				int current = start;
				int count = 0;
				std::unordered_set<int> visited;
				while (visited.insert(current).second)
				{
					if (current < 0 || current >= static_cast<int>(halfedges.size()) || halfedges[current].size() < 3)
						return zIOResult::error("Legacy mesh JSON contains invalid halfedge connectivity.");

					const int vertex = halfedges[current][2];
					if (vertex < 0 || vertex >= static_cast<int>(data.positions.size()))
						return zIOResult::error("Legacy mesh JSON contains an invalid vertex reference.");

					data.polygonConnects.push_back(vertex);
					++count;
					current = halfedges[current][1];
					if (current == start) break;
				}

				if (current != start || count < 3)
					return zIOResult::error("Legacy mesh JSON contains an open or invalid face loop.");
				data.polygonCounts.push_back(count);
			}

			if (document.contains("FaceAttributes"))
			{
				for (const auto& value : document["FaceAttributes"])
					if (value.is_array() && value.size() >= 6)
						data.faceColors.emplace_back(value[3].get<float>(), value[4].get<float>(), value[5].get<float>(), 1.0f);
			}

			if (document.contains("HalfedgeAttributes"))
			{
				const auto attributes =
					document["HalfedgeAttributes"].get<std::vector<zDoubleArray>>();
				meshEdgesFromLegacyHalfedgeAttributes(
					attributes,
					data.edgeColors,
					data.edgeWeights);
			}

			return zIOResult::ok();
		}

		zIOResult readLegacyGraph(const Json& document, GraphData& data)
		{
			if (!readLegacyPositions(document, data.positions, data.vertexColors))
				return zIOResult::error("Legacy graph JSON has invalid VertexAttributes.");
			if (!document.contains("Halfedges"))
				return zIOResult::error("Legacy graph JSON is missing Halfedges.");

			const auto halfedges = document["Halfedges"].get<std::vector<std::vector<int>>>();
			if (halfedges.size() % 2 != 0)
				return zIOResult::error("Legacy graph JSON contains incomplete halfedge pairs.");

			for (std::size_t i = 0; i < halfedges.size(); i += 2)
			{
				if (halfedges[i].size() < 3 || halfedges[i + 1].size() < 3)
					return zIOResult::error("Legacy graph JSON contains invalid halfedge data.");
				data.edgeConnects.push_back(halfedges[i][2]);
				data.edgeConnects.push_back(halfedges[i + 1][2]);
			}

			if (document.contains("HalfedgeAttributes"))
			{
				const auto& values = document["HalfedgeAttributes"];
				for (std::size_t i = 0; i < values.size(); i += 2)
					if (values[i].is_array() && values[i].size() >= 3)
						data.edgeColors.emplace_back(values[i][0].get<float>(), values[i][1].get<float>(), values[i][2].get<float>(), 1.0f);
			}

			return zIOResult::ok();
		}
	}

	zIOResult readMeshJSON(const std::string& path, MeshData& data)
	{
		Json document;
		auto result = readDocument(path, document);
		if (!result) return result;

		data = {};
		const std::string schema = document.value("schema", "");
		if (schema == "zspace.mesh.v1" || schema == "zspace.mesh.v2")
		{
			if (!document.contains("positions") || !readPoints(document["positions"], data.positions))
				return zIOResult::error("Mesh JSON has invalid positions.");

			try
			{
				data.polygonCounts = document.at("polygonCounts").get<zIntArray>();
				data.polygonConnects = document.at("polygonConnects").get<zIntArray>();
			}
			catch (const std::exception& error)
			{
				return zIOResult::error("Mesh JSON has invalid connectivity: " + std::string(error.what()));
			}

			if (document.contains("vertexColors") && !readColors(document["vertexColors"], data.vertexColors))
				return zIOResult::error("Mesh JSON has invalid vertex colors.");
			if (document.contains("faceColors") && !readColors(document["faceColors"], data.faceColors))
				return zIOResult::error("Mesh JSON has invalid face colors.");
			if (document.contains("edgeAttributes"))
			{
				try
				{
					data.edgeAttributes =
						document["edgeAttributes"].get<std::vector<zDoubleArray>>();
					meshEdgesFromAttributes(
						data.edgeAttributes,
						data.edgeColors,
						data.edgeWeights);
				}
				catch (const std::exception& error)
				{
					return zIOResult::error(
						"Mesh JSON has invalid edge attributes: " + std::string(error.what()));
				}
			}
			else if (document.contains("halfedgeAttributes"))
			{
				try
				{
					const auto attributes =
						document["halfedgeAttributes"].get<std::vector<zDoubleArray>>();
					meshEdgesFromLegacyHalfedgeAttributes(
						attributes,
						data.edgeColors,
						data.edgeWeights);
				}
				catch (const std::exception& error)
				{
					return zIOResult::error(
						"Mesh JSON has invalid legacy halfedge attributes: " + std::string(error.what()));
				}
			}
			return zIOResult::ok();
		}

		return readLegacyMesh(document, data);
	}

	zIOResult writeMeshJSON(const std::string& path, const MeshData& data)
	{
		Json document = {
			{ "schema", "zspace.mesh.v2" },
			{ "positions", pointsToJson(data.positions) },
			{ "polygonCounts", data.polygonCounts },
			{ "polygonConnects", data.polygonConnects }
		};
		if (!data.vertexColors.empty()) document["vertexColors"] = colorsToJson(data.vertexColors);
		if (!data.faceColors.empty()) document["faceColors"] = colorsToJson(data.faceColors);
		if (!data.edgeAttributes.empty())
			document["edgeAttributes"] = data.edgeAttributes;
		return writeDocument(path, document);
	}

	zIOResult readGraphJSON(const std::string& path, GraphData& data)
	{
		Json document;
		auto result = readDocument(path, document);
		if (!result) return result;

		data = {};
		if (document.value("schema", "") == "zspace.graph.v1")
		{
			if (!document.contains("positions") || !readPoints(document["positions"], data.positions))
				return zIOResult::error("Graph JSON has invalid positions.");

			try
			{
				data.edgeConnects = document.at("edgeConnects").get<zIntArray>();
			}
			catch (const std::exception& error)
			{
				return zIOResult::error("Graph JSON has invalid connectivity: " + std::string(error.what()));
			}

			if (document.contains("vertexColors") && !readColors(document["vertexColors"], data.vertexColors))
				return zIOResult::error("Graph JSON has invalid vertex colors.");
			if (document.contains("edgeColors") && !readColors(document["edgeColors"], data.edgeColors))
				return zIOResult::error("Graph JSON has invalid edge colors.");
			return zIOResult::ok();
		}

		return readLegacyGraph(document, data);
	}

	zIOResult writeGraphJSON(const std::string& path, const GraphData& data)
	{
		Json document = {
			{ "schema", "zspace.graph.v1" },
			{ "positions", pointsToJson(data.positions) },
			{ "edgeConnects", data.edgeConnects }
		};
		if (!data.vertexColors.empty()) document["vertexColors"] = colorsToJson(data.vertexColors);
		if (!data.edgeColors.empty()) document["edgeColors"] = colorsToJson(data.edgeColors);
		return writeDocument(path, document);
	}
}
