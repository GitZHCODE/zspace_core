#include <zspace/io.h>
#include <depends/nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
	using namespace zSpace;

	void require(bool condition, const char* message)
	{
		if (!condition) throw std::runtime_error(message);
	}

	void requireSuccess(const zIOResult& result)
	{
		if (!result) throw std::runtime_error(result.message());
	}

	void testMesh(const std::filesystem::path& directory)
	{
		zObjectMesh source;
		zFnMesh sourceFn(source);
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(1, 1, 0),
			zPoint(0, 1, 0)
		};
		zIntArray polygonCounts = { 4 };
		zIntArray polygonConnects = { 0, 1, 2, 3 };
		sourceFn.create(positions, polygonCounts, polygonConnects);
		zColorArray edgeColors = {
			zColor(1, 0, 0, 1),
			zColor(0, 1, 0, 1),
			zColor(0, 0, 1, 1),
			zColor(1, 1, 0, 1)
		};
		sourceFn.setEdgeColors(edgeColors, false);
		zDoubleArray edgeWeights = { 0.5, 1.5, 2.5, 3.5 };
		sourceFn.setEdgeWeights(edgeWeights);

		const auto objPath = directory / "mesh.obj";
		const auto jsonPath = directory / "mesh.json";
		const auto usdPath = directory / "mesh.usda";
		requireSuccess(zIO::writeMesh(objPath.string(), source));
		requireSuccess(zIO::writeMesh(jsonPath.string(), source));
		requireSuccess(zIO::writeMesh(usdPath.string(), source));

		nlohmann::json document;
		{
			std::ifstream input(jsonPath);
			input >> document;
		}
		require(document["schema"] == "zspace.mesh.v2", "mesh JSON schema version");
		require(document.contains("edgeAttributes"), "mesh JSON edge attributes");
		require(!document.contains("halfedgeAttributes"), "mesh JSON omits halfedge attributes");
		require(document["edgeAttributes"].size() == edgeColors.size(), "mesh JSON edge attribute count");
		require(document["edgeAttributes"][2].size() == 5, "mesh JSON edge attribute layout");
		require(document["edgeAttributes"][2][4].get<double>() == 2.5, "mesh JSON edge weight value");

		zObjectMesh fromObj;
		zObjectMesh fromJson;
		zObjectMesh fromUsd;
		requireSuccess(zIO::readMesh(objPath.string(), fromObj));
		requireSuccess(zIO::readMesh(jsonPath.string(), fromJson));
		requireSuccess(zIO::readMesh(usdPath.string(), fromUsd));

		zFnMesh objFn(fromObj);
		zFnMesh jsonFn(fromJson);
		zFnMesh usdFn(fromUsd);
		require(objFn.numVertices() == 4 && objFn.numPolygons() == 1, "OBJ mesh round trip");
		require(jsonFn.numVertices() == 4 && jsonFn.numPolygons() == 1, "JSON mesh round trip");
		require(usdFn.numVertices() == 4 && usdFn.numPolygons() == 1, "USD mesh round trip");

		zColorArray roundTripEdgeColors;
		jsonFn.getEdgeColors(roundTripEdgeColors);
		require(roundTripEdgeColors.size() == edgeColors.size(), "JSON mesh edge color count");
		require(roundTripEdgeColors[2].b == 1.0f, "JSON mesh edge color value");

		zDoubleArray roundTripEdgeWeights;
		jsonFn.getEdgeWeights(roundTripEdgeWeights);
		require(roundTripEdgeWeights.size() == edgeWeights.size(), "JSON mesh edge weight count");
		require(roundTripEdgeWeights[2] == 2.5, "JSON mesh edge weight round trip");

		zColorArray usdEdgeColors;
		usdFn.getEdgeColors(usdEdgeColors);
		require(usdEdgeColors.size() == edgeColors.size(), "USD mesh edge color count");
		require(usdEdgeColors[2].b == 1.0f, "USD mesh edge color round trip");

		zDoubleArray usdEdgeWeights;
		usdFn.getEdgeWeights(usdEdgeWeights);
		require(usdEdgeWeights.size() == edgeWeights.size(), "USD mesh edge weight count");
		require(usdEdgeWeights[2] == 2.5, "USD mesh edge weight round trip");

		std::ifstream usdInput(usdPath);
		const std::string usdText(
			(std::istreambuf_iterator<char>(usdInput)),
			std::istreambuf_iterator<char>());
		require(usdText.find("customLayerData") != std::string::npos, "USD writes layer metadata");
		require(usdText.find("primvars:zspace:edgeColor") != std::string::npos,
			"USD writes edge color primvar");
		require(usdText.find("primvars:zspace:edgeWeight") != std::string::npos,
			"USD writes edge weight primvar");
		require(!zIO::writeMesh((directory / "mesh.usdc").string(), source),
			"USD binary writing reports unsupported");

		std::ifstream objInput(objPath);
		const std::string objText(
			(std::istreambuf_iterator<char>(objInput)),
			std::istreambuf_iterator<char>());
		require(objText.find("\nvn ") != std::string::npos, "OBJ writes normals");
		require(objText.find("//1") != std::string::npos, "OBJ faces reference normals");
	}

	void testExternalOBJ(const std::filesystem::path& directory)
	{
		const auto path = directory / "external.obj";
		{
			std::ofstream output(path);
			output
				<< "# OBJ syntax commonly emitted by DCC software\n"
				<< "o ExternalMesh\n"
				<< "v 0 0 0 1 0 0\n"
				<< "v 1 0 0 0 1 0\n"
				<< "v 1 1 0 0 0 1\n"
				<< "v 0 1 0 1 1 1\n"
				<< "vt 0 0\n"
				<< "vt 1 0\n"
				<< "vt 1 1\n"
				<< "vt 0 1\n"
				<< "vn 0 0 1\n"
				<< "f -4/1/1 -3/2/1 -2/3/1 -1/4/1 # negative vertex indices\n";
		}

		zObjectMesh mesh;
		requireSuccess(zIO::readMesh(path.string(), mesh));
		zFnMesh functionSet(mesh);
		require(functionSet.numVertices() == 4, "external OBJ vertex count");
		require(functionSet.numPolygons() == 1, "external OBJ polygon count");

		zVectorArray normals;
		functionSet.getFaceNormals(normals);
		require(normals.size() == 1 && normals[0].z > 0.99f, "external OBJ normal import");
	}

	void testGraph(const std::filesystem::path& directory)
	{
		zObjectGraph source;
		zFnGraph sourceFn(source);
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(1, 1, 0)
		};
		zIntArray edgeConnects = { 0, 1, 1, 2 };
		sourceFn.create(positions, edgeConnects);

		const auto txtPath = directory / "graph.txt";
		const auto jsonPath = directory / "graph.json";
		requireSuccess(zIO::writeGraph(txtPath.string(), source));
		requireSuccess(zIO::writeGraph(jsonPath.string(), source));

		zObjectGraph fromTxt;
		zObjectGraph fromJson;
		requireSuccess(zIO::readGraph(txtPath.string(), fromTxt));
		requireSuccess(zIO::readGraph(jsonPath.string(), fromJson));

		zFnGraph txtFn(fromTxt);
		zFnGraph jsonFn(fromJson);
		require(txtFn.numVertices() == 3 && txtFn.numEdges() == 2, "TXT graph round trip");
		require(jsonFn.numVertices() == 3 && jsonFn.numEdges() == 2, "JSON graph round trip");
	}

	void testPointCloud(const std::filesystem::path& directory)
	{
		zObjectPointCloud source;
		zFnPointCloud sourceFn(source);
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 2, 3)
		};
		sourceFn.create(positions);

		zColorArray colors = {
			zColor(1, 0, 0, 1),
			zColor(0, 0.5f, 1, 0.75f)
		};
		sourceFn.setVertexColors(colors);

		const auto csvPath = directory / "points.csv";
		requireSuccess(zIO::writePointCloud(csvPath.string(), source));

		zObjectPointCloud fromCsv;
		requireSuccess(zIO::readPointCloud(csvPath.string(), fromCsv));

		zFnPointCloud csvFn(fromCsv);
		require(csvFn.numVertices() == 2, "CSV point-cloud round trip");

		zPointArray roundTripPositions;
		csvFn.getVertexPositions(roundTripPositions);
		require(roundTripPositions.size() == positions.size(), "CSV point-cloud position count");
		require(roundTripPositions[1].x == 1.0f && roundTripPositions[1].z == 3.0f, "CSV point-cloud position value");

		zColorArray roundTripColors;
		csvFn.getVertexColors(roundTripColors);
		require(roundTripColors.size() == colors.size(), "CSV point-cloud color count");
		require(roundTripColors[1].g == 0.5f && roundTripColors[1].a == 0.75f, "CSV point-cloud color value");
	}
}

int main()
{
	try
	{
		const auto directory = std::filesystem::temp_directory_path() / "zspace_io_smoke";
		std::filesystem::create_directories(directory);

		testMesh(directory);
		testExternalOBJ(directory);
		testGraph(directory);
		testPointCloud(directory);

		std::filesystem::remove_all(directory);
		std::cout << "zspace IO smoke tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "zspace IO smoke tests failed: " << error.what() << '\n';
		return 1;
	}
}
