#pragma once

#include <chrono>
#include <cmath>
#include <cstring>
#include <sstream>

namespace
{
	zSpace::zObjectMeshScalarField gField2DObject;
	zSpace::zObjectMeshVectorField gVectorField2DObject;
	zSpace::zObjectMesh gField2DIsoMeshObject;
	zSpace::zObjectPointScalarField gField3DObject;
	zSpace::zVectorArray gField3DGradients;
	zSpace::zFieldColorType gField2DColorType = zSpace::zFieldColorType::zFieldSDF;
	float gField2DSdfWidth = 0.1f;
	zSpace::zDomainColor gField2DColorDomain(zSpace::zColor(0.08f, 0.25f, 0.95f, 1.0f), zSpace::zColor(0.95f, 0.22f, 0.12f, 1.0f));
	bool gHasField2D = false;
	bool gHasField3D = false;

	void ensureField2DExists()
	{
		if (!gHasField2D) throw std::runtime_error("No active 2D field. Create a Field2D first.");
	}

	void syncField2DMeshToRenderBuffers(zSpace::zObjectMesh& meshObj)
	{
		gActiveMeshObject = &meshObj;
		gHasMesh = true;
		copyActiveMeshToRenderBuffers();
		clearPrimitiveBuffers();
	}

	void syncField2DColorsOnlyToRenderBuffers()
	{
		const auto& data = zSpace::zMeshObjectStorage::read(gField2DObject);
		const int nF = data.numFaces();
		if (gActiveMeshObject != &gField2DObject ||
			gBuffers.positions.size() != data.positions.size() * 3 ||
			gBuffers.colors.size() != data.positions.size() * 3 ||
			gBuffers.faceCounts.size() != static_cast<std::size_t>(nF))
		{
			syncField2DMeshToRenderBuffers(gField2DObject);
			return;
		}

		if (data.vertexColors.size() == data.positions.size())
		{
			float* dst = gBuffers.colors.data();
			for (std::size_t i = 0; i < data.vertexColors.size(); ++i)
			{
				dst[i * 3] = data.vertexColors[i].r;
				dst[i * 3 + 1] = data.vertexColors[i].g;
				dst[i * 3 + 2] = data.vertexColors[i].b;
			}
		}

		if (data.faceColors.size() == static_cast<std::size_t>(nF) &&
			gBuffers.faceColors.size() == gBuffers.indices.size())
		{
			float* fDst = gBuffers.faceColors.data();
			std::size_t triOffset = 0;
			for (int f = 0; f < nF; ++f)
			{
				const int count = data.faceOffsets[f + 1] - data.faceOffsets[f];
				const int tris = std::max(0, count - 2);
				const zSpace::zColor& c = data.faceColors[f];
				for (int t = 0; t < tris; ++t)
				{
					if (triOffset + 2 < gBuffers.faceColors.size())
					{
						fDst[triOffset] = c.r;
						fDst[triOffset + 1] = c.g;
						fDst[triOffset + 2] = c.b;
					}
					triOffset += 3;
				}
			}
		}
		clearPrimitiveBuffers();
	}

	void applyAndCombineField2DScalars(
		zSpace::zFnMeshScalarField& fnField,
		zSpace::zScalarArray& newScalars,
		int combineMode,
		float sminK,
		bool normalise,
		bool syncRenderBuffers = true)
	{
		auto& current = zSpace::zFieldObjectStorage::get(gField2DObject).fieldValues;
		if (current.size() != newScalars.size() || combineMode == 0)
		{
			if (normalise) fnField.normliseValues(newScalars);
			fnField.setFieldColorDomain(gField2DColorDomain);
			fnField.setFieldValues(newScalars, gField2DColorType, gField2DSdfWidth);
		}
		else
		{
			zSpace::zScalarArray currentCopy = current;
			zSpace::zScalarArray combined;
			if (combineMode == 1)
				fnField.boolean_union(currentCopy, newScalars, combined, normalise);
			else if (combineMode == 2)
				fnField.boolean_subtract(currentCopy, newScalars, combined, normalise);
			else if (combineMode == 3)
				fnField.boolean_intersect(currentCopy, newScalars, combined, normalise);
			else if (combineMode == 4)
				fnField.boolean_difference(currentCopy, newScalars, combined, normalise);
			else if (combineMode == 5)
			{
				const float k = std::max(1.0e-5f, sminK);
				combined.resize(currentCopy.size());
				for (std::size_t i = 0; i < currentCopy.size(); ++i)
				{
					const float a = currentCopy[i];
					const float b = newScalars[i];
					const float h = std::clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
					combined[i] = (b * (1.0f - h) + a * h) - k * h * (1.0f - h);
				}
				if (normalise) fnField.normliseValues(combined);
			}
			else
				combined = std::move(newScalars);

			fnField.setFieldColorDomain(gField2DColorDomain);
			fnField.setFieldValues(combined, gField2DColorType, gField2DSdfWidth);
		}
		if (syncRenderBuffers)
		{
			syncField2DColorsOnlyToRenderBuffers();
		}
	}
}

extern "C"
{
	ZSPACE_WASM_EXPORT int zspace_field2d_create(
		float minX, float minY, float minZ,
		float maxX, float maxY, float maxZ,
		int nX, int nY,
		int valuesPerVertex, int triMesh)
	{
		try
		{
			gBuffers.lastError.clear();
			if (nX < 2 || nY < 2) throw std::runtime_error("Field2D resolution must be at least 2x2.");
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zPoint minBB(minX, minY, minZ);
			zSpace::zPoint maxBB(maxX, maxY, maxZ);
			fnField.create(minBB, maxBB, nX, nY, 1, valuesPerVertex != 0, triMesh != 0);
			fnField.setFieldColorDomain(gField2DColorDomain);
			zSpace::zScalarArray zeroScalars(static_cast<std::size_t>(fnField.numFieldValues()), 0.0f);
			fnField.setFieldValues(zeroScalars, gField2DColorType, gField2DSdfWidth);
			gHasField2D = true;
			syncField2DMeshToRenderBuffers(gField2DObject);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D create error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_show_field_mesh()
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			syncField2DMeshToRenderBuffers(gField2DObject);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D show mesh error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_set_color_mode(
		int colorType, float sdfWidth,
		float minR, float minG, float minB,
		float maxR, float maxG, float maxB)
	{
		try
		{
			gBuffers.lastError.clear();
			if (colorType == 0) gField2DColorType = zSpace::zFieldColorType::zFieldSlime;
			else if (colorType == 1) gField2DColorType = zSpace::zFieldColorType::zFieldSDF;
			else gField2DColorType = zSpace::zFieldColorType::zFieldRegular;
			gField2DSdfWidth = std::max(0.001f, sdfWidth);
			gField2DColorDomain = zSpace::zDomainColor(
				zSpace::zColor(minR, minG, minB, 1.0f),
				zSpace::zColor(maxR, maxG, maxB, 1.0f));
			if (gHasField2D)
			{
				zSpace::zFnMeshScalarField fnField(gField2DObject);
				fnField.setFieldColorDomain(gField2DColorDomain);
				fnField.updateColors(gField2DColorType, gField2DSdfWidth);
				syncField2DColorsOnlyToRenderBuffers();
			}
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D color mode error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_smooth(int numSmooth, double diffuseDamp, int diffusionType)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			fnField.getFieldValues(scalars);
			zSpace::zDiffusionType type = (diffusionType == 1) ? zSpace::zLaplacian : zSpace::zAverage;
			fnField.smoothField(scalars, std::max(1, numSmooth), diffuseDamp, type);
			fnField.setFieldColorDomain(gField2DColorDomain);
			fnField.setFieldValues(scalars, gField2DColorType, gField2DSdfWidth);
			syncField2DColorsOnlyToRenderBuffers();
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D smooth error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_isocontour(float threshold, float distTolerance)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zObjectGraph contourGraph;
			fnField.getIsocontour(contourGraph, threshold, 6, distTolerance);

			const auto& gData = zSpace::zGraphObjectStorage::read(contourGraph);
			clearMeshBuffers();
			clearPrimitiveBuffers();

			gBuffers.positions.reserve(gData.positions.size() * 3);
			gBuffers.colors.reserve(gData.positions.size() * 3);
			for (std::size_t i = 0; i < gData.positions.size(); ++i)
			{
				const auto& p = gData.positions[i];
				gBuffers.positions.push_back(p.x);
				gBuffers.positions.push_back(p.y);
				gBuffers.positions.push_back(p.z);
				const zSpace::zColor c = (i < gData.vertexColors.size()) ? gData.vertexColors[i] : zSpace::zColor(0.0f, 0.75f, 0.95f, 1.0f);
				gBuffers.colors.push_back(c.r);
				gBuffers.colors.push_back(c.g);
				gBuffers.colors.push_back(c.b);
			}

			gBuffers.edgeIndices.reserve(gData.edgeVertexIndices.size());
			gBuffers.linePositions.reserve(gData.edgeVertexIndices.size() * 3);
			gBuffers.lineColors.reserve(gData.edgeVertexIndices.size() * 3);
			gBuffers.lineWeights.reserve(gData.edgeVertexIndices.size() / 2);
			for (std::size_t i = 0; i + 1 < gData.edgeVertexIndices.size(); i += 2)
			{
				const int a = gData.edgeVertexIndices[i];
				const int b = gData.edgeVertexIndices[i + 1];
				if (a < 0 || b < 0 || static_cast<std::size_t>(a) >= gData.positions.size() || static_cast<std::size_t>(b) >= gData.positions.size()) continue;
				gBuffers.edgeIndices.push_back(static_cast<std::uint32_t>(a));
				gBuffers.edgeIndices.push_back(static_cast<std::uint32_t>(b));

				const auto& pa = gData.positions[a];
				const auto& pb = gData.positions[b];
				gBuffers.linePositions.push_back(pa.x);
				gBuffers.linePositions.push_back(pa.y);
				gBuffers.linePositions.push_back(pa.z);
				gBuffers.linePositions.push_back(pb.x);
				gBuffers.linePositions.push_back(pb.y);
				gBuffers.linePositions.push_back(pb.z);

				const zSpace::zColor ca = (static_cast<std::size_t>(a) < gData.vertexColors.size()) ? gData.vertexColors[a] : zSpace::zColor(0.0f, 0.75f, 0.95f, 1.0f);
				const zSpace::zColor cb = (static_cast<std::size_t>(b) < gData.vertexColors.size()) ? gData.vertexColors[b] : zSpace::zColor(0.0f, 0.75f, 0.95f, 1.0f);
				gBuffers.lineColors.push_back(ca.r);
				gBuffers.lineColors.push_back(ca.g);
				gBuffers.lineColors.push_back(ca.b);
				gBuffers.lineColors.push_back(cb.r);
				gBuffers.lineColors.push_back(cb.g);
				gBuffers.lineColors.push_back(cb.b);
				gBuffers.lineWeights.push_back(2.0f);
			}
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D isocontour error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_isomesh(float threshold, int invertMesh)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			fnField.getIsolineMesh(gField2DIsoMeshObject, threshold, invertMesh != 0);
			zSpace::zFnMesh(gField2DIsoMeshObject).computeMeshNormals();
			syncField2DMeshToRenderBuffers(gField2DIsoMeshObject);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D isomesh error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_isoband(float thresholdLow, float thresholdHigh)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			fnField.getIsobandMesh(gField2DIsoMeshObject, thresholdLow, thresholdHigh);
			zSpace::zFnMesh(gField2DIsoMeshObject).computeMeshNormals();
			syncField2DMeshToRenderBuffers(gField2DIsoMeshObject);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D isoband error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_vector_from_scalar(float vectorScale)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshVectorField fnVec(gVectorField2DObject);
			fnVec.createVectorFromScalarField(gField2DObject);

			clearPrimitiveBuffers();
			const auto& vecs = zSpace::zFieldObjectStorage::get(gVectorField2DObject).fieldValues;
			const auto& meshData = zSpace::zMeshObjectStorage::read(gField2DObject);
			const float scale = (vectorScale > 0.0f) ? vectorScale : 1.0f;
			const zSpace::zColor vecColor(0.05f, 0.45f, 0.95f, 1.0f);

			const std::size_t count = std::min(vecs.size(), meshData.positions.size());
			for (std::size_t i = 0; i < count; ++i)
			{
				zSpace::zVector dir = vecs[i];
				appendVectorPrimitive(meshData.positions[i], dir * scale, vecColor);
			}
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D vector from scalar error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT double zspace_field2d_sample_pair_sum(const float* queryPositions, int queryCount)
	{
		if (!gHasField2D || !queryPositions || queryCount <= 0) return 0.0;
		zSpace::zFnMeshScalarField fnField(gField2DObject);
		double accum = 0.0;
		for (int i = 0; i < queryCount; ++i)
		{
			zSpace::zPoint p(queryPositions[i * 3], queryPositions[i * 3 + 1], queryPositions[i * 3 + 2]);
			zSpace::zScalar v1 = 0.0f, v2 = 0.0f;
			fnField.getFieldValue(p, zSpace::zFieldNeighbourWeighted, v1);
			fnField.getFieldValue(p, zSpace::zFieldContainedWeighted, v2);
			accum += v1 + v2;
		}
		return accum;
	}

	ZSPACE_WASM_EXPORT const float* zspace_field2d_scalars_ptr()
	{
		if (!gHasField2D) return nullptr;
		return zSpace::zFieldObjectStorage::get(gField2DObject).fieldValues.data();
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_scalars_count()
	{
		if (!gHasField2D) return 0;
		return static_cast<int>(zSpace::zFieldObjectStorage::get(gField2DObject).fieldValues.size());
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_set_scalars(const float* values, int count, int colorType, float sdfWidth)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			if (!values || count <= 0) throw std::runtime_error("Invalid scalar array.");
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			if (count != fnField.numFieldValues()) throw std::runtime_error("Scalar count does not match Field2D resolution.");
			zSpace::zScalarArray scalars(values, values + count);
			if (colorType == 0) gField2DColorType = zSpace::zFieldColorType::zFieldSlime;
			else if (colorType == 1) gField2DColorType = zSpace::zFieldColorType::zFieldSDF;
			else gField2DColorType = zSpace::zFieldColorType::zFieldRegular;
			gField2DSdfWidth = std::max(0.001f, sdfWidth);
			fnField.setFieldColorDomain(gField2DColorDomain);
			fnField.setFieldValues(scalars, gField2DColorType, gField2DSdfWidth);
			syncField2DMeshToRenderBuffers(gField2DObject);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D set scalars error."; }
		return 0;
	}

	// ---- Field2D SDF & Distance Functions ----

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_circle(
		float cx, float cy, float cz, float radius, float annularVal,
		int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			zSpace::zVector cen(cx, cy, cz);
			fnField.getScalars_Circle(scalars, cen, radius, annularVal, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D circle SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_square(
		float cx, float cy, float cz, float dimX, float dimY, float annularVal,
		int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			zSpace::zVector cen(cx, cy, cz);
			zSpace::zVector dims(dimX, dimY, 0.0f);
			fnField.getScalars_Square(scalars, cen, dims, annularVal, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D square SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_ellipse(
		float cx, float cy, float cz, float a, float b, float annularVal,
		int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			zSpace::zVector cen(cx, cy, cz);
			fnField.getScalars_Ellipse(scalars, cen, a, b, annularVal, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D ellipse SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_line(
		float v0x, float v0y, float v0z,
		float v1x, float v1y, float v1z,
		float annularVal, int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			zSpace::zVector v0(v0x, v0y, v0z), v1(v1x, v1y, v1z);
			fnField.getScalars_Line(scalars, v0, v1, annularVal, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D line SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_triangle(
		float p0x, float p0y, float p0z,
		float p1x, float p1y, float p1z,
		float p2x, float p2y, float p2z,
		float annularVal, int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			zSpace::zPoint p0(p0x, p0y, p0z), p1(p1x, p1y, p1z), p2(p2x, p2y, p2z);
			fnField.getScalars_Triangle(scalars, p0, p1, p2, annularVal, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D triangle SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_trapezoid(
		float r1, float r2, float he, float annularVal,
		int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			fnField.getScalars_Trapezoid(scalars, r1, r2, he, annularVal, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D trapezoid SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_polygon_regular(
		float cx, float cy, float cz, float radius, int numSides, float annularVal,
		int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			if (numSides < 3) throw std::runtime_error("Regular polygon requires at least 3 sides.");
			zSpace::zPointArray pts;
			zSpace::zIntArray edges;
			pts.reserve(static_cast<std::size_t>(numSides));
			edges.reserve(static_cast<std::size_t>(numSides * 2));
			constexpr float kTwoPi = 6.2831853071795864769f;
			for (int i = 0; i < numSides; ++i)
			{
				const float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(numSides);
				pts.emplace_back(cx + radius * std::cos(angle), cy + radius * std::sin(angle), cz);
				edges.push_back(i);
				edges.push_back((i + 1) % numSides);
			}
			zSpace::zObjectGraph polyGraph;
			zSpace::zFnGraph(polyGraph).create(pts, edges);

			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			fnField.getScalars_Polygon(scalars, polyGraph, false);
			if (annularVal != 0.0f)
			{
				const float shift = std::clamp(annularVal, 0.0f, 1.0f);
				for (auto& val : scalars) val = std::abs(val) - shift;
			}
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D regular polygon SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_polygon_from_graph(
		const float* positions, int positionCount,
		const std::uint32_t* edgeConnects, int edgeConnectCount,
		int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			if (!positions || positionCount < 9 || positionCount % 3 != 0 || !edgeConnects || edgeConnectCount < 6 || edgeConnectCount % 2 != 0)
				throw std::runtime_error("Polygon SDF requires a closed graph with at least 3 vertices.");

			zSpace::zPointArray pts;
			pts.reserve(static_cast<std::size_t>(positionCount / 3));
			for (int i = 0; i < positionCount; i += 3)
				pts.emplace_back(positions[i], positions[i + 1], positions[i + 2]);

			zSpace::zIntArray edges;
			edges.reserve(static_cast<std::size_t>(edgeConnectCount));
			for (int i = 0; i < edgeConnectCount; ++i)
				edges.push_back(static_cast<int>(edgeConnects[i]));

			zSpace::zObjectGraph polyGraph;
			zSpace::zFnGraph(polyGraph).create(pts, edges);

			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			fnField.getScalars_Polygon(scalars, polyGraph, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D graph polygon SDF error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_vertex_distance(
		const float* positions, int positionCount,
		float offset, int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			if (!positions || positionCount < 3 || positionCount % 3 != 0)
				throw std::runtime_error("Vertex distance requires at least one 3D vertex.");

			zSpace::zObjectMesh tempMesh;
			auto& mData = zSpace::zMeshObjectStorage::edit(tempMesh);
			mData.positions.reserve(static_cast<std::size_t>(positionCount / 3));
			for (int i = 0; i < positionCount; i += 3)
				mData.positions.emplace_back(positions[i], positions[i + 1], positions[i + 2]);

			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			fnField.getScalarsAsVertexDistance(scalars, tempMesh, offset, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D vertex distance error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field2d_sdf_edge_distance(
		const float* positions, int positionCount,
		const std::uint32_t* edgeConnects, int edgeConnectCount,
		float offset, int normalise, int combineMode, float sminK)
	{
		try
		{
			gBuffers.lastError.clear();
			ensureField2DExists();
			if (!positions || positionCount < 6 || positionCount % 3 != 0 || !edgeConnects || edgeConnectCount < 2 || edgeConnectCount % 2 != 0)
				throw std::runtime_error("Edge distance requires graph vertices and edge endpoint pairs.");

			zSpace::zPointArray pts;
			pts.reserve(static_cast<std::size_t>(positionCount / 3));
			for (int i = 0; i < positionCount; i += 3)
				pts.emplace_back(positions[i], positions[i + 1], positions[i + 2]);

			zSpace::zIntArray edges;
			edges.reserve(static_cast<std::size_t>(edgeConnectCount));
			for (int i = 0; i < edgeConnectCount; ++i)
				edges.push_back(static_cast<int>(edgeConnects[i]));

			zSpace::zObjectGraph sourceGraph;
			zSpace::zFnGraph(sourceGraph).create(pts, edges);

			zSpace::zFnMeshScalarField fnField(gField2DObject);
			zSpace::zScalarArray scalars;
			fnField.getScalarsAsEdgeDistance(scalars, sourceGraph, offset, false);
			applyAndCombineField2DScalars(fnField, scalars, combineMode, sminK, normalise != 0);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field2D edge distance error."; }
		return 0;
	}

	// ---- Field3D C ABI Functions ----

	ZSPACE_WASM_EXPORT int zspace_field3d_create(
		float minX, float minY, float minZ,
		float maxX, float maxY, float maxZ,
		int nX, int nY, int nZ)
	{
		try
		{
			gBuffers.lastError.clear();
			if (nX < 2 || nY < 2 || nZ < 2) throw std::runtime_error("Field3D resolution must be at least 2x2x2.");
			zSpace::zFnPointScalarField fn3D(gField3DObject);
			zSpace::zPoint minBB(minX, minY, minZ), maxBB(maxX, maxY, maxZ);
			fn3D.create(minBB, maxBB, nX, nY, nZ, 1);
			gHasField3D = true;
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field3D create error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field3d_vertex_distance(
		const float* positions, int positionCount,
		float offset, int normalise)
	{
		try
		{
			gBuffers.lastError.clear();
			if (!gHasField3D) throw std::runtime_error("No active 3D field.");
			if (!positions || positionCount < 3 || positionCount % 3 != 0)
				throw std::runtime_error("Vertex distance requires at least one 3D vertex.");

			zSpace::zObjectMesh tempMesh;
			auto& mData = zSpace::zMeshObjectStorage::edit(tempMesh);
			mData.positions.reserve(static_cast<std::size_t>(positionCount / 3));
			for (int i = 0; i < positionCount; i += 3)
				mData.positions.emplace_back(positions[i], positions[i + 1], positions[i + 2]);

			zSpace::zFnPointScalarField fn3D(gField3DObject);
			zSpace::zScalarArray dist3D;
			fn3D.getScalarsAsVertexDistance(dist3D, tempMesh, offset, normalise != 0);
			fn3D.setFieldValues(dist3D);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field3D vertex distance error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field3d_smooth(int numSmooth, double diffuseDamp, int diffusionType)
	{
		try
		{
			gBuffers.lastError.clear();
			if (!gHasField3D) throw std::runtime_error("No active 3D field.");
			zSpace::zFnPointScalarField fn3D(gField3DObject);
			zSpace::zDiffusionType type = (diffusionType == 1) ? zSpace::zLaplacian : zSpace::zAverage;
			fn3D.smoothField(std::max(1, numSmooth), diffuseDamp, type);
			return 1;
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field3D smooth error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT int zspace_field3d_gradients()
	{
		try
		{
			gBuffers.lastError.clear();
			if (!gHasField3D) throw std::runtime_error("No active 3D field.");
			zSpace::zFnPointScalarField fn3D(gField3DObject);
			gField3DGradients = fn3D.getGradients();
			return static_cast<int>(gField3DGradients.size());
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown Field3D gradients error."; }
		return 0;
	}

	ZSPACE_WASM_EXPORT const float* zspace_field3d_scalars_ptr()
	{
		if (!gHasField3D) return nullptr;
		return zSpace::zFieldObjectStorage::get(gField3DObject).fieldValues.data();
	}

	ZSPACE_WASM_EXPORT int zspace_field3d_scalars_count()
	{
		if (!gHasField3D) return 0;
		return static_cast<int>(zSpace::zFieldObjectStorage::get(gField3DObject).fieldValues.size());
	}

	ZSPACE_WASM_EXPORT const char* zspace_field_benchmark_json_ptr()
	{
		static std::string sBenchmarkJson;
		try
		{
			gBuffers.lastError.clear();
			using Clock = std::chrono::steady_clock;
			using Json = nlohmann::json;

			auto hashFloats = [](const zSpace::zScalarArray& values) -> std::string {
				std::uint64_t h = 14695981039346656037ULL;
				for (float v : values)
				{
					float q = std::round(v * 10000.0f) / 10000.0f;
					unsigned char bytes[sizeof(float)];
					std::memcpy(bytes, &q, sizeof(float));
					for (auto b : bytes)
					{
						h ^= b;
						h *= 1099511628211ULL;
					}
				}
				std::ostringstream ss;
				ss << std::hex << h;
				return ss.str();
			};

			auto median3 = [](double a, double b, double c) -> double {
				if (a > b) std::swap(a, b);
				if (b > c) std::swap(b, c);
				if (a > b) std::swap(a, b);
				return b;
			};

			zSpace::zObjectMesh sourceMesh;
			{
				const int gridN = 25;
				zSpace::zPointArray pts;
				zSpace::zIntArray counts, connects;
				pts.reserve((gridN + 1) * (gridN + 1));
				for (int i = 0; i <= gridN; ++i)
				{
					for (int j = 0; j <= gridN; ++j)
					{
						float x = -8.0f + 16.0f * (float(i) / float(gridN)) + 0.013f * float(j);
						float y = -8.0f + 16.0f * (float(j) / float(gridN)) + 0.017f * float(i);
						float z = 0.25f + std::sin(x * 0.5f) * std::cos(y * 0.5f);
						pts.emplace_back(x, y, z);
					}
				}
				for (int i = 0; i < gridN; ++i)
				{
					for (int j = 0; j < gridN; ++j)
					{
						int v0 = i * (gridN + 1) + j;
						int v1 = (i + 1) * (gridN + 1) + j;
						counts.push_back(4);
						connects.push_back(v0);
						connects.push_back(v1);
						connects.push_back(v1 + 1);
						connects.push_back(v0 + 1);
					}
				}
				zSpace::zFnMesh(sourceMesh).create(pts, counts, connects);
			}

			Json report;
			report["method"] = "WebAssembly (zspace_core.wasm) field benchmark across 3 trials per workload.";

			// 1. 2D Create
			{
				double t[3] = {};
				int numVals = 0;
				for (int trial = 0; trial < 3; ++trial)
				{
					zSpace::zObjectMeshScalarField field;
					zSpace::zFnMeshScalarField fn(field);
					auto start = Clock::now();
					fn.create(zSpace::zPoint(-10, -10, 0), zSpace::zPoint(10, 10, 0), 250, 250, 1, true, true);
					t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
					numVals = fn.numFieldValues();
				}
				report["meshField2D_create_250x250"] = {
					{"cells", numVals}, {"median_ms", median3(t[0], t[1], t[2])}, {"trials_ms", {t[0], t[1], t[2]}}
				};
			}

			zSpace::zObjectMeshScalarField field2D;
			zSpace::zFnMeshScalarField fn2D(field2D);
			fn2D.create(zSpace::zPoint(-10, -10, 0), zSpace::zPoint(10, 10, 0), 250, 250, 1, true, true);

			// 2. 2D SDF + Booleans + Colors
			zSpace::zScalarArray sdfResult;
			{
				double t[3] = {};
				for (int trial = 0; trial < 3; ++trial)
				{
					auto start = Clock::now();
					zSpace::zScalarArray sCircle, sSquare, sLine, sUnion, sDiff;
					zSpace::zVector cen1(-2.5f, 0.0f, 0.0f), cen2(2.5f, 0.0f, 0.0f), dims(3.5f, 2.5f, 0.0f);
					zSpace::zVector l0(-6.0f, -4.0f, 0.0f), l1(6.0f, 4.0f, 0.0f);
					fn2D.getScalars_Circle(sCircle, cen1, 4.0f, 0.0, false);
					fn2D.getScalars_Square(sSquare, cen2, dims, 0.0f, false);
					fn2D.getScalars_Line(sLine, l0, l1, 0.5, false);
					fn2D.boolean_union(sCircle, sSquare, sUnion, false);
					fn2D.boolean_subtract(sUnion, sLine, sDiff, true);
					fn2D.setFieldValues(sDiff, zSpace::zFieldColorType::zFieldSDF, 0.1f);
					t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
					sdfResult = std::move(sDiff);
				}
				report["meshField2D_sdf_booleans_colors_250x250"] = {
					{"cells", fn2D.numFieldValues()}, {"median_ms", median3(t[0], t[1], t[2])},
					{"trials_ms", {t[0], t[1], t[2]}}, {"hash", hashFloats(sdfResult)}
				};
			}

			// 3. 2D Smooth 10 iter
			{
				double t[3] = {};
				zSpace::zScalarArray smoothed;
				for (int trial = 0; trial < 3; ++trial)
				{
					smoothed = sdfResult;
					auto start = Clock::now();
					fn2D.smoothField(smoothed, 10, 1.0, zSpace::zAverage);
					t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
				}
				report["meshField2D_smooth_10iter_250x250"] = {
					{"cells", fn2D.numFieldValues()}, {"median_ms", median3(t[0], t[1], t[2])},
					{"trials_ms", {t[0], t[1], t[2]}}, {"hash", hashFloats(smoothed)}
				};
			}

			// 4. 2D Vector from Scalar
			{
				double t[3] = {};
				int vecCount = 0;
				for (int trial = 0; trial < 3; ++trial)
				{
					zSpace::zObjectMeshVectorField vecField;
					zSpace::zFnMeshVectorField fnVec(vecField);
					auto start = Clock::now();
					fnVec.createVectorFromScalarField(field2D);
					t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
					vecCount = fnVec.numFieldValues();
				}
				report["meshField2D_vector_from_scalar_250x250"] = {
					{"cells", vecCount}, {"median_ms", median3(t[0], t[1], t[2])}, {"trials_ms", {t[0], t[1], t[2]}}
				};
			}

			// 5. 2D Sampling 50k queries
			{
				zSpace::zPointArray queryPts;
				queryPts.reserve(25000);
				for (int i = 0; i < 25000; ++i)
				{
					float u = float(i % 250) / 250.0f;
					float v = float(i / 250) / 100.0f;
					queryPts.emplace_back(-9.0f + 18.0f * u, -9.0f + 18.0f * v, 0.0f);
				}
				double t[3] = {};
				double accum = 0.0;
				for (int trial = 0; trial < 3; ++trial)
				{
					accum = 0.0;
					auto start = Clock::now();
					for (std::size_t i = 0; i < queryPts.size(); ++i)
					{
						zSpace::zScalar val1 = 0.0f, val2 = 0.0f;
						fn2D.getFieldValue(queryPts[i], zSpace::zFieldNeighbourWeighted, val1);
						fn2D.getFieldValue(queryPts[i], zSpace::zFieldContainedWeighted, val2);
						accum += val1 + val2;
					}
					t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
				}
				report["meshField2D_sampling_50k_queries"] = {
					{"queries", 50000}, {"median_ms", median3(t[0], t[1], t[2])},
					{"trials_ms", {t[0], t[1], t[2]}}, {"checksum", accum}
				};
			}

			// 6. 2D Vertex & Edge Distance
			{
				zSpace::zObjectMeshScalarField distField;
				zSpace::zFnMeshScalarField fnDist(distField);
				fnDist.create(zSpace::zPoint(-10, -10, 0), zSpace::zPoint(10, 10, 0), 150, 150, 1, true, true);
				double tVert[3] = {};
				zSpace::zScalarArray vScalars;
				for (int trial = 0; trial < 3; ++trial)
				{
					auto startV = Clock::now();
					fnDist.getScalarsAsVertexDistance(vScalars, sourceMesh, 2.0f, true);
					tVert[trial] = std::chrono::duration<double, std::milli>(Clock::now() - startV).count();
				}
				report["meshField2D_vertex_distance_150x150"] = {
					{"cells", fnDist.numFieldValues()}, {"source_vertices", 676},
					{"median_ms", median3(tVert[0], tVert[1], tVert[2])},
					{"trials_ms", {tVert[0], tVert[1], tVert[2]}}, {"hash", hashFloats(vScalars)}
				};

				zSpace::zObjectGraph sourceGraph;
				{
					zSpace::zPointArray gPts;
					zSpace::zIntArray gEdges;
					for (int i = 0; i <= 10; ++i)
					{
						float t = -8.0f + 16.0f * (float(i) / 10.0f);
						gPts.emplace_back(t, std::sin(t) * 3.0f, 0.0f);
						if (i > 0) { gEdges.push_back(i - 1); gEdges.push_back(i); }
					}
					zSpace::zFnGraph(sourceGraph).create(gPts, gEdges);
				}
				zSpace::zObjectMeshScalarField edgeDistField;
				zSpace::zFnMeshScalarField fnEdgeDist(edgeDistField);
				fnEdgeDist.create(zSpace::zPoint(-10, -10, 0), zSpace::zPoint(10, 10, 0), 50, 50, 1, true, true);
				double tEdge[3] = {};
				zSpace::zScalarArray eScalars;
				for (int trial = 0; trial < 3; ++trial)
				{
					auto startE = Clock::now();
					fnEdgeDist.getScalarsAsEdgeDistance(eScalars, sourceGraph, 2.0f, true);
					tEdge[trial] = std::chrono::duration<double, std::milli>(Clock::now() - startE).count();
				}
				report["meshField2D_edge_distance_50x50"] = {
					{"cells", fnEdgeDist.numFieldValues()}, {"source_edges", 10},
					{"median_ms", median3(tEdge[0], tEdge[1], tEdge[2])},
					{"trials_ms", {tEdge[0], tEdge[1], tEdge[2]}}, {"hash", hashFloats(eScalars)}
				};
			}

			// 7. 2D Contours
			{
				double t[3] = {};
				int contourVerts = 0, isoFaces = 0, bandFaces = 0;
				for (int trial = 0; trial < 3; ++trial)
				{
					zSpace::zObjectGraph contourGraph;
					zSpace::zObjectMesh isoMesh, bandMesh;
					auto start = Clock::now();
					fn2D.getIsocontour(contourGraph, 0.0f);
					fn2D.getIsolineMesh(isoMesh, 0.0f, false);
					fn2D.getIsobandMesh(bandMesh, -0.15f, 0.15f);
					t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
					contourVerts = zSpace::zFnGraph(contourGraph).numVertices();
					isoFaces = zSpace::zFnMesh(isoMesh).numPolygons();
					bandFaces = zSpace::zFnMesh(bandMesh).numPolygons();
				}
				report["meshField2D_contours_250x250"] = {
					{"cells", fn2D.numFieldValues()}, {"contour_vertices", contourVerts},
					{"isomesh_faces", isoFaces}, {"isoband_faces", bandFaces},
					{"median_ms", median3(t[0], t[1], t[2])}, {"trials_ms", {t[0], t[1], t[2]}}
				};
			}

			// 8. 3D Point Scalar Field
			{
				double tCreate[3] = {}, tDist[3] = {}, tSmooth[3] = {}, tGrad[3] = {};
				int voxels = 0;
				for (int trial = 0; trial < 3; ++trial)
				{
					zSpace::zObjectPointScalarField field3D;
					zSpace::zFnPointScalarField fn3D(field3D);

					auto sCreate = Clock::now();
					fn3D.create(zSpace::zPoint(-10, -10, -10), zSpace::zPoint(10, 10, 10), 50, 50, 50, 1);
					tCreate[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sCreate).count();
					voxels = fn3D.numFieldValues();

					zSpace::zScalarArray dist3D;
					auto sDist = Clock::now();
					fn3D.getScalarsAsVertexDistance(dist3D, sourceMesh, 2.0, true);
					fn3D.setFieldValues(dist3D);
					tDist[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sDist).count();

					auto sSmooth = Clock::now();
					fn3D.smoothField(5, 1.0, zSpace::zAverage);
					tSmooth[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sSmooth).count();

					auto sGrad = Clock::now();
					zSpace::zVectorArray grads = fn3D.getGradients();
					tGrad[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sGrad).count();
					(void)grads;
				}
				report["pointField3D_create_50x50x50"] = {
					{"voxels", voxels}, {"median_ms", median3(tCreate[0], tCreate[1], tCreate[2])}, {"trials_ms", {tCreate[0], tCreate[1], tCreate[2]}}
				};
				report["pointField3D_vertex_distance_50x50x50"] = {
					{"voxels", voxels}, {"median_ms", median3(tDist[0], tDist[1], tDist[2])}, {"trials_ms", {tDist[0], tDist[1], tDist[2]}}
				};
				report["pointField3D_smooth_5iter_50x50x50"] = {
					{"voxels", voxels}, {"median_ms", median3(tSmooth[0], tSmooth[1], tSmooth[2])}, {"trials_ms", {tSmooth[0], tSmooth[1], tSmooth[2]}}
				};
				report["pointField3D_gradients_50x50x50"] = {
					{"voxels", voxels}, {"median_ms", median3(tGrad[0], tGrad[1], tGrad[2])}, {"trials_ms", {tGrad[0], tGrad[1], tGrad[2]}}
				};
			}

			sBenchmarkJson = report.dump(2);
			return sBenchmarkJson.c_str();
		}
		catch (const std::exception& error) { gBuffers.lastError = error.what(); }
		catch (...) { gBuffers.lastError = "Unknown WASM field benchmark error."; }
		return nullptr;
	}
}
