#include <zspace/zIO/zUSDapi.h>
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
			zColor(0, 0, 1, 0.5f),
			zColor(1, 1, 0, 1)
		};
		sourceFn.setEdgeColors(edgeColors, false);
		zDoubleArray edgeWeights = { 0.5, 1.5, 2.5, 3.5 };
		sourceFn.setEdgeWeights(edgeWeights);
		zColorArray faceColors = { zColor(0.2f, 0.4f, 0.6f, 0.25f) };
		sourceFn.setFaceColors(faceColors);

		const auto objPath = directory / "mesh.obj";
		const auto jsonPath = directory / "mesh.json";
		const auto usdPath = directory / "mesh.usda";
		requireSuccess(zIO::writeMesh(objPath.string(), source));
		requireSuccess(zIO::writeMesh(jsonPath.string(), source));
#if defined(ZSPACE_TEST_OPENUSD)
		requireSuccess(zIO::writeMesh(usdPath.string(), source));
#else
		require(!zIO::writeMesh(usdPath.string(), source), "disabled USD reports unsupported");
#endif

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
#if defined(ZSPACE_TEST_OPENUSD)
		requireSuccess(zIO::readMesh(usdPath.string(), fromUsd));
#endif

		zFnMesh objFn(fromObj);
		zFnMesh jsonFn(fromJson);
		zFnMesh usdFn(fromUsd);
		require(objFn.numVertices() == 4 && objFn.numPolygons() == 1, "OBJ mesh round trip");
		require(jsonFn.numVertices() == 4 && jsonFn.numPolygons() == 1, "JSON mesh round trip");
#if defined(ZSPACE_TEST_OPENUSD)
		require(usdFn.numVertices() == 4 && usdFn.numPolygons() == 1, "USD mesh round trip");
#endif

		zColorArray roundTripEdgeColors;
		jsonFn.getEdgeColors(roundTripEdgeColors);
		require(roundTripEdgeColors.size() == edgeColors.size(), "JSON mesh edge color count");
		require(roundTripEdgeColors[2].b == 1.0f, "JSON mesh edge color value");

		zDoubleArray roundTripEdgeWeights;
		jsonFn.getEdgeWeights(roundTripEdgeWeights);
		require(roundTripEdgeWeights.size() == edgeWeights.size(), "JSON mesh edge weight count");
		require(roundTripEdgeWeights[2] == 2.5, "JSON mesh edge weight round trip");

#if defined(ZSPACE_TEST_OPENUSD)
		zColorArray usdEdgeColors;
		usdFn.getEdgeColors(usdEdgeColors);
		require(usdEdgeColors.size() == edgeColors.size(), "USD mesh edge color count");
		require(usdEdgeColors[2].b == 1.0f, "USD mesh edge color round trip");
		require(usdEdgeColors[2].a == 0.5f, "USD edge opacity round trip");
		zColorArray usdFaceColors;
		usdFn.getFaceColors(usdFaceColors);
		require(usdFaceColors.size() == 1 && usdFaceColors[0].a == 0.25f, "USD face opacity round trip");

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
		require(usdText.find("subdivisionScheme = \"none\"") != std::string::npos, "USD exports polygon mesh semantics");
		require(usdText.find("normal3f[] normals") != std::string::npos, "USD exports standard normals");
		for (const auto* extension : { ".usd", ".usdc", ".usdz" })
		{
			const auto binaryPath = directory / (std::string("mesh") + extension);
			requireSuccess(zIO::writeMesh(binaryPath.string(), source));
			zObjectMesh restored;
			requireSuccess(zIO::readMesh(binaryPath.string(), restored));
			zFnMesh restoredFn(restored);
			require(restoredFn.numVertices() == 4 && restoredFn.numPolygons() == 1, "USD format round trip");
			zColorArray restoredColors;
			restoredFn.getEdgeColors(restoredColors);
			require(restoredColors.size() == 4 && restoredColors[2].a == 0.5f, "USD binary edge opacity");
			if (std::string(extension) == ".usd")
			{
				std::ifstream textFile(binaryPath);
				std::string header;
				std::getline(textFile, header);
				require(header == "#usda 1.0", ".usd preserves text writing");
			}
		}
#endif

		std::ifstream objInput(objPath);
		const std::string objText(
			(std::istreambuf_iterator<char>(objInput)),
			std::istreambuf_iterator<char>());
		require(objText.find("\nvn ") != std::string::npos, "OBJ writes normals");
		require(objText.find("//1") != std::string::npos, "OBJ faces reference normals");
	}

#if defined(ZSPACE_TEST_OPENUSD)
	void testExternalUSD(const std::filesystem::path& directory)
	{
		const auto asset = directory / "asset.usda";
		{
			std::ofstream output(asset);
			output << R"USD(#usda 1.0
( defaultPrim = "Asset" )
def Xform "Asset" {
    double3 xformOp:translate = (100, 200, 300)
    uniform token[] xformOpOrder = ["xformOp:translate"]
    def Mesh "First" {
        point3f[] points = [(0,0,0), (1,0,0), (0,1,0)]
        int[] faceVertexCounts = [3]
        int[] faceVertexIndices = [0,1,2]
        color3f[] primvars:displayColor = [(1,0,0), (0,1,0)] ( interpolation = "vertex" )
        int[] primvars:displayColor:indices = [1,0,1]
        float[] primvars:displayOpacity = [0.5] ( interpolation = "constant" )
        color3f[] primvars:zspace:faceColor = [(0,0,1)] ( interpolation = "uniform" )
        float[] primvars:zspace:faceColorOpacity = [0.25] ( interpolation = "uniform" )
        normal3f[] primvars:normals = [(0,0,1)] ( interpolation = "uniform" )
    }
    def Mesh "Second" {
        point3f[] points = [(10,0,0), (11,0,0), (10,1,0)]
        int[] faceVertexCounts = [3]
        int[] faceVertexIndices = [0,1,2]
    }
}
)USD";
		}
		const auto stage = directory / "reference.usda";
		{
			std::ofstream output(stage);
			output << "#usda 1.0\ndef Xform \"Reference\" ( prepend references = @asset.usda@ ) {}\n";
		}
		zObjectMesh mesh;
		requireSuccess(zIO::readMesh(stage.string(), mesh));
		zFnMesh fn(mesh);
		zPointArray positions;
		fn.getVertexPositions(positions);
		require(positions.size() == 3 && positions[0].x == 0 && positions[0].y == 0,
			"composed USD selects first mesh in object space");
		zColorArray colors;
		fn.getVertexColors(colors);
		require(colors.size() == 3 && colors[0].g == 1 && colors[1].r == 1 && colors[0].a == 0.5f,
			"indexed USD colors and constant opacity");
		fn.getFaceColors(colors);
		require(colors.size() == 1 && colors[0].a == 0.25f, "legacy TinyUSDZ opacity names remain readable");
		zVectorArray normals;
		fn.getFaceNormals(normals);
		require(normals.size() == 1 && normals[0].z == 1, "legacy normals primvar remains readable");
		const auto invalid = directory / "invalid.usda";
		{
			std::ofstream output(invalid);
			output << "#usda 1.0\ndef Mesh \"Invalid\" {\npoint3f[] points = [(0,0,0)]\n"
				<< "int[] faceVertexCounts = [3]\nint[] faceVertexIndices = [0,1,2]\n}\n";
		}
		require(!zIO::readMesh(invalid.string(), mesh), "invalid USD topology returns an error");
		require(fn.numVertices() == 3, "failed USD read preserves destination");
	}
#endif

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

	void testNonManifoldMeshIO(const std::filesystem::path& directory)
	{
		zObjectMesh source;
		zFnMesh sourceFn(source);
		zPointArray positions = {
			zPoint(0, 0, 0), zPoint(1, 0, 0), zPoint(0.5, 1, 0),
			zPoint(0.5, -1, 0), zPoint(0.5, 0, 1)
		};
		zIntArray counts = { 3, 3, 3 };
		zIntArray connects = { 0, 1, 2, 1, 0, 3, 0, 1, 4 };
		sourceFn.create(positions, counts, connects);

		const auto path = directory / "nonmanifold.json";
		requireSuccess(zIO::writeMesh(path.string(), source));

		zObjectMesh restored;
		requireSuccess(zIO::readMesh(path.string(), restored));
		zFnMesh restoredFn(restored);
		require(restoredFn.numVertices() == 5, "non-manifold IO vertex count");
		require(restoredFn.numPolygons() == 3, "non-manifold IO polygon count");
		require(restoredFn.numEdges() == 7, "non-manifold IO edge count");
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

#if defined(ZSPACE_TEST_OPENUSD)
void testUSDObjectAPI(const std::filesystem::path &directory) {
  using namespace zSpace;
  const auto source = directory / "objects.usda";
  { std::ofstream file(source); file << R"(#usda 1.0
def Mesh "Block" {
  point3f[] points = [(1,2,3)]
  double3 xformOp:translate = (10,0,0)
  uniform token[] xformOpOrder = ["xformOp:translate"]
  float[] primvars:test = [10,20] (interpolation = "vertex")
  int[] primvars:test:indices = [1,0,1]
}
)"; }
  USDdoc doc; doc.open(source.string());
  auto prim = doc.getPrimAtPath("/Block");
  require(prim.isValid() && prim.getTypeName() == "Mesh", "object prim lookup");
  auto points = prim.getAttribute("points");
  require(points.get<std::vector<float>>()[0] == 1, "raw points remain local");
  require(doc.readWorldPoints("/Block")[0][0] == 11, "explicit world transform");
  require(prim.getAttribute("primvars:test").get<std::vector<float>>().size() == 2, "raw primvar is not flattened");
  require(!prim.getAttribute("missing").isValid(), "missing attribute validity");
  auto output = doc.definePrim("/Output", "Scope");
  auto width = output.createAttribute("width", "float");
  require(width.getTypeName() == "float", "unvalued attribute type");
  width.set(2.5f); require(width.get<float>() == 2.5f, "direct float set/get");
  bool rejected = false;
  try { width.set(std::string("bad")); } catch (...) { rejected = true; }
  require(rejected && width.get<float>() == 2.5f, "type mismatch preserves value");
  output.createAttribute("normals", "normal3f[]").set(std::vector<float>{0,0,1});
  const auto saved = directory / "objects-out.usda"; doc.save(saved.string());
  doc.open(saved.string());
  require(!width.isValid() && !prim.isValid(), "reopen invalidates old handles");
  auto reopened = doc.getPrimAtPath("/Output").getAttribute("width");
  require(reopened.get<float>() == 2.5f, "roundtrip direct edits");
  doc.close(); require(!reopened.isValid(), "close invalidates handles");
}
#endif

int main()
{
	try
	{
		const auto directory = std::filesystem::temp_directory_path() / "zspace_io_smoke";
		std::filesystem::create_directories(directory);

		testMesh(directory);
#if defined(ZSPACE_TEST_OPENUSD)
		testExternalUSD(directory);
        testUSDObjectAPI(directory);
#endif
		testExternalOBJ(directory);
		testNonManifoldMeshIO(directory);
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
