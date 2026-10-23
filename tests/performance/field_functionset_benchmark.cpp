#include <zspace/interface.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

using namespace zSpace;
using Clock = std::chrono::steady_clock;
using Json = nlohmann::json;

namespace
{
	std::uint64_t hashFloats(const zScalarArray& values)
	{
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
		return h;
	}

	std::string hexHash(std::uint64_t h)
	{
		std::ostringstream ss;
		ss << std::hex << h;
		return ss.str();
	}

	void buildSourceMesh(zObjectMesh& mesh, int gridN)
	{
		zPointArray pts;
		zIntArray counts;
		zIntArray connects;
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
				int v2 = v1 + 1;
				int v3 = v0 + 1;
				counts.push_back(4);
				connects.push_back(v0);
				connects.push_back(v1);
				connects.push_back(v2);
				connects.push_back(v3);
			}
		}
		zFnMesh fn(mesh);
		fn.create(pts, counts, connects);
	}

	double median3(double a, double b, double c)
	{
		if (a > b) std::swap(a, b);
		if (b > c) std::swap(b, c);
		if (a > b) std::swap(a, b);
		return b;
	}
}

int main()
{
	try
	{
		Json report;
		report["method"] = "Native Release field benchmark across 3 trials per workload.";

		zObjectMesh sourceMesh;
		buildSourceMesh(sourceMesh, 25); // 676 vertices, 625 faces, 1300 edges

		// 1. 2D Scalar Mesh Field Creation (250x250 = 62,500 cells)
		{
			double t[3] = {};
			int numVals = 0;
			for (int trial = 0; trial < 3; ++trial)
			{
				zObjectMeshScalarField field;
				zFnMeshScalarField fn(field);
				auto start = Clock::now();
				fn.create(zPoint(-10, -10, 0), zPoint(10, 10, 0), 250, 250, 1, true, true);
				t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
				numVals = fn.numFieldValues();
			}
			report["meshField2D_create_250x250"] = {
				{"cells", numVals},
				{"median_ms", median3(t[0], t[1], t[2])},
				{"trials_ms", {t[0], t[1], t[2]}}
			};
		}

		// Shared 250x250 2D Scalar Field for subsequent 2D benchmarks
		zObjectMeshScalarField field2D;
		zFnMeshScalarField fn2D(field2D);
		fn2D.create(zPoint(-10, -10, 0), zPoint(10, 10, 0), 250, 250, 1, true, true);

		// 2. 2D SDF Primitives + Booleans + setFieldValues/updateColors (250x250)
		zScalarArray sdfResult;
		{
			double t[3] = {};
			for (int trial = 0; trial < 3; ++trial)
			{
				auto start = Clock::now();
				zScalarArray sCircle, sSquare, sLine, sUnion, sDiff;
				zVector cen1(-2.5f, 0.0f, 0.0f);
				zVector cen2(2.5f, 0.0f, 0.0f);
				zVector dims(3.5f, 2.5f, 0.0f);
				zVector l0(-6.0f, -4.0f, 0.0f), l1(6.0f, 4.0f, 0.0f);
				fn2D.getScalars_Circle(sCircle, cen1, 4.0f, 0.0, false);
				fn2D.getScalars_Square(sSquare, cen2, dims, 0.0f, false);
				fn2D.getScalars_Line(sLine, l0, l1, 0.5, false);
				fn2D.boolean_union(sCircle, sSquare, sUnion, false);
				fn2D.boolean_subtract(sUnion, sLine, sDiff, true);
				fn2D.setFieldValues(sDiff, zFieldColorType::zFieldSDF, 0.1f);
				t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
				sdfResult = std::move(sDiff);
			}
			report["meshField2D_sdf_booleans_colors_250x250"] = {
				{"cells", fn2D.numFieldValues()},
				{"median_ms", median3(t[0], t[1], t[2])},
				{"trials_ms", {t[0], t[1], t[2]}},
				{"hash", hexHash(hashFloats(sdfResult))}
			};
		}

		// 3. 2D Scalar Field Smoothing (250x250, 10 iterations zAverage)
		{
			double t[3] = {};
			zScalarArray smoothed;
			for (int trial = 0; trial < 3; ++trial)
			{
				smoothed = sdfResult;
				auto start = Clock::now();
				fn2D.smoothField(smoothed, 10, 1.0, zAverage);
				t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
			}
			report["meshField2D_smooth_10iter_250x250"] = {
				{"cells", fn2D.numFieldValues()},
				{"median_ms", median3(t[0], t[1], t[2])},
				{"trials_ms", {t[0], t[1], t[2]}},
				{"hash", hexHash(hashFloats(smoothed))}
			};
		}

		// 4. 2D Gradient & Vector Field Creation from Scalar Field (250x250)
		{
			double t[3] = {};
			int vecCount = 0;
			for (int trial = 0; trial < 3; ++trial)
			{
				zObjectMeshVectorField vecField;
				zFnMeshVectorField fnVec(vecField);
				auto start = Clock::now();
				fnVec.createVectorFromScalarField(field2D);
				t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
				vecCount = fnVec.numFieldValues();
			}
			report["meshField2D_vector_from_scalar_250x250"] = {
				{"cells", vecCount},
				{"median_ms", median3(t[0], t[1], t[2])},
				{"trials_ms", {t[0], t[1], t[2]}}
			};
		}

		// 5. 2D Field Sampling (50,000 queries across zFieldNeighbourWeighted & zFieldContainedWeighted)
		{
			zPointArray queryPts;
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
					zScalar val1 = 0.0f, val2 = 0.0f;
					fn2D.getFieldValue(queryPts[i], zFieldNeighbourWeighted, val1);
					fn2D.getFieldValue(queryPts[i], zFieldContainedWeighted, val2);
					accum += val1 + val2;
				}
				t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
			}
			report["meshField2D_sampling_50k_queries"] = {
				{"queries", 50000},
				{"median_ms", median3(t[0], t[1], t[2])},
				{"trials_ms", {t[0], t[1], t[2]}},
				{"checksum", accum}
			};
		}

		// 6. 2D Vertex & Edge Distance Fields
		{
			zObjectMeshScalarField distField;
			zFnMeshScalarField fnDist(distField);
			fnDist.create(zPoint(-10, -10, 0), zPoint(10, 10, 0), 150, 150, 1, true, true);

			double tVert[3] = {};
			zScalarArray vScalars;
			for (int trial = 0; trial < 3; ++trial)
			{
				auto startV = Clock::now();
				fnDist.getScalarsAsVertexDistance(vScalars, sourceMesh, 2.0f, true);
				tVert[trial] = std::chrono::duration<double, std::milli>(Clock::now() - startV).count();
			}
			report["meshField2D_vertex_distance_150x150"] = {
				{"cells", fnDist.numFieldValues()},
				{"source_vertices", 676},
				{"median_ms", median3(tVert[0], tVert[1], tVert[2])},
				{"trials_ms", {tVert[0], tVert[1], tVert[2]}},
				{"hash", hexHash(hashFloats(vScalars))}
			};

			// Use a 30x30 field and 100-edge graph for edge distance so the baseline's
			// per-edge getHalfEdge(0) + syncEdgeList pathology completes in reasonable time.
			zObjectGraph sourceGraph;
			{
				zPointArray gPts;
				zIntArray gEdges;
				for (int i = 0; i <= 10; ++i)
				{
					float t = -8.0f + 16.0f * (float(i) / 10.0f);
					gPts.emplace_back(t, std::sin(t) * 3.0f, 0.0f);
					if (i > 0) { gEdges.push_back(i - 1); gEdges.push_back(i); }
				}
				zFnGraph(sourceGraph).create(gPts, gEdges);
			}
			zObjectMeshScalarField edgeDistField;
			zFnMeshScalarField fnEdgeDist(edgeDistField);
			fnEdgeDist.create(zPoint(-10, -10, 0), zPoint(10, 10, 0), 50, 50, 1, true, true);
			double tEdge[3] = {};
			zScalarArray eScalars;
			for (int trial = 0; trial < 3; ++trial)
			{
				auto startE = Clock::now();
				fnEdgeDist.getScalarsAsEdgeDistance(eScalars, sourceGraph, 2.0f, true);
				tEdge[trial] = std::chrono::duration<double, std::milli>(Clock::now() - startE).count();
			}
			report["meshField2D_edge_distance_50x50"] = {
				{"cells", fnEdgeDist.numFieldValues()},
				{"source_edges", 10},
				{"median_ms", median3(tEdge[0], tEdge[1], tEdge[2])},
				{"trials_ms", {tEdge[0], tEdge[1], tEdge[2]}},
				{"hash", hexHash(hashFloats(eScalars))}
			};
		}

		// 7. 2D IsoContour, IsolineMesh & IsobandMesh (250x250)
		{
			double t[3] = {};
			int contourVerts = 0, isoFaces = 0, bandFaces = 0;
			for (int trial = 0; trial < 3; ++trial)
			{
				zObjectGraph contourGraph;
				zObjectMesh isoMesh, bandMesh;
				auto start = Clock::now();
				fn2D.getIsocontour(contourGraph, 0.0f);
				fn2D.getIsolineMesh(isoMesh, 0.0f, false);
				fn2D.getIsobandMesh(bandMesh, -0.15f, 0.15f);
				t[trial] = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
				contourVerts = zFnGraph(contourGraph).numVertices();
				isoFaces = zFnMesh(isoMesh).numPolygons();
				bandFaces = zFnMesh(bandMesh).numPolygons();
			}
			report["meshField2D_contours_250x250"] = {
				{"cells", fn2D.numFieldValues()},
				{"contour_vertices", contourVerts},
				{"isomesh_faces", isoFaces},
				{"isoband_faces", bandFaces},
				{"median_ms", median3(t[0], t[1], t[2])},
				{"trials_ms", {t[0], t[1], t[2]}}
			};
		}

		// 8. 3D Point Scalar Field Creation, Distance, Smoothing & Gradients (50x50x50 = 125,000 voxels)
		{
			double tCreate[3] = {}, tDist[3] = {}, tSmooth[3] = {}, tGrad[3] = {};
			int voxels = 0;
			for (int trial = 0; trial < 3; ++trial)
			{
				zObjectPointScalarField field3D;
				zFnPointScalarField fn3D(field3D);

				auto sCreate = Clock::now();
				fn3D.create(zPoint(-10, -10, -10), zPoint(10, 10, 10), 50, 50, 50, 1);
				tCreate[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sCreate).count();
				voxels = fn3D.numFieldValues();

				zScalarArray dist3D;
				auto sDist = Clock::now();
				fn3D.getScalarsAsVertexDistance(dist3D, sourceMesh, 2.0, true);
				fn3D.setFieldValues(dist3D);
				tDist[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sDist).count();

				auto sSmooth = Clock::now();
				fn3D.smoothField(5, 1.0, zAverage);
				tSmooth[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sSmooth).count();

				auto sGrad = Clock::now();
				zVectorArray grads = fn3D.getGradients();
				tGrad[trial] = std::chrono::duration<double, std::milli>(Clock::now() - sGrad).count();
				(void)grads;
			}
			report["pointField3D_create_50x50x50"] = {
				{"voxels", voxels},
				{"median_ms", median3(tCreate[0], tCreate[1], tCreate[2])},
				{"trials_ms", {tCreate[0], tCreate[1], tCreate[2]}}
			};
			report["pointField3D_vertex_distance_50x50x50"] = {
				{"voxels", voxels},
				{"median_ms", median3(tDist[0], tDist[1], tDist[2])},
				{"trials_ms", {tDist[0], tDist[1], tDist[2]}}
			};
			report["pointField3D_smooth_5iter_50x50x50"] = {
				{"voxels", voxels},
				{"median_ms", median3(tSmooth[0], tSmooth[1], tSmooth[2])},
				{"trials_ms", {tSmooth[0], tSmooth[1], tSmooth[2]}}
			};
			report["pointField3D_gradients_50x50x50"] = {
				{"voxels", voxels},
				{"median_ms", median3(tGrad[0], tGrad[1], tGrad[2])},
				{"trials_ms", {tGrad[0], tGrad[1], tGrad[2]}}
			};
		}

		std::cout << "\n===JSON_REPORT_BEGIN===\n" << report.dump(2) << "\n===JSON_REPORT_END===\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}
