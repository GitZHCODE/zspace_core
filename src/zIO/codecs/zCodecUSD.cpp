#include <src/zIO/codecs/zCodecUSD.h>

#if defined(ZSPACE_IO_TINYUSDZ)
#include <tinyusdz.hh>
#include <usdGeom.hh>
#include <usda-writer.hh>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <vector>
#endif

namespace zSpace::io_detail
{
#if defined(ZSPACE_IO_TINYUSDZ)
	namespace
	{
		const tinyusdz::GeomMesh* findFirstMesh(const std::vector<tinyusdz::Prim>& prims)
		{
			for (const auto& prim : prims)
			{
				if (const auto* mesh = prim.as<tinyusdz::GeomMesh>()) return mesh;
				if (const auto* mesh = findFirstMesh(prim.children())) return mesh;
			}
			return nullptr;
		}

		template <typename T>
		bool readPrimvar(const tinyusdz::GeomMesh& mesh, const std::string& name,
			std::vector<T>& values)
		{
			tinyusdz::GeomPrimvar primvar;
			std::string error;
			return mesh.get_primvar(name, &primvar, &error) &&
				primvar.get_value(&values, &error);
		}

		template <typename T>
		bool addPrimvar(tinyusdz::GeomMesh& mesh, const std::string& name,
			const std::vector<T>& values, tinyusdz::Interpolation interpolation,
			std::string& error, uint32_t elementSize = 0)
		{
			if (values.empty()) return true;
			tinyusdz::GeomPrimvar primvar;
			primvar.set_name(name);
			primvar.set_value(values);
			primvar.set_interpolation(interpolation);
			if (elementSize > 0) primvar.set_elementSize(elementSize);
			return mesh.set_primvar(primvar, &error);
		}

		std::string formatMessage(const std::string& prefix, const std::string& warning,
			const std::string& error)
		{
			std::string message = prefix;
			if (!error.empty()) message += " " + error;
			if (!warning.empty()) message += " Warning: " + warning;
			return message;
		}
	}

	zIOResult readMeshUSD(const std::string& path, MeshData& data)
	{
		tinyusdz::Stage stage;
		std::string warning;
		std::string error;
		if (!tinyusdz::LoadUSDFromFile(path, &stage, &warning, &error))
			return zIOResult::error(formatMessage("Could not read USD mesh.", warning, error));

		const auto* mesh = findFirstMesh(stage.root_prims());
		if (!mesh) return zIOResult::error("USD stage contains no mesh prim.");

		data = {};
		for (const auto& point : mesh->get_points())
			data.positions.emplace_back(point.x, point.y, point.z);

		const auto counts = mesh->get_faceVertexCounts();
		data.polygonCounts.assign(counts.begin(), counts.end());
		const auto indices = mesh->get_faceVertexIndices();
		data.polygonConnects.assign(indices.begin(), indices.end());

		const auto normals = mesh->get_normals();
		if (mesh->get_normalsInterpolation() == tinyusdz::Interpolation::Uniform)
		{
			for (const auto& normal : normals)
				data.faceNormals.emplace_back(normal.x, normal.y, normal.z);
		}

		std::vector<tinyusdz::value::color3f> vertexColors;
		std::vector<float> vertexOpacity;
		if (readPrimvar(*mesh, "displayColor", vertexColors))
		{
			readPrimvar(*mesh, "displayOpacity", vertexOpacity);
			for (std::size_t i = 0; i < vertexColors.size(); ++i)
			{
				const float alpha = i < vertexOpacity.size() ? vertexOpacity[i] : 1.0f;
				const auto& color = vertexColors[i];
				data.vertexColors.emplace_back(color.r, color.g, color.b, alpha);
			}
		}

		std::vector<tinyusdz::value::color3f> faceColors;
		std::vector<float> faceOpacity;
		if (readPrimvar(*mesh, "zspace:faceColor", faceColors))
		{
			readPrimvar(*mesh, "zspace:faceOpacity", faceOpacity);
			for (std::size_t i = 0; i < faceColors.size(); ++i)
			{
				const float alpha = i < faceOpacity.size() ? faceOpacity[i] : 1.0f;
				const auto& color = faceColors[i];
				data.faceColors.emplace_back(color.r, color.g, color.b, alpha);
			}
		}

		std::vector<int32_t> edgeConnects;
		std::vector<tinyusdz::value::color3f> edgeColors;
		std::vector<float> edgeOpacity;
		std::vector<float> edgeWeights;
		if (readPrimvar(*mesh, "zspace:edgeVertexIndices", edgeConnects))
			data.edgeConnects.assign(edgeConnects.begin(), edgeConnects.end());
		readPrimvar(*mesh, "zspace:edgeColor", edgeColors);
		readPrimvar(*mesh, "zspace:edgeOpacity", edgeOpacity);
		readPrimvar(*mesh, "zspace:edgeWeight", edgeWeights);
		for (std::size_t i = 0; i < edgeColors.size(); ++i)
		{
			const float alpha = i < edgeOpacity.size() ? edgeOpacity[i] : 1.0f;
			const auto& color = edgeColors[i];
			data.edgeColors.emplace_back(color.r, color.g, color.b, alpha);
		}
		data.edgeWeights.assign(edgeWeights.begin(), edgeWeights.end());

		return zIOResult::ok();
	}

	zIOResult writeMeshUSD(const std::string& path, const MeshData& data)
	{
		std::string extension = std::filesystem::path(path).extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		if (extension == ".usdc" || extension == ".usdz")
			return zIOResult::error("TinyUSDZ binary USD writing is experimental; write .usda or .usd instead.");

		tinyusdz::GeomMesh mesh;
		mesh.name = "Mesh";

		std::vector<tinyusdz::value::point3f> points;
		points.reserve(data.positions.size());
		for (const auto& point : data.positions)
			points.push_back({ point.x, point.y, point.z });
		mesh.points.set_value(points);

		std::vector<int32_t> counts(data.polygonCounts.begin(), data.polygonCounts.end());
		std::vector<int32_t> indices(data.polygonConnects.begin(), data.polygonConnects.end());
		mesh.faceVertexCounts.set_value(counts);
		mesh.faceVertexIndices.set_value(indices);

		std::string error;
		std::vector<tinyusdz::value::normal3f> normals;
		for (const auto& normal : data.faceNormals)
			normals.push_back({ normal.x, normal.y, normal.z });
		if (!addPrimvar(mesh, "normals", normals, tinyusdz::Interpolation::Uniform, error))
			return zIOResult::error("Could not add USD normals primvar. " + error);

		auto addColors = [&](const std::string& name, const zColorArray& colors,
			tinyusdz::Interpolation interpolation) -> bool
		{
			std::vector<tinyusdz::value::color3f> rgb;
			std::vector<float> opacity;
			for (const auto& color : colors)
			{
				rgb.push_back({ color.r, color.g, color.b });
				opacity.push_back(color.a);
			}
			return addPrimvar(mesh, name, rgb, interpolation, error) &&
				addPrimvar(mesh, name == "displayColor" ? "displayOpacity" : name + "Opacity",
					opacity, interpolation, error);
		};

		if (!addColors("displayColor", data.vertexColors, tinyusdz::Interpolation::Vertex))
			return zIOResult::error("Could not add USD vertex color primvars. " + error);
		if (!addColors("zspace:faceColor", data.faceColors, tinyusdz::Interpolation::Uniform))
			return zIOResult::error("Could not add USD face color primvars. " + error);
		if (!addColors("zspace:edgeColor", data.edgeColors, tinyusdz::Interpolation::Constant))
			return zIOResult::error("Could not add USD edge color primvars. " + error);

		std::vector<int32_t> edgeConnects(data.edgeConnects.begin(), data.edgeConnects.end());
		if (!addPrimvar(mesh, "zspace:edgeVertexIndices", edgeConnects,
			tinyusdz::Interpolation::Constant, error, 2))
			return zIOResult::error("Could not add USD edge topology primvar. " + error);

		std::vector<float> edgeWeights;
		edgeWeights.reserve(data.edgeWeights.size());
		for (double weight : data.edgeWeights)
			edgeWeights.push_back(static_cast<float>(weight));
		if (!addPrimvar(mesh, "zspace:edgeWeight", edgeWeights,
			tinyusdz::Interpolation::Constant, error))
			return zIOResult::error("Could not add USD edge weight primvar. " + error);

		tinyusdz::Stage stage;
		stage.metas().comment = "Generated by zSpace_IO with TinyUSDZ.";
		stage.metas().metersPerUnit = 0.01;
		stage.metas().upAxis = tinyusdz::Axis::Z;
		tinyusdz::Dictionary customData;
		customData.emplace("generator", std::string("zSpace_IO"));
		customData.emplace("schema", std::string("zspace.mesh.v1"));
		stage.metas().customLayerData = customData;
		stage.root_prims().emplace_back(std::move(mesh));
		if (!stage.commit())
			return zIOResult::error("Could not commit USD stage. " + stage.get_error());

		std::string warning;
		if (!tinyusdz::usda::SaveAsUSDA(path, stage, &warning, &error))
			return zIOResult::error(formatMessage("Could not write USD mesh.", warning, error));
		return zIOResult::ok();
	}
#else
	zIOResult readMeshUSD(const std::string&, MeshData&)
	{
		return zIOResult::error("USD support is disabled. Configure with ZSPACE_IO_WITH_TINYUSDZ=ON.");
	}

	zIOResult writeMeshUSD(const std::string&, const MeshData&)
	{
		return zIOResult::error("USD support is disabled. Configure with ZSPACE_IO_WITH_TINYUSDZ=ON.");
	}
#endif
}
