#ifndef ZSPACE_FIELD_OBJECT_STORAGE_H
#define ZSPACE_FIELD_OBJECT_STORAGE_H

#pragma once

#include <zspace/zInterface/objects/zObjectMeshField.h>
#include <zspace/zInterface/objects/zObjectPointField.h>
#include <src/zInterface/objects/zMeshObjectStorage.h>
#include <src/zInterface/objects/internal/zObjectPointCloudImpl.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace zSpace
{
	class zObjectMeshScalarField::Impl
	{
	public:
		zField2D<zScalar> field;
	};

	class zObjectMeshVectorField::Impl
	{
	public:
		zField2D<zVector> field;
	};

	class zObjectPointScalarField::Impl
	{
	public:
		zField3D<zScalar> field;
	};

	class zObjectPointVectorField::Impl
	{
	public:
		zField3D<zVector> field;
	};

	class zFieldObjectStorage
	{
	public:
		static zField2D<zScalar>& get(zObjectMeshScalarField& obj) { return obj.fieldImpl->field; }
		static const zField2D<zScalar>& get(const zObjectMeshScalarField& obj) { return obj.fieldImpl->field; }

		static zField2D<zVector>& get(zObjectMeshVectorField& obj) { return obj.fieldImpl->field; }
		static const zField2D<zVector>& get(const zObjectMeshVectorField& obj) { return obj.fieldImpl->field; }

		static zField3D<zScalar>& get(zObjectPointScalarField& obj) { return obj.fieldImpl->field; }
		static const zField3D<zScalar>& get(const zObjectPointScalarField& obj) { return obj.fieldImpl->field; }

		static zField3D<zVector>& get(zObjectPointVectorField& obj) { return obj.fieldImpl->field; }
		static const zField3D<zVector>& get(const zObjectPointVectorField& obj) { return obj.fieldImpl->field; }

		// Analytical O(N) 2D regular-grid mesh builder that populates zMeshFaceListStorage
		// directly in exact first-face encounter order without hash-map edge rebuilding.
		static void populateGridMesh2D(
			zObjectMesh& meshObj,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			int n_X,
			int n_Y,
			bool setValuesperVertex,
			bool triMesh)
		{
			const int resX = setValuesperVertex ? n_X : (n_X + 1);
			const int resY = setValuesperVertex ? n_Y : (n_Y + 1);
			auto& data = zMeshObjectStorage::edit(meshObj);
			data.clear();
			if (resX <= 0 || resY <= 0) return;

			const int numVerts = resX * resY;
			const int cellsX = std::max(0, resX - 1);
			const int cellsY = std::max(0, resY - 1);
			const int numCells = cellsX * cellsY;
			const int numFaces = triMesh ? (numCells * 2) : numCells;
			const int cornersPerFace = triMesh ? 3 : 4;
			const int numHorizEdges = (resX > 1 && resY > 0) ? (cellsX * resY) : 0;
			const int numVertEdges = (resX > 0 && resY > 1) ? (resX * cellsY) : 0;
			const int numDiagEdges = triMesh ? numCells : 0;
			const int numEdges = (numFaces > 0) ? (numHorizEdges + numVertEdges + numDiagEdges) : 0;

			zVector startPt = minBB;
			if (!setValuesperVertex)
			{
				startPt.x -= static_cast<float>(unit_X * 0.5);
				startPt.y -= static_cast<float>(unit_Y * 0.5);
			}

			data.positions.resize(static_cast<std::size_t>(numVerts));
			for (int i = 0; i < resX; ++i)
			{
				const float px = startPt.x + static_cast<float>(i * unit_X);
				zPoint* row = data.positions.data() + static_cast<std::size_t>(i) * resY;
				for (int j = 0; j < resY; ++j)
				{
					row[j] = zPoint(px, startPt.y + static_cast<float>(j * unit_Y), startPt.z);
				}
			}

			data.faceOffsets.resize(static_cast<std::size_t>(numFaces + 1));
			for (int f = 0; f <= numFaces; ++f)
			{
				data.faceOffsets[f] = f * cornersPerFace;
			}

			data.faceVertexIndices.resize(static_cast<std::size_t>(numFaces * cornersPerFace));
			data.edgeVertexIndices.reserve(static_cast<std::size_t>(numEdges * 2));
			data.edgeFirstVertices.reserve(static_cast<std::size_t>(numEdges));
			data.edgeUseCounts.reserve(static_cast<std::size_t>(numEdges));

			auto pushEdge = [&](int minV, int maxV, int firstV, int useCount)
			{
				data.edgeVertexIndices.push_back(minV);
				data.edgeVertexIndices.push_back(maxV);
				data.edgeFirstVertices.push_back(firstV);
				data.edgeUseCounts.push_back(useCount);
			};

			int* fPtr = data.faceVertexIndices.data();
			for (int i = 0; i < cellsX; ++i)
			{
				for (int j = 0; j < cellsY; ++j)
				{
					const int v0 = i * resY + j;
					const int v1 = (i + 1) * resY + j;
					const int v2 = v1 + 1;
					const int v3 = v0 + 1;

					if (triMesh)
					{
						// Triangle 1: (v0, v1, v3)
						fPtr[0] = v0;
						fPtr[1] = v1;
						fPtr[2] = v3;
						// Triangle 2: (v1, v2, v3)
						fPtr[3] = v1;
						fPtr[4] = v2;
						fPtr[5] = v3;
						fPtr += 6;

						// First-encounter edges in Triangle 1:
						// (v0, v1): new iff j == 0
						if (j == 0) pushEdge(v0, v1, v0, 1);
						// (v1, v3): diagonal, always new, shared by Triangle 1 and Triangle 2
						pushEdge(v3, v1, v1, 2);
						// (v3, v0): new iff i == 0
						if (i == 0) pushEdge(v0, v3, v3, 1);

						// First-encounter edges in Triangle 2:
						// (v1, v2): always new, shared if i + 1 < cellsX
						pushEdge(v1, v2, v1, (i + 1 < cellsX) ? 2 : 1);
						// (v2, v3): always new, shared if j + 1 < cellsY
						pushEdge(v3, v2, v2, (j + 1 < cellsY) ? 2 : 1);
					}
					else
					{
						// Quad: (v0, v1, v2, v3)
						fPtr[0] = v0;
						fPtr[1] = v1;
						fPtr[2] = v2;
						fPtr[3] = v3;
						fPtr += 4;

						if (j == 0) pushEdge(v0, v1, v0, 1);
						pushEdge(v1, v2, v1, (i + 1 < cellsX) ? 2 : 1);
						pushEdge(v3, v2, v2, (j + 1 < cellsY) ? 2 : 1);
						if (i == 0) pushEdge(v0, v3, v3, 1);
					}
				}
			}

			data.vertexNormals.assign(static_cast<std::size_t>(numVerts), zVector());
			data.faceNormals.assign(static_cast<std::size_t>(numFaces), zVector());
			data.vertexColors.assign(static_cast<std::size_t>(numVerts), zColor(1, 0, 0, 1));
			data.faceColors.assign(static_cast<std::size_t>(numFaces), zColor(0.5, 0.5, 0.5, 1));
			data.vertexWeights.assign(static_cast<std::size_t>(numVerts), 2.0);
			data.edgeColors.assign(static_cast<std::size_t>(numEdges), zColor(0, 0, 0, 0));
			data.edgeWeights.assign(static_cast<std::size_t>(numEdges), 1.0);
		}

		// Analytical O(N) 3D regular-grid point cloud builder.
		static void populateGridPointCloud3D(
			zObjectPointCloud& pointsObj,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			double unit_Z,
			int n_X,
			int n_Y,
			int n_Z)
		{
			auto& pc = pointsObj.impl->storage;
			pc.clear();
			if (n_X <= 0 || n_Y <= 0 || n_Z <= 0) return;

			const int total = n_X * n_Y * n_Z;
			pc.n_v = total;
			pc.vertices.resize(static_cast<std::size_t>(total));
			pc.vertexPositions.resize(static_cast<std::size_t>(total));
			pc.vertexColors.assign(static_cast<std::size_t>(total), zColor(1, 0, 0, 1));
			pc.vertexWeights.assign(static_cast<std::size_t>(total), 2.0);
			pc.positionMapDirty = true;

			int idx = 0;
			for (int i = 0; i < n_X; ++i)
			{
				const float px = minBB.x + static_cast<float>(i * unit_X);
				for (int j = 0; j < n_Y; ++j)
				{
					const float py = minBB.y + static_cast<float>(j * unit_Y);
					for (int k = 0; k < n_Z; ++k)
					{
						pc.vertices[idx].setId(idx);
						pc.vertexPositions[idx] = zPoint(px, py, minBB.z + static_cast<float>(k * unit_Z));
						++idx;
					}
				}
			}
		}
	};
}

#endif
