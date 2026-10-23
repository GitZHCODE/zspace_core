// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Authors : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>, Leo Bieling <leo.bieling.zaha-hadid.com>
//

#include <zspace/zInterface/functionsets/zFnMeshField.h>
#include <src/zInterface/functionsets/internal/zFieldAccel.h>
#include <src/zInterface/objects/zFieldObjectStorage.h>
#include <src/zInterface/objects/zMeshObjectStorage.h>
#include <src/zInterface/objects/zGraphObjectStorage.h>
#include <src/zInterface/objects/internal/zObjectPointCloudImpl.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace zSpace
{
	namespace
	{
		struct EdgeSegment2D
		{
			zPoint p0;
			zVector ab;
			double ab2;
		};

		inline zPoint gridCellPosition2D(const zVector& minBB, double unit_X, double unit_Y, bool valuesperVertex, int i, int j) noexcept
		{
			const double offset = valuesperVertex ? 0.0 : 0.5;
			return zPoint(
				minBB.x + static_cast<float>((static_cast<double>(i) + offset) * unit_X),
				minBB.y + static_cast<float>((static_cast<double>(j) + offset) * unit_Y),
				minBB.z);
		}

		inline bool computeCellCoords2D(
			const zPoint& pos,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			int n_X,
			int n_Y,
			int& index_X,
			int& index_Y) noexcept
		{
			if (n_X <= 0 || n_Y <= 0 || unit_X == 0.0 || unit_Y == 0.0)
			{
				index_X = -1;
				index_Y = -1;
				return false;
			}
			index_X = static_cast<int>(std::floor((pos.x - minBB.x) / unit_X));
			index_Y = static_cast<int>(std::floor((pos.y - minBB.y) / unit_Y));
			return (index_X >= 0 && index_X < n_X && index_Y >= 0 && index_Y < n_Y);
		}

		template<typename T, typename ValueT>
		bool sampleField2D(
			const ValueT* values,
			std::size_t valueCount,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			int n_X,
			int n_Y,
			bool valuesperVertex,
			const zPoint& samplePos,
			zFieldValueType type,
			T& fieldValue)
		{
			int ix = -1;
			int iy = -1;
			if (!computeCellCoords2D(samplePos, minBB, unit_X, unit_Y, n_X, n_Y, ix, iy))
			{
				return false;
			}
			const int centerId = ix * n_Y + iy;
			if (centerId < 0 || static_cast<std::size_t>(centerId) >= valueCount)
			{
				return false;
			}

			if (type == zFieldIndex)
			{
				fieldValue = values[centerId];
				return true;
			}

			if (type == zFieldNeighbourWeighted)
			{
				const int iMin = std::max(0, ix - 1);
				const int iMax = std::min(n_X - 1, ix + 1);
				const int jMin = std::max(0, iy - 1);
				const int jMax = std::min(n_Y - 1, iy + 1);

				double wSum = 0.0;
				T weightedSum = T();
				for (int i = iMin; i <= iMax; ++i)
				{
					for (int j = jMin; j <= jMax; ++j)
					{
						const zPoint nPos = gridCellPosition2D(minBB, unit_X, unit_Y, valuesperVertex, i, j);
						const double dx = static_cast<double>(samplePos.x) - nPos.x;
						const double dy = static_cast<double>(samplePos.y) - nPos.y;
						const double dz = static_cast<double>(samplePos.z) - nPos.z;
						const double distSq = dx * dx + dy * dy + dz * dz;
						if (distSq <= 0.0)
						{
							fieldValue = values[i * n_Y + j];
							return true;
						}
						const double w = 1.0 / distSq;
						ValueT v = values[i * n_Y + j];
						weightedSum += v * static_cast<float>(w);
						wSum += w;
					}
				}
				if (wSum > 0.0) weightedSum /= static_cast<float>(wSum);
				fieldValue = weightedSum;
				return true;
			}

			if (type == zFieldAdjacentWeighted)
			{
				const int coords[5][2] = {
					{ix - 1, iy},
					{ix, iy - 1},
					{ix, iy},
					{ix, iy + 1},
					{ix + 1, iy}
				};
				double wSum = 0.0;
				T weightedSum = T();
				for (int k = 0; k < 5; ++k)
				{
					const int i = coords[k][0];
					const int j = coords[k][1];
					if (i < 0 || i >= n_X || j < 0 || j >= n_Y) continue;
					const zPoint nPos = gridCellPosition2D(minBB, unit_X, unit_Y, valuesperVertex, i, j);
					const double dx = static_cast<double>(samplePos.x) - nPos.x;
					const double dy = static_cast<double>(samplePos.y) - nPos.y;
					const double dz = static_cast<double>(samplePos.z) - nPos.z;
					const double distSq = dx * dx + dy * dy + dz * dz;
					if (distSq <= 0.0)
					{
						fieldValue = values[i * n_Y + j];
						return true;
					}
					const double w = 1.0 / distSq;
					ValueT v = values[i * n_Y + j];
					weightedSum += v * static_cast<float>(w);
					wSum += w;
				}
				if (wSum > 0.0) weightedSum /= static_cast<float>(wSum);
				fieldValue = weightedSum;
				return true;
			}

			if (type == zFieldContainedWeighted)
			{
				const zPoint cPos = gridCellPosition2D(minBB, unit_X, unit_Y, valuesperVertex, ix, iy);
				const int stepX = (samplePos.x - cPos.x < 0.0f) ? -1 : 1;
				const int stepY = (samplePos.y - cPos.y < 0.0f) ? -1 : 1;
				const int coords[4][2] = {
					{ix, iy},
					{ix + stepX, iy},
					{ix, iy + stepY},
					{ix + stepX, iy + stepY}
				};

				double wSum = 0.0;
				T weightedSum = T();
				for (int k = 0; k < 4; ++k)
				{
					const int i = coords[k][0];
					const int j = coords[k][1];
					if (i < 0 || i >= n_X || j < 0 || j >= n_Y) continue;
					const zPoint nPos = gridCellPosition2D(minBB, unit_X, unit_Y, valuesperVertex, i, j);
					const double dx = static_cast<double>(samplePos.x) - nPos.x;
					const double dy = static_cast<double>(samplePos.y) - nPos.y;
					const double dz = static_cast<double>(samplePos.z) - nPos.z;
					const double distSq = dx * dx + dy * dy + dz * dz;
					if (distSq <= 0.0)
					{
						fieldValue = values[i * n_Y + j];
						return true;
					}
					const double w = 1.0 / distSq;
					ValueT v = values[i * n_Y + j];
					weightedSum += v * static_cast<float>(w);
					wSum += w;
				}
				if (wSum > 0.0) weightedSum /= static_cast<float>(wSum);
				fieldValue = weightedSum;
				return true;
			}

			return false;
		}

		void buildContainedNeighbours2D(
			const zPoint& pos,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			int n_X,
			int n_Y,
			bool valuesperVertex,
			zIntArray& containedNeighbour)
		{
			containedNeighbour.clear();
			int ix = -1;
			int iy = -1;
			if (!computeCellCoords2D(pos, minBB, unit_X, unit_Y, n_X, n_Y, ix, iy)) return;

			const zPoint cPos = gridCellPosition2D(minBB, unit_X, unit_Y, valuesperVertex, ix, iy);
			const int stepX = (pos.x - cPos.x < 0.0f) ? -1 : 1;
			const int stepY = (pos.y - cPos.y < 0.0f) ? -1 : 1;
			const int coords[4][2] = {
				{ix, iy},
				{ix + stepX, iy},
				{ix, iy + stepY},
				{ix + stepX, iy + stepY}
			};
			for (int k = 0; k < 4; ++k)
			{
				const int i = coords[k][0];
				const int j = coords[k][1];
				if (i >= 0 && i < n_X && j >= 0 && j < n_Y)
				{
					containedNeighbour.push_back(i * n_Y + j);
				}
			}
		}

		void buildRingNeighbours2D(int index, int numRings, int n_X, int n_Y, zIntArray& out)
		{
			out.clear();
			if (n_X <= 0 || n_Y <= 0 || index < 0 || index >= n_X * n_Y) return;
			const int ix = index / n_Y;
			const int iy = index % n_Y;
			const int iMin = std::max(0, ix - numRings);
			const int iMax = std::min(n_X - 1, ix + numRings);
			const int jMin = std::max(0, iy - numRings);
			const int jMax = std::min(n_Y - 1, iy + numRings);
			for (int i = iMin; i <= iMax; ++i)
			{
				for (int j = jMin; j <= jMax; ++j)
				{
					out.push_back(i * n_Y + j);
				}
			}
		}

		void buildAdjacentNeighbours2D(int index, int n_X, int n_Y, zIntArray& out)
		{
			out.clear();
			if (n_X <= 0 || n_Y <= 0 || index < 0 || index >= n_X * n_Y) return;
			const int ix = index / n_Y;
			const int iy = index % n_Y;
			for (int di = -1; di <= 1; ++di)
			{
				for (int dj = -1; dj <= 1; ++dj)
				{
					if (di != 0 && dj != 0) continue;
					const int i = ix + di;
					const int j = iy + dj;
					if (i >= 0 && i < n_X && j >= 0 && j < n_Y)
					{
						out.push_back(i * n_Y + j);
					}
				}
			}
		}

		void updateFaceColorsFromVertexColorsFlat(detail::zMeshFaceListStorage& data)
		{
			const int nF = data.numFaces();
			if (data.faceColors.size() != static_cast<std::size_t>(nF))
			{
				data.faceColors.resize(static_cast<std::size_t>(nF));
			}
			for (int f = 0; f < nF; ++f)
			{
				const int begin = data.faceOffsets[f];
				const int end = data.faceOffsets[f + 1];
				const int count = end - begin;
				if (count <= 0) continue;
				zColor avg(0, 0, 0, 0);
				for (int k = begin; k < end; ++k)
				{
					const zColor& c = data.vertexColors[data.faceVertexIndices[k]];
					avg.r += c.r;
					avg.g += c.g;
					avg.b += c.b;
					avg.a += c.a;
				}
				const float inv = 1.0f / static_cast<float>(count);
				avg.r *= inv;
				avg.g *= inv;
				avg.b *= inv;
				avg.a *= inv;
				avg.toHSV();
				data.faceColors[f] = avg;
			}
		}

		void computeVertexAveragesFromFaceScalarsFlat(
			detail::zMeshFaceListStorage& data,
			const zScalarArray& faceScalars,
			zScalarArray& vertexScalars,
			bool updateVertexColors)
		{
			const int nV = data.numVertices();
			const int nF = data.numFaces();
			vertexScalars.assign(static_cast<std::size_t>(nV), 0.0f);
			std::vector<int> counts(static_cast<std::size_t>(nV), 0);

			if (updateVertexColors)
			{
				data.vertexColors.assign(static_cast<std::size_t>(nV), zColor(0, 0, 0, 0));
			}

			for (int f = 0; f < nF; ++f)
			{
				const int begin = data.faceOffsets[f];
				const int end = data.faceOffsets[f + 1];
				const float s = faceScalars[f];
				const zColor fc = (updateVertexColors && f < static_cast<int>(data.faceColors.size()))
					? data.faceColors[f] : zColor();
				for (int k = begin; k < end; ++k)
				{
					const int v = data.faceVertexIndices[k];
					vertexScalars[v] += s;
					counts[v]++;
					if (updateVertexColors)
					{
						data.vertexColors[v].r += fc.r;
						data.vertexColors[v].g += fc.g;
						data.vertexColors[v].b += fc.b;
						data.vertexColors[v].a += fc.a;
					}
				}
			}

			for (int v = 0; v < nV; ++v)
			{
				if (counts[v] > 0)
				{
					const float inv = 1.0f / static_cast<float>(counts[v]);
					vertexScalars[v] *= inv;
					if (updateVertexColors)
					{
						data.vertexColors[v].r *= inv;
						data.vertexColors[v].g *= inv;
						data.vertexColors[v].b *= inv;
						data.vertexColors[v].a *= inv;
						data.vertexColors[v].toHSV();
					}
				}
			}
		}
	}

	//==========================================================================
	// zFnMeshScalarField
	//==========================================================================

	ZSPACE_INLINE zFnMeshScalarField::zFnMeshScalarField()
		: fnType(zFnType::zMeshFieldFn), setValuesperVertex(true), triMesh(true), fieldObj(nullptr)
	{
	}

	ZSPACE_INLINE zFnMeshScalarField::zFnMeshScalarField(zObjectMeshScalarField& _fieldObj)
		: fnType(zFnType::zMeshFieldFn), setValuesperVertex(true), triMesh(true), fieldObj(&_fieldObj), fnMesh(_fieldObj)
	{
	}

	ZSPACE_INLINE zFnMeshScalarField::~zFnMeshScalarField() = default;

	ZSPACE_INLINE zFnType zFnMeshScalarField::getType()
	{
		return zFnType::zMeshFieldFn;
	}

	ZSPACE_INLINE void zFnMeshScalarField::getBounds(zPoint& minBB, zPoint& maxBB)
	{
		getBoundingBox(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnMeshScalarField::clear()
	{
		fnMesh.clear();
		if (fieldObj)
		{
			zFieldObjectStorage::get(*fieldObj).fieldValues.clear();
		}
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		contourVertexValues.clear();
	}

	ZSPACE_INLINE void zFnMeshScalarField::create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField2D<zScalar>(_minBB, _maxBB, _n_X, _n_Y);
		setValuesperVertex = _setValuesperVertex;
		field.valuesperVertex = _setValuesperVertex;
		triMesh = _triMesh;
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createFieldMesh();
	}

	ZSPACE_INLINE void zFnMeshScalarField::create(double _unit_X, double _unit_Y, int _n_X, int _n_Y, zPoint _minBB, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField2D<zScalar>(_unit_X, _unit_Y, _n_X, _n_Y, _minBB);
		setValuesperVertex = _setValuesperVertex;
		field.valuesperVertex = _setValuesperVertex;
		triMesh = _triMesh;
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createFieldMesh();
	}

	ZSPACE_INLINE zObjectMesh* zFnMeshScalarField::getRawMesh()
	{
		return fieldObj;
	}

	ZSPACE_INLINE int zFnMeshScalarField::getIndex(int index_X, int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return index_X * field.n_Y + index_Y;
	}

	ZSPACE_INLINE void zFnMeshScalarField::getIndices(int index, int& index_X, int& index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		index_X = (field.n_Y > 0) ? (index / field.n_Y) : 0;
		index_Y = (field.n_Y > 0) ? (index % field.n_Y) : 0;
	}

	ZSPACE_INLINE zPoint zFnMeshScalarField::getPosition(int index) const
	{
		int ix = 0, iy = 0;
		getIndices(index, ix, iy);
		return getPosition(ix, iy);
	}

	ZSPACE_INLINE zPoint zFnMeshScalarField::getPosition(int index_X, int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return gridCellPosition2D(field.minBB, field.unit_X, field.unit_Y, field.valuesperVertex, index_X, index_Y);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildRingNeighbours2D(index, numRings, field.n_X, field.n_Y, outRingNeighbours);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildAdjacentNeighbours2D(index, field.n_X, field.n_Y, outAdjacentNeighbours);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildContainedNeighbours2D(pos, field.minBB, field.unit_X, field.unit_Y, field.n_X, field.n_Y, field.valuesperVertex, containedNeighbour);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour)
	{
		containedNeighbour.clear();
		zIntArray indices;
		getNeighbour_Contained(pos, indices);
		containedNeighbour.reserve(indices.size());
		for (int id : indices)
		{
			containedNeighbour.push_back(getPosition(id));
		}
	}

	ZSPACE_INLINE int zFnMeshScalarField::numFieldValues() const
	{
		return static_cast<int>(zFieldObjectStorage::get(*fieldObj).fieldValues.size());
	}

	ZSPACE_INLINE void zFnMeshScalarField::getResolution(int& _n_X, int& _n_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_n_X = field.n_X;
		_n_Y = field.n_Y;
	}

	ZSPACE_INLINE void zFnMeshScalarField::getUnitDistances(double& _unit_X, double& _unit_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_unit_X = field.unit_X;
		_unit_Y = field.unit_Y;
	}

	ZSPACE_INLINE void zFnMeshScalarField::getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_minBB = field.minBB;
		_maxBB = field.maxBB;
	}

	ZSPACE_INLINE void zFnMeshScalarField::getPositions(zPointArray& positions)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		if (field.valuesperVertex)
		{
			positions = zMeshObjectStorage::read(*fieldObj).positions;
			return;
		}
		const int total = field.n_X * field.n_Y;
		positions.resize(static_cast<std::size_t>(total));
		int idx = 0;
		for (int i = 0; i < field.n_X; ++i)
		{
			for (int j = 0; j < field.n_Y; ++j)
			{
				positions[idx++] = gridCellPosition2D(field.minBB, field.unit_X, field.unit_Y, false, i, j);
			}
		}
	}

	ZSPACE_INLINE zScalar zFnMeshScalarField::getFieldValue(int index) const
	{
		return zFieldObjectStorage::get(*fieldObj).fieldValues[index];
	}

	ZSPACE_INLINE bool zFnMeshScalarField::getFieldValue(zPoint& samplePos, zFieldValueType type, zScalar& fieldValue)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return sampleField2D(
			field.fieldValues.data(),
			field.fieldValues.size(),
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.n_X,
			field.n_Y,
			field.valuesperVertex,
			samplePos,
			type,
			fieldValue);
	}

	ZSPACE_INLINE bool zFnMeshScalarField::getScalarValue(zScalarArray& scalars, zPoint& samplePos, zFieldValueType type, zScalar& fieldValue)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return sampleField2D(
			scalars.data(),
			scalars.size(),
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.n_X,
			field.n_Y,
			field.valuesperVertex,
			samplePos,
			type,
			fieldValue);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getFieldValues(zScalarArray& fieldValues)
	{
		fieldValues = zFieldObjectStorage::get(*fieldObj).fieldValues;
	}

	ZSPACE_INLINE zScalar* zFnMeshScalarField::getRawFieldValues()
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (vals.empty()) throw std::invalid_argument(" error: null pointer.");
		return vals.data();
	}

	ZSPACE_INLINE zVector zFnMeshScalarField::getGradient(int index, float epsilon)
	{
		(void)epsilon;
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		const int nX = field.n_X;
		const int nY = field.n_Y;
		if (nY <= 0) return zVector();
		const int id_X = index / nY;
		const int id_Y = index % nY;
		if (id_X <= 0 || id_Y <= 0 || id_X >= nX - 1 || id_Y >= nY - 1)
		{
			return zVector();
		}
		const float invX = (field.unit_X != 0.0) ? static_cast<float>(1.0 / (2.0 * field.unit_X)) : 0.0f;
		const float invY = (field.unit_Y != 0.0) ? static_cast<float>(1.0 / (2.0 * field.unit_Y)) : 0.0f;
		const zScalar fieldVal = field.fieldValues[index];
		const zScalar fieldVal1 = field.fieldValues[(id_X + 1) * nY + id_Y];
		const zScalar fieldVal2 = field.fieldValues[index + 1];
		return zVector((fieldVal1 - fieldVal) * invX, (fieldVal2 - fieldVal) * invY, 0.0f);
	}

	ZSPACE_INLINE zVectorArray zFnMeshScalarField::getGradients(float epsilon)
	{
		(void)epsilon;
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		const int nX = field.n_X;
		const int nY = field.n_Y;
		const int total = static_cast<int>(field.fieldValues.size());
		zVectorArray out(static_cast<std::size_t>(total), zVector());
		if (nX <= 2 || nY <= 2 || total != nX * nY) return out;

		const float invX = (field.unit_X != 0.0) ? static_cast<float>(1.0 / (2.0 * field.unit_X)) : 0.0f;
		const float invY = (field.unit_Y != 0.0) ? static_cast<float>(1.0 / (2.0 * field.unit_Y)) : 0.0f;
		const zScalar* vals = field.fieldValues.data();
		zVector* dst = out.data();

		for (int i = 1; i < nX - 1; ++i)
		{
			const int rowBase = i * nY;
			const int nextRowBase = (i + 1) * nY;
			for (int j = 1; j < nY - 1; ++j)
			{
				const int idx = rowBase + j;
				const zScalar v = vals[idx];
				dst[idx] = zVector((vals[nextRowBase + j] - v) * invX, (vals[idx + 1] - v) * invY, 0.0f);
			}
		}
		return out;
	}

	ZSPACE_INLINE bool zFnMeshScalarField::getValuesPerVertexBoolean() const
	{
		return setValuesperVertex;
	}

	ZSPACE_INLINE bool zFnMeshScalarField::getTriMeshBoolean() const
	{
		return triMesh;
	}

	ZSPACE_INLINE void zFnMeshScalarField::getFieldColorDomain(zDomainColor& colDomain)
	{
		colDomain = fieldColorDomain;
	}

	ZSPACE_INLINE void zFnMeshScalarField::setFieldColorDomain(zDomainColor& colDomain)
	{
		fieldColorDomain = colDomain;
	}

	ZSPACE_INLINE void zFnMeshScalarField::setBoundingBox(zPoint& _minBB, zPoint& _maxBB)
	{
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field.minBB = _minBB;
		field.maxBB = _maxBB;
	}

	ZSPACE_INLINE void zFnMeshScalarField::setFieldValue(int index, zScalar val, bool append)
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (append) vals[index] += val;
		else vals[index] = val;
	}

	ZSPACE_INLINE void zFnMeshScalarField::setFieldValues(zScalarArray& fValues, zFieldColorType type, float sdfWidth)
	{
		auto& field = zFieldObjectStorage::get(*fieldObj);
		if (fValues.size() != field.fieldValues.size())
		{
			throw std::invalid_argument("input fValues size not field scalars.");
		}
		field.fieldValues = fValues;
		updateColors(type, sdfWidth);
	}

	//---- 2D IDW FIELD METHODS (Scalar)

	ZSPACE_INLINE void zFnMeshScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectMesh& inMeshObj, zScalar meshValue, double influence, double power, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zScalar d = 0.0f;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(const_cast<zPoint&>(inData.positions[j]));
				if (r < tempDist)
				{
					const double w = std::pow(r, power);
					wSum += w;
					const double val = (w > 0.0) ? ((r * influence) / w) : 0.0;
					d = static_cast<zScalar>(meshValue * val);
					tempDist = r;
				}
			}
			fieldValues[i] = (wSum > 0.0) ? static_cast<zScalar>(d / wSum) : 0.0f;
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectGraph& inGraphObj, zScalar graphValue, double influence, double power, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zScalar d = 0.0f;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(const_cast<zPoint&>(inData.positions[j]));
				if (r < tempDist)
				{
					const double w = std::pow(r, power);
					wSum += w;
					const double val = (w > 0.0) ? ((r * influence) / w) : 0.0;
					d = static_cast<zScalar>(graphValue * val);
					tempDist = r;
				}
			}
			fieldValues[i] = (wSum > 0.0) ? static_cast<zScalar>(d / wSum) : 0.0f;
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalar value, double influence, double power, bool normalise)
	{
		zFnPointCloud fnPts(inPointsObj);
		if (fnPts.numVertices() == 0)
		{
			fieldValues.assign(static_cast<std::size_t>(fnMesh.numVertices()), 0.0f);
			return;
		}
		zPoint* inPositions = fnPts.getRawVertexPositions();
		const int nIn = fnPts.numVertices();
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			double d = 0.0;
			double wSum = 0.0;
			zPoint gp = gridData.positions[i];

			for (int j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(inPositions[j]);
				const double w = std::pow(r, power);
				wSum += w;
				const double val = (w > 0.0) ? ((r * influence) / w) : 0.0;
				d += value * val;
			}
			fieldValues[i] = (wSum > 0.0) ? static_cast<zScalar>(d / wSum) : 0.0f;
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalarArray& values, vector<double>& influences, double power, bool normalise)
	{
		zFnPointCloud fnPts(inPointsObj);
		if (fnPts.numVertices() != static_cast<int>(values.size())) throw std::invalid_argument(" error: size of inPositions and values dont match.");
		if (fnPts.numVertices() != static_cast<int>(influences.size())) throw std::invalid_argument(" error: size of inPositions and influences dont match.");
		if (fnPts.numVertices() == 0)
		{
			fieldValues.assign(static_cast<std::size_t>(fnMesh.numVertices()), 0.0f);
			return;
		}
		zPoint* inPositions = fnPts.getRawVertexPositions();
		const int nIn = fnPts.numVertices();
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			double d = 0.0;
			double wSum = 0.0;
			zPoint gp = gridData.positions[i];

			for (int j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(inPositions[j]);
				const double w = std::pow(r, power);
				wSum += w;
				const double val = (w > 0.0) ? ((r * influences[j]) / w) : 0.0;
				d += values[j] * val;
			}
			fieldValues[i] = (wSum > 0.0) ? static_cast<zScalar>(d / wSum) : 0.0f;
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalar value, double influence, double power, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			double d = 0.0;
			double wSum = 0.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(inPositions[j]);
				const double w = std::pow(r, power);
				wSum += w;
				const double val = (w > 0.0) ? ((r * influence) / w) : 0.0;
				d += value * val;
			}
			fieldValues[i] = (wSum > 0.0) ? static_cast<zScalar>(d / wSum) : 0.0f;
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalarArray& values, vector<double>& influences, double power, bool normalise)
	{
		if (inPositions.size() != values.size()) throw std::invalid_argument(" error: size of inPositions and values dont match.");
		if (inPositions.size() != influences.size()) throw std::invalid_argument(" error: size of inPositions and influences dont match.");

		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			double d = 0.0;
			double wSum = 0.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(inPositions[j]);
				const double w = std::pow(r, power);
				wSum += w;
				const double val = (w > 0.0) ? ((r * influences[j]) / w) : 0.0;
				d += values[j] * val;
			}
			fieldValues[i] = (wSum > 0.0) ? static_cast<zScalar>(d / wSum) : 0.0f;
		}
		if (normalise) normliseValues(fieldValues);
	}

	//---- 2D SCALAR DISTANCE METHODS

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zPointArray& inPositions, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inPositions.size();
		scalars.resize(nGrid);

		const zPoint* gridPts = gridData.positions.data();
		const zPoint* srcPts = inPositions.data();

		if (nIn >= 32)
		{
			detail::FlatPointKDTree kdtree;
			kdtree.build(srcPts, nIn);
			detail::zFieldParallelFor(static_cast<int>(nGrid), 2048, [&](int begin, int end)
			{
				int hint = 0;
				for (int i = begin; i < end; ++i)
				{
					scalars[i] = kdtree.nearestDistSq(gridPts[i].x, gridPts[i].y, gridPts[i].z, hint);
				}
			});
		}
		else
		{
			detail::zFieldParallelFor(static_cast<int>(nGrid), 2048, [&](int begin, int end)
			{
				for (int i = begin; i < end; ++i)
				{
					const float gx = gridPts[i].x;
					const float gy = gridPts[i].y;
					const float gz = gridPts[i].z;
					float minDistSq = 100000000.0f;
					for (std::size_t j = 0; j < nIn; ++j)
					{
						const float dx = gx - srcPts[j].x;
						const float dy = gy - srcPts[j].y;
						const float dz = gz - srcPts[j].z;
						const float distSq = dx * dx + dy * dy + dz * dz;
						if (distSq < minDistSq) minDistSq = distSq;
					}
					scalars[i] = minDistSq;
				}
			});
		}

		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zPointArray& inPositions, float offset, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inPositions.size();
		scalars.resize(nGrid);
		if (nIn == 0)
		{
			std::fill(scalars.begin(), scalars.end(), 0.0f);
			return;
		}

		const zPoint* gridPts = gridData.positions.data();
		const zPoint* srcPts = inPositions.data();

		if (nIn >= 32)
		{
			detail::FlatPointKDTree kdtree;
			kdtree.build(srcPts, nIn);
			detail::zFieldParallelFor(static_cast<int>(nGrid), 2048, [&](int begin, int end)
			{
				int hint = 0;
				for (int i = begin; i < end; ++i)
				{
					const float minDistSq = kdtree.nearestDistSq(gridPts[i].x, gridPts[i].y, gridPts[i].z, hint);
					scalars[i] = std::sqrt(minDistSq) - offset;
				}
			});
		}
		else
		{
			detail::zFieldParallelFor(static_cast<int>(nGrid), 2048, [&](int begin, int end)
			{
				for (int i = begin; i < end; ++i)
				{
					const float gx = gridPts[i].x;
					const float gy = gridPts[i].y;
					const float gz = gridPts[i].z;
					float minDistSq = std::numeric_limits<float>::max();
					for (std::size_t j = 0; j < nIn; ++j)
					{
						const float dx = gx - srcPts[j].x;
						const float dy = gy - srcPts[j].y;
						const float dz = gz - srcPts[j].z;
						const float distSq = dx * dx + dy * dy + dz * dz;
						if (distSq < minDistSq) minDistSq = distSq;
					}
					scalars[i] = std::sqrt(minDistSq) - offset;
				}
			});
		}

		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zObjectPointCloud& inPointsObj, bool normalise)
	{
		zFnPointCloud fnPts(inPointsObj);
		const int nIn = fnPts.numVertices();
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);
		if (nIn == 0)
		{
			std::fill(scalars.begin(), scalars.end(), 0.0f);
			return;
		}

		const zPoint* gridPts = gridData.positions.data();
		const zPoint* srcPts = fnPts.getRawVertexPositions();

		std::vector<double> distVals(nGrid, 10000.0);
		double dMin = 100000.0;
		double dMax = 0.0;

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			const float gx = gridPts[i].x;
			const float gy = gridPts[i].y;
			const float gz = gridPts[i].z;
			double minSq = 10000.0;
			for (int j = 0; j < nIn; ++j)
			{
				const double dx = static_cast<double>(gx) - srcPts[j].x;
				const double dy = static_cast<double>(gy) - srcPts[j].y;
				const double dz = static_cast<double>(gz) - srcPts[j].z;
				const double distSq = dx * dx + dy * dy + dz * dz;
				if (distSq < minSq) minSq = distSq;
			}
			distVals[i] = minSq;
			if (minSq < dMin) dMin = minSq;
			if (minSq > dMax) dMax = minSq;
		}

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			scalars[i] = static_cast<zScalar>(coreUtils.ofMap(distVals[i], dMin, dMax, 0.0, 1.0));
		}

		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zObjectPointCloud& inPointsObj, float offset, bool normalise)
	{
		zFnPointCloud fnPts(inPointsObj);
		getScalarsAsVertexDistance(scalars, inPointsObj.impl->storage.vertexPositions, offset, normalise);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, float offset, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		zPointArray& nonConstPts = const_cast<zPointArray&>(inData.positions);
		getScalarsAsVertexDistance(scalars, nonConstPts, offset, normalise);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, float offset, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		zPointArray& nonConstPts = const_cast<zPointArray&>(inData.positions);
		getScalarsAsVertexDistance(scalars, nonConstPts, offset, normalise);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, float offset, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const int nEdges = inData.numEdges();
		scalars.resize(nGrid);

		std::vector<EdgeSegment2D> segments;
		segments.reserve(static_cast<std::size_t>(nEdges));
		for (int e = 0; e < nEdges; ++e)
		{
			const int v0 = inData.edgeVertexIndices[e * 2];
			const int v1 = inData.edgeVertexIndices[e * 2 + 1];
			zPoint p0 = inData.positions[v0];
			zPoint p1 = inData.positions[v1];
			const zVector ab = p1 - p0;
			const double ab2 = static_cast<double>(ab.x) * ab.x +
				static_cast<double>(ab.y) * ab.y +
				static_cast<double>(ab.z) * ab.z;
			segments.push_back({ p0, ab, ab2 });
		}

		const zPoint* gridPts = gridData.positions.data();
		for (std::size_t i = 0; i < nGrid; ++i)
		{
			if (segments.empty())
			{
				scalars[i] = 0.0f;
				continue;
			}
			const zPoint& pt = gridPts[i];
			double minDistSq = std::numeric_limits<double>::max();
			for (const auto& seg : segments)
			{
				double t = 0.0;
				if (seg.ab2 > 1.0e-12)
				{
					const double apx = static_cast<double>(pt.x) - seg.p0.x;
					const double apy = static_cast<double>(pt.y) - seg.p0.y;
					const double apz = static_cast<double>(pt.z) - seg.p0.z;
					t = (apx * seg.ab.x + apy * seg.ab.y + apz * seg.ab.z) / seg.ab2;
					if (t < 0.0) t = 0.0;
					else if (t > 1.0) t = 1.0;
				}
				const double cx = seg.p0.x + seg.ab.x * t - pt.x;
				const double cy = seg.p0.y + seg.ab.y * t - pt.y;
				const double cz = seg.p0.z + seg.ab.z * t - pt.z;
				const double distSq = cx * cx + cy * cy + cz * cz;
				if (distSq < minDistSq) minDistSq = distSq;
			}
			double minDist = std::sqrt(minDistSq);
			if (minDist < 1.0e-6) minDist = 0.0;
			scalars[i] = static_cast<zScalar>(minDist - offset);
		}

		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, float offset, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const int nEdges = inData.numEdges();
		scalars.resize(nGrid);

		std::vector<EdgeSegment2D> segments;
		segments.reserve(static_cast<std::size_t>(nEdges));
		for (int e = 0; e < nEdges; ++e)
		{
			const int v0 = inData.edgeVertexIndices[e * 2];
			const int v1 = inData.edgeVertexIndices[e * 2 + 1];
			zPoint p0 = inData.positions[v0];
			zPoint p1 = inData.positions[v1];
			const zVector ab = p1 - p0;
			const double ab2 = static_cast<double>(ab.x) * ab.x +
				static_cast<double>(ab.y) * ab.y +
				static_cast<double>(ab.z) * ab.z;
			if (ab2 < static_cast<double>(EPS) * static_cast<double>(EPS)) continue;
			segments.push_back({ p0, ab, ab2 });
		}

		const zPoint* gridPts = gridData.positions.data();
		for (std::size_t i = 0; i < nGrid; ++i)
		{
			if (segments.empty())
			{
				scalars[i] = 0.0f;
				continue;
			}
			const zPoint& pt = gridPts[i];
			double minDistSq = std::numeric_limits<double>::max();
			for (const auto& seg : segments)
			{
				const double apx = static_cast<double>(pt.x) - seg.p0.x;
				const double apy = static_cast<double>(pt.y) - seg.p0.y;
				const double apz = static_cast<double>(pt.z) - seg.p0.z;
				double t = (apx * seg.ab.x + apy * seg.ab.y + apz * seg.ab.z) / seg.ab2;
				if (t < 0.0) t = 0.0;
				else if (t > 1.0) t = 1.0;

				const double cx = seg.p0.x + seg.ab.x * t - pt.x;
				const double cy = seg.p0.y + seg.ab.y * t - pt.y;
				const double cz = seg.p0.z + seg.ab.z * t - pt.z;
				const double distSq = cx * cx + cy * cy + cz * cz;
				if (distSq < minDistSq) minDistSq = distSq;
			}
			double minDist = std::sqrt(minDistSq);
			if (minDist < 1.0e-6) minDist = 0.0;
			scalars[i] = static_cast<zScalar>(minDist - offset);
		}

		if (normalise) normliseValues(scalars);
	}

	//---- 2D SDF PRIMITIVE METHODS

	ZSPACE_INLINE void zFnMeshScalarField::getScalars_Polygon(zScalarArray& scalars, zObjectGraph& inGraphObj, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);

		struct PolySeg2D
		{
			zPoint vi;
			zPoint vj;
			zVector e;
			float invEE;
		};
		std::vector<PolySeg2D> polySegs;
		zItGraphVertex v(inGraphObj, 0);
		zItGraphHalfEdge he = v.getHalfEdge();
		zItGraphHalfEdge start = he;
		zPoint v0 = he.getVertex().getPosition();
		do
		{
			zPoint vj = he.getStartVertex().getPosition();
			zPoint vi = he.getVertex().getPosition();
			zVector e = vj - vi;
			const float ee = e * e;
			polySegs.push_back({ vi, vj, e, (ee > 1.0e-12f) ? (1.0f / ee) : 0.0f });
			he = he.getNext();
		} while (he != start);

		const zPoint* gridPts = gridData.positions.data();
		detail::zFieldParallelFor(static_cast<int>(nGrid), 2048, [&](int begin, int end)
		{
			for (int i = begin; i < end; ++i)
			{
				zPoint p = gridPts[i];
				zVector dv0 = p - v0;
				float d = dv0 * dv0;
				float s = 1.0f;
				for (const auto& seg : polySegs)
				{
					const float wx = p.x - seg.vi.x;
					const float wy = p.y - seg.vi.y;
					const float wz = p.z - seg.vi.z;
					const float t = coreUtils.ofClamp<float>((wx * seg.e.x + wy * seg.e.y + wz * seg.e.z) * seg.invEE, 0.0f, 1.0f);
					const float bx = wx - seg.e.x * t;
					const float by = wy - seg.e.y * t;
					const float bz = wz - seg.e.z * t;
					const float bb = bx * bx + by * by + bz * bz;
					if (bb < d) d = bb;

					const bool c1 = (p.y >= seg.vi.y);
					const bool c2 = (p.y < seg.vj.y);
					const bool c3 = (seg.e.x * wy > seg.e.y * wx);
					if ((c1 && c2 && c3) || (!c1 && !c2 && !c3)) s = -s;
				}
				scalars[i] = s * std::sqrt(d);
			}
		});
		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalars_Circle(zScalarArray& scalars, zVector& cen, float r, double annularVal, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);
		cen.z = 0;

		const zPoint* gridPts = gridData.positions.data();
		const float cx = cen.x;
		const float cy = cen.y;
		const float cz = cen.z;
		if (annularVal == 0.0)
		{
			detail::zFieldParallelFor(static_cast<int>(nGrid), 4096, [&](int begin, int end)
			{
				for (int i = begin; i < end; ++i)
				{
					const float dx = gridPts[i].x - cx;
					const float dy = gridPts[i].y - cy;
					const float dz = gridPts[i].z - cz;
					scalars[i] = std::sqrt(dx * dx + dy * dy + dz * dz) - r;
				}
			});
		}
		else
		{
			const float annularF = static_cast<float>(annularVal);
			detail::zFieldParallelFor(static_cast<int>(nGrid), 4096, [&](int begin, int end)
			{
				for (int i = begin; i < end; ++i)
				{
					const float dx = gridPts[i].x - cx;
					const float dy = gridPts[i].y - cy;
					const float dz = gridPts[i].z - cz;
					scalars[i] = std::abs((std::sqrt(dx * dx + dy * dy + dz * dz) - r) - annularF);
				}
			});
		}
		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalars_Ellipse(zScalarArray& scalars, zVector& cen, float a, float b, double annularVal, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);
		cen.z = 0;

		const zPoint* gridPts = gridData.positions.data();
		detail::zFieldParallelFor(static_cast<int>(nGrid), 4096, [&](int begin, int end)
		{
			for (int i = begin; i < end; ++i)
			{
				zPoint p = gridPts[i];
				if (annularVal == 0.0) scalars[i] = getScalar_Ellipse(cen, p, a, b);
				else scalars[i] = static_cast<zScalar>(std::abs(getScalar_Ellipse(cen, p, a, b) - annularVal));
			}
		});
		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalars_Line(zScalarArray& scalars, zVector& v0, zVector& v1, double annularVal, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);

		const zPoint* gridPts = gridData.positions.data();
		detail::zFieldParallelFor(static_cast<int>(nGrid), 4096, [&](int begin, int end)
		{
			for (int i = begin; i < end; ++i)
			{
				zPoint p = gridPts[i];
				if (annularVal == 0.0) scalars[i] = getScalar_Line(p, v0, v1);
				else scalars[i] = static_cast<zScalar>(std::abs(getScalar_Line(p, v0, v1) - annularVal));
			}
		});
		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalars_Triangle(zScalarArray& scalars, zPoint& p0, zPoint& p1, zPoint& p2, double annularVal, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);
		p0.z = 0; p1.z = 0; p2.z = 0;

		const zPoint* gridPts = gridData.positions.data();
		detail::zFieldParallelFor(static_cast<int>(nGrid), 4096, [&](int begin, int end)
		{
			for (int i = begin; i < end; ++i)
			{
				zPoint p = gridPts[i];
				if (annularVal == 0.0) scalars[i] = getScalar_Triangle(p, p0, p1, p2);
				else scalars[i] = static_cast<zScalar>(std::abs(getScalar_Triangle(p, p0, p1, p2) - annularVal));
			}
		});
		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalars_Square(zScalarArray& scalars, zVector& cen, zVector& dimensions, float annularVal, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);
		cen.z = 0;

		const zPoint* gridPts = gridData.positions.data();
		if (annularVal == 0.0f)
		{
			detail::zFieldParallelFor(static_cast<int>(nGrid), 4096, [&](int begin, int end)
			{
				for (int i = begin; i < end; ++i)
				{
					zPoint p = gridPts[i];
					scalars[i] = getScalar_Square(p, cen, dimensions);
				}
			});
		}
		else
		{
			detail::zFieldParallelFor(static_cast<int>(nGrid), 4096, [&](int begin, int end)
			{
				for (int i = begin; i < end; ++i)
				{
					zPoint p = gridPts[i];
					scalars[i] = std::abs(getScalar_Square(p, cen, dimensions) - annularVal);
				}
			});
		}
		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getScalars_Trapezoid(zScalarArray& scalars, float r1, float r2, float he, float annularVal, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		scalars.resize(nGrid);

		const zPoint* gridPts = gridData.positions.data();
		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zPoint p = gridPts[i];
			if (annularVal == 0.0f) scalars[i] = static_cast<zScalar>(getScalar_Trapezoid(p, r1, r2, he));
			else scalars[i] = static_cast<zScalar>(std::abs(getScalar_Trapezoid(p, r1, r2, he) - annularVal));
		}
		if (normalise) normliseValues(scalars);
	}

	//---- COMPUTE & SMOOTH METHODS

	ZSPACE_INLINE bool zFnMeshScalarField::checkPositionBounds(zPoint& pos, int& index)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		int ix = -1, iy = -1;
		const bool inBounds = computeCellCoords2D(pos, field.minBB, field.unit_X, field.unit_Y, field.n_X, field.n_Y, ix, iy);
		index = ix * field.n_Y + iy;
		return inBounds;
	}

	ZSPACE_INLINE bool zFnMeshScalarField::checkBounds_X(int index_X) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_X >= 0 && index_X < field.n_X);
	}

	ZSPACE_INLINE bool zFnMeshScalarField::checkBounds_Y(int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_Y >= 0 && index_Y < field.n_Y);
	}

	ZSPACE_INLINE void zFnMeshScalarField::computeDomain(zScalarArray& values, zDomain<zScalar>& domain)
	{
		if (values.empty())
		{
			domain.min = 0.0f;
			domain.max = 0.0f;
			return;
		}
		auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());
		domain.min = *minIt;
		domain.max = *maxIt;
	}

	ZSPACE_INLINE void zFnMeshScalarField::normliseValues(zScalarArray& fieldValues)
	{
		zDomainFloat d;
		computeDomain(fieldValues, d);

		zDomainFloat outNeg(-1.0f, 0.0f);
		zDomainFloat outPos(0.0f, 1.0f);

		zDomainFloat inNeg(d.min, 0.0f);
		zDomainFloat inPos(0.0f, d.max);

		const std::size_t n = fieldValues.size();
		for (std::size_t i = 0; i < n; ++i)
		{
			if (fieldValues[i] < 0.0f) fieldValues[i] = coreUtils.ofMap(fieldValues[i], inNeg, outNeg);
			else fieldValues[i] = coreUtils.ofMap(fieldValues[i], inPos, outPos);
		}
	}

	ZSPACE_INLINE void zFnMeshScalarField::smoothField(zScalarArray& scalars, int numSmooth, double diffuseDamp, zDiffusionType type)
	{
		if (numSmooth <= 0 || scalars.empty()) return;
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		const int nX = field.n_X;
		const int nY = field.n_Y;
		if (static_cast<int>(scalars.size()) != nX * nY) return;

		zScalarArray tempValues(scalars.size());
		const float damp = static_cast<float>(diffuseDamp);

		for (int step = 0; step < numSmooth; ++step)
		{
			const zScalar* src = scalars.data();
			zScalar* dst = tempValues.data();

			if (type == zAverage)
			{
				detail::zFieldParallelFor(nX, 16, [&](int iBegin, int iEnd)
				{
					for (int i = iBegin; i < iEnd; ++i)
					{
						const int iMin = std::max(0, i - 1);
						const int iMax = std::min(nX - 1, i + 1);
						for (int j = 0; j < nY; ++j)
						{
							const int jMin = std::max(0, j - 1);
							const int jMax = std::min(nY - 1, j + 1);
							float sum = 0.0f;
							int count = 0;
							for (int ii = iMin; ii <= iMax; ++ii)
							{
								const zScalar* row = src + ii * nY;
								for (int jj = jMin; jj <= jMax; ++jj)
								{
									sum += row[jj];
									++count;
								}
							}
							dst[i * nY + j] = (sum != 0.0f && count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
						}
					}
				});
			}
			else if (type == zLaplacian)
			{
				detail::zFieldParallelFor(nX, 16, [&](int iBegin, int iEnd)
				{
					for (int i = iBegin; i < iEnd; ++i)
					{
						const int iMin = std::max(0, i - 1);
						const int iMax = std::min(nX - 1, i + 1);
						for (int j = 0; j < nY; ++j)
						{
							const int jMin = std::max(0, j - 1);
							const int jMax = std::min(nY - 1, j + 1);
							const int centerId = i * nY + j;
							float lapA = 0.0f;
							for (int ii = iMin; ii <= iMax; ++ii)
							{
								const int rowBase = ii * nY;
								for (int jj = jMin; jj <= jMax; ++jj)
								{
									const int id = rowBase + jj;
									if (id != centerId) lapA += src[id];
									else lapA += src[id] * -8.0f;
								}
							}
							dst[centerId] = src[centerId] + lapA * damp;
						}
					}
				});
			}

			scalars.swap(tempValues);
		}
	}

	ZSPACE_INLINE void zFnMeshScalarField::computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions)
	{
		fieldIndexPositions.clear();
		fieldIndexPositions.resize(static_cast<std::size_t>(numFieldValues()));
		for (std::size_t i = 0; i < positions.size(); ++i)
		{
			int fieldIndex = -1;
			if (checkPositionBounds(positions[i], fieldIndex) && fieldIndex >= 0 && fieldIndex < numFieldValues())
			{
				fieldIndexPositions[fieldIndex].push_back(positions[i]);
			}
		}
	}

	ZSPACE_INLINE void zFnMeshScalarField::computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies)
	{
		fieldIndexPositionIndicies.clear();
		fieldIndexPositionIndicies.resize(static_cast<std::size_t>(numFieldValues()));
		for (std::size_t i = 0; i < positions.size(); ++i)
		{
			int fieldIndex = -1;
			if (checkPositionBounds(positions[i], fieldIndex) && fieldIndex >= 0 && fieldIndex < numFieldValues())
			{
				fieldIndexPositionIndicies[fieldIndex].push_back(static_cast<int>(i));
			}
		}
	}

	ZSPACE_INLINE double zFnMeshScalarField::F_of_r(double& r, double& a, double& b)
	{
		if (0 <= r && r <= b / 3.0) return (a * (1.0 - (3.0 * r * r) / (b * b)));
		if (b / 3.0 <= r && r <= b) return (3 * a / 2 * std::pow(1.0 - (r / b), 2.0));
		return 0.0;
	}

	//---- BLEND & BOOLEAN METHODS

	ZSPACE_INLINE void zFnMeshScalarField::blend_linear(int currentFrame, int totalFrames, zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result)
	{
		float weight = static_cast<float>(currentFrame) / static_cast<float>(totalFrames);
		if (currentFrame <= 0) weight = 0.0f;
		if (currentFrame >= totalFrames) weight = 1.0f;

		const std::size_t n = fieldValues_A.size();
		fieldValues_Result.resize(n);
		const float oneMinusW = 1.0f - weight;
		for (std::size_t i = 0; i < n; ++i)
		{
			fieldValues_Result[i] = oneMinusW * fieldValues_A[i] + weight * fieldValues_B[i];
		}
	}

	ZSPACE_INLINE void zFnMeshScalarField::boolean_union(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		const std::size_t n = fieldValues_A.size();
		fieldValues_Result.resize(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			fieldValues_Result[i] = std::min(fieldValues_A[i], fieldValues_B[i]);
		}
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnMeshScalarField::boolean_subtract(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		const std::size_t n = fieldValues_A.size();
		fieldValues_Result.resize(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			fieldValues_Result[i] = std::max(fieldValues_A[i], -fieldValues_B[i]);
		}
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnMeshScalarField::boolean_intersect(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		const std::size_t n = fieldValues_A.size();
		fieldValues_Result.resize(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			fieldValues_Result[i] = std::max(fieldValues_A[i], fieldValues_B[i]);
		}
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnMeshScalarField::boolean_difference(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		zScalarArray AUnionB;
		boolean_union(fieldValues_A, fieldValues_B, AUnionB, normalise);

		zScalarArray AIntersectB;
		boolean_intersect(fieldValues_B, fieldValues_A, AIntersectB, normalise);

		boolean_subtract(AUnionB, AIntersectB, fieldValues_Result, normalise);
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnMeshScalarField::boolean_clipwithPlane(zScalarArray& scalars, zPlane& clipPlane)
	{
		const auto& data = zMeshObjectStorage::read(*fieldObj);
		zVector O(clipPlane(3, 0), clipPlane(3, 1), clipPlane(3, 2));
		zVector Z(clipPlane(2, 0), clipPlane(2, 1), clipPlane(2, 2));
		const double zLen = Z.length();
		if (zLen <= 1.0e-12) return;

		const std::size_t n = std::min(scalars.size(), data.positions.size());
		for (std::size_t i = 0; i < n; ++i)
		{
			zPoint pos = data.positions[i];
			zVector A = pos - O;
			const double minDist_Plane = (A * Z) / zLen;
			if (minDist_Plane > 0.0)
			{
				scalars[i] = 1.0f;
			}
		}
	}

	ZSPACE_INLINE void zFnMeshScalarField::boolean_clipwithPlane(zScalarArray& fieldValues_A, zScalarArray& fieldValues_Result, zPoint& O, zVector& Z)
	{
		const auto& data = zMeshObjectStorage::read(*fieldObj);
		const std::size_t n = data.positions.size();
		zScalarArray temp(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			zPoint p = data.positions[i];
			temp[i] = static_cast<zScalar>(coreUtils.minDist_Point_Plane(p, O, Z));
		}
		boolean_subtract(fieldValues_A, temp, fieldValues_Result, false);
	}

	//---- UPDATE & CONTOUR METHODS

	ZSPACE_INLINE void zFnMeshScalarField::updateColors(zFieldColorType type, float sdfWidth)
	{
		const auto& scalars = zFieldObjectStorage::get(*fieldObj).fieldValues;
		auto& data = zMeshObjectStorage::edit(*fieldObj);
		const std::size_t nV = data.positions.size();
		const std::size_t nF = static_cast<std::size_t>(data.numFaces());

		if (scalars.size() != nV && scalars.size() != nF) return;

		zScalarArray& mutableScalars = zFieldObjectStorage::get(*fieldObj).fieldValues;
		computeDomain(mutableScalars, contourValueDomain);

		if (contourValueDomain.min == contourValueDomain.max)
		{
			const zColor col(0.5f, 0.5f, 0.5f);
			if (scalars.size() == nV)
			{
				data.vertexColors.assign(nV, col);
				contourVertexValues = scalars;
				updateFaceColorsFromVertexColorsFlat(data);
			}
			else
			{
				data.faceColors.assign(nF, col);
				computeVertexAveragesFromFaceScalarsFlat(data, scalars, contourVertexValues, true);
				computeDomain(contourVertexValues, contourValueDomain);
			}
			return;
		}

		fieldColorDomain.min.toHSV();
		fieldColorDomain.max.toHSV();

		if (scalars.size() == nV && data.vertexColors.size() != nV) data.vertexColors.resize(nV);
		if (scalars.size() == nF && data.faceColors.size() != nF) data.faceColors.resize(nF);
		zColor* cols = (scalars.size() == nF) ? data.faceColors.data() : data.vertexColors.data();

		const float maxAbsValue = std::max(std::abs(contourValueDomain.min), std::abs(contourValueDomain.max));
		const float zeroBand = sdfWidth * maxAbsValue;
		const float domSpan = contourValueDomain.max - contourValueDomain.min;
		const float invDomSpan = (domSpan != 0.0f) ? (1.0f / domSpan) : 0.0f;

		const std::size_t count = scalars.size();
		if (type == zFieldColorType::zFieldSDF)
		{
			for (std::size_t i = 0; i < count; ++i)
			{
				if (scalars[i] < -zeroBand) cols[i] = zGREY;
				else if (scalars[i] > zeroBand) cols[i] = zWHITE;
				else cols[i] = zMAGENTA;
			}
		}
		else if (type == zFieldColorType::zFieldSlime)
		{
			for (std::size_t i = 0; i < count; ++i)
			{
				if (scalars[i] < contourValueDomain.min) cols[i] = fieldColorDomain.min;
				else if (scalars[i] > contourValueDomain.max) cols[i] = fieldColorDomain.max;
				else cols[i] = detail::blendHSVPrecomputed(scalars[i], contourValueDomain.min, invDomSpan, fieldColorDomain.min, fieldColorDomain.max);
			}
		}
		else
		{
			for (std::size_t i = 0; i < count; ++i)
			{
				cols[i] = detail::blendHSVPrecomputed(scalars[i], contourValueDomain.min, invDomSpan, fieldColorDomain.min, fieldColorDomain.max);
			}
		}

		if (scalars.size() == nF)
		{
			computeVertexAveragesFromFaceScalarsFlat(data, scalars, contourVertexValues, true);
			computeDomain(contourVertexValues, contourValueDomain);
		}
		else
		{
			contourVertexValues = scalars;
			updateFaceColorsFromVertexColorsFlat(data);
		}
	}

	ZSPACE_INLINE void zFnMeshScalarField::getIsocontour(zObjectGraph& coutourGraphObj, float inThreshold, int precision, float distTolerance)
	{
		const auto& fieldValues = zFieldObjectStorage::get(*fieldObj).fieldValues;
		auto& data = zMeshObjectStorage::edit(*fieldObj);

		zScalarArray vertexValues;
		if (data.positions.size() == fieldValues.size())
		{
			vertexValues = fieldValues;
		}
		else if (static_cast<std::size_t>(data.numFaces()) == fieldValues.size())
		{
			computeVertexAveragesFromFaceScalarsFlat(data, fieldValues, vertexValues, false);
		}
		else
		{
			throw std::invalid_argument(" error: invalid contour condition. Field values must match mesh vertices or mesh faces.");
		}

		zPointArray pos;
		zIntArray edgeConnects;
		zColorArray vColors;
		fnMesh.getIsoContour(vertexValues, inThreshold, pos, edgeConnects, vColors, precision, distTolerance);

		if (!pos.empty() && !edgeConnects.empty())
		{
			zGraphObjectStorage::set(coutourGraphObj, pos, edgeConnects);
			auto& gData = zGraphObjectStorage::edit(coutourGraphObj);
			if (vColors.size() == gData.positions.size())
			{
				gData.vertexColors = std::move(vColors);
			}
		}
		else
		{
			zGraphObjectStorage::edit(coutourGraphObj).clear();
		}
	}

	ZSPACE_INLINE void zFnMeshScalarField::getIsolineMesh(zObjectMesh& coutourMeshObj, float inThreshold, bool invertMesh)
	{
		const auto& fieldValues = zFieldObjectStorage::get(*fieldObj).fieldValues;
		auto& data = zMeshObjectStorage::edit(*fieldObj);

		zScalarArray vertexValues;
		if (data.positions.size() == fieldValues.size())
		{
			vertexValues = fieldValues;
		}
		else if (static_cast<std::size_t>(data.numFaces()) == fieldValues.size())
		{
			computeVertexAveragesFromFaceScalarsFlat(data, fieldValues, vertexValues, false);
		}
		else
		{
			throw std::invalid_argument(" error: invalid contour condition. Field values must match mesh vertices or mesh faces.");
		}

		fnMesh.getIsoMesh(vertexValues, inThreshold, invertMesh, coutourMeshObj);
	}

	ZSPACE_INLINE void zFnMeshScalarField::getIsobandMesh(zObjectMesh& coutourMeshObj, float inThresholdLow, float inThresholdHigh)
	{
		const auto& fieldValues = zFieldObjectStorage::get(*fieldObj).fieldValues;
		auto& data = zMeshObjectStorage::edit(*fieldObj);

		zScalarArray vertexValues;
		if (data.positions.size() == fieldValues.size())
		{
			vertexValues = fieldValues;
		}
		else if (static_cast<std::size_t>(data.numFaces()) == fieldValues.size())
		{
			computeVertexAveragesFromFaceScalarsFlat(data, fieldValues, vertexValues, false);
		}
		else
		{
			throw std::invalid_argument(" error: invalid contour condition. Field values must match mesh vertices or mesh faces.");
		}

		fnMesh.getIsobandMesh(vertexValues, inThresholdLow, inThresholdHigh, coutourMeshObj);
	}

	ZSPACE_INLINE void zFnMeshScalarField::createFieldMesh()
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		zFieldObjectStorage::populateGridMesh2D(
			*fieldObj,
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.n_X,
			field.n_Y,
			setValuesperVertex,
			triMesh);
	}

	//---- PROTECTED SCALAR SDF HELPERS

	ZSPACE_INLINE float zFnMeshScalarField::getScalar_Polygon(zObjectGraph& inGraphObj, zPoint& p)
	{
		zItGraphVertex v(inGraphObj, 0);
		zItGraphHalfEdge he = v.getHalfEdge();
		zItGraphHalfEdge start = he;
		zPoint v0 = he.getVertex().getPosition();

		float d = (p - v0) * (p - v0);
		float s = 1.0f;

		do
		{
			zPoint vj = he.getStartVertex().getPosition();
			zPoint vi = he.getVertex().getPosition();
			zVector e = vj - vi;
			zVector w = p - vi;
			zVector b = w - e * coreUtils.ofClamp<float>((w * e) / (e * e), 0.0f, 1.0f);
			d = coreUtils.zMin(d, (b * b));

			const bool c1 = (p.y >= vi.y);
			const bool c2 = (p.y < vj.y);
			const bool c3 = (e.x * w.y > e.y * w.x);

			if (c1 && c2 && c3) s *= -1.0f;
			if (!c1 && !c2 && !c3) s *= -1.0f;

			he = he.getNext();
		} while (he != start);

		return s * std::sqrt(d);
	}

	ZSPACE_INLINE float zFnMeshScalarField::getScalar_Circle(zPoint& cen, zPoint& p, float r)
	{
		return ((p - cen).length() - r);
	}

	ZSPACE_INLINE float zFnMeshScalarField::getScalar_Ellipse(zPoint& cen, zPoint& p, float a, float b)
	{
		zPoint localP = p - cen;
		if (std::abs(a - b) < 1.0e-6f)
		{
			return localP.length() - a;
		}
		localP.x = std::abs(localP.x);
		localP.y = std::abs(localP.y);
		localP.z = 0.0f;
		zVector ab(a, b, 0.0f);

		if (localP.x > localP.y)
		{
			std::swap(localP.x, localP.y);
			std::swap(ab.x, ab.y);
		}

		const float l = ab.y * ab.y - ab.x * ab.x;
		const float m = ab.x * localP.x / l;      const float m2 = m * m;
		const float n = ab.y * localP.y / l;      const float n2 = n * n;
		const float c = (m2 + n2 - 1.0f) / 3.0f; const float c3 = c * c * c;
		const float q = c3 + m2 * n2 * 2.0f;
		const float d = c3 + m2 * n2;
		const float g = m + m * n2;
		float co;
		if (d < 0.0f)
		{
			const float h = std::acos(coreUtils.ofClamp(q / c3, -1.0f, 1.0f)) / 3.0f;
			const float s = std::cos(h);
			const float t = std::sin(h) * std::sqrt(3.0f);
			const float rx = std::sqrt(std::max(0.0f, -c * (s + t + 2.0f) + m2));
			const float ry = std::sqrt(std::max(0.0f, -c * (s - t + 2.0f) + m2));
			co = (ry + coreUtils.zSign<float>(l) * rx + std::abs(g) / (rx * ry) - m) / 2.0f;
		}
		else
		{
			const float h = 2.0f * m * n * std::sqrt(d);
			const float s = coreUtils.zSign<float>(q + h) * std::pow(std::abs(q + h), 1.0f / 3.0f);
			const float u = coreUtils.zSign<float>(q - h) * std::pow(std::abs(q - h), 1.0f / 3.0f);
			const float rx = -s - u - c * 4.0f + 2.0f * m2;
			const float ry = (s - u) * std::sqrt(3.0f);
			const float rm = std::sqrt(rx * rx + ry * ry);
			co = (ry / std::sqrt(std::max(1.0e-12f, rm - rx)) + 2.0f * g / rm - m) / 2.0f;
		}

		co = coreUtils.ofClamp(co, -1.0f, 1.0f);
		zVector r(ab.x * co, ab.y * std::sqrt(std::max(0.0f, 1.0f - co * co)), 0.0f);

		return (r - localP).length() * coreUtils.zSign(localP.y - r.y);
	}

	ZSPACE_INLINE float zFnMeshScalarField::getScalar_Line(zPoint& p, zPoint& v0, zPoint& v1)
	{
		zVector pa = p - v0;
		zVector ba = v1 - v0;
		const float h = coreUtils.ofClamp((pa * ba) / (ba * ba), 0.0f, 1.0f);
		zVector out = pa - (ba * h);
		return out.length();
	}

	ZSPACE_INLINE float zFnMeshScalarField::getScalar_Triangle(zPoint& p, zPoint& p0, zPoint& p1, zPoint& p2)
	{
		zVector e0 = p1 - p0, e1 = p2 - p1, e2 = p0 - p2;
		zVector v0 = p - p0, v1 = p - p1, v2 = p - p2;

		zVector pq0 = v0 - e0 * coreUtils.ofClamp((v0 * e0) / (e0 * e0), 0.0f, 1.0f);
		zVector pq1 = v1 - e1 * coreUtils.ofClamp((v1 * e1) / (e1 * e1), 0.0f, 1.0f);
		zVector pq2 = v2 - e2 * coreUtils.ofClamp((v2 * e2) / (e2 * e2), 0.0f, 1.0f);

		const float s = coreUtils.zSign(e0.x * e2.y - e0.y * e2.x);

		zVector d = coreUtils.zMin(coreUtils.zMin(zVector((pq0 * pq0), s * (v0.x * e0.y - v0.y * e0.x), 0),
			zVector((pq1 * pq1), s * (v1.x * e1.y - v1.y * e1.x), 0)),
			zVector((pq2 * pq2), s * (v2.x * e2.y - v2.y * e2.x), 0));

		return -std::sqrt(d.x) * coreUtils.zSign(d.y);
	}

	ZSPACE_INLINE float zFnMeshScalarField::getScalar_Square(zPoint& p, zVector& cen, zVector& dimensions)
	{
		const float dx = std::abs(p.x - cen.x) - dimensions.x;
		const float dy = std::abs(p.y - cen.y) - dimensions.y;
		const float ox = std::max(dx, 0.0f);
		const float oy = std::max(dy, 0.0f);
		return std::sqrt(ox * ox + oy * oy) + std::min(std::max(dx, dy), 0.0f);
	}

	ZSPACE_INLINE double zFnMeshScalarField::getScalar_Trapezoid(zPoint& p, float& r1, float& r2, float& he)
	{
		zVector k1 = zVector(r2, he, 0);
		zVector k2 = zVector((r2 - r1), (2.0f * he), 0);

		p.x = std::abs(p.x);
		zVector ca = zVector(p.x - coreUtils.zMin(p.x, (p.y < 0.0f) ? r1 : r2), std::abs(p.y) - he, 0.0f);
		zVector cb = p - k1 + k2 * coreUtils.ofClamp(((k1 - p) * k2) / (k2 * k2), 0.0f, 1.0f);

		const double s = (cb.x < 0.0f && ca.y < 0.0f) ? -1.0 : 1.0;
		return s * std::sqrt(coreUtils.zMin((ca * ca), (cb * cb)));
	}

	//==========================================================================
	// zFnMeshVectorField
	//==========================================================================

	ZSPACE_INLINE zFnMeshVectorField::zFnMeshVectorField()
		: fnType(zFnType::zMeshFieldFn), setValuesperVertex(true), triMesh(true), fieldObj(nullptr)
	{
	}

	ZSPACE_INLINE zFnMeshVectorField::zFnMeshVectorField(zObjectMeshVectorField& _fieldObj)
		: fnType(zFnType::zMeshFieldFn), setValuesperVertex(true), triMesh(true), fieldObj(&_fieldObj), fnMesh(_fieldObj)
	{
	}

	ZSPACE_INLINE zFnMeshVectorField::~zFnMeshVectorField() = default;

	ZSPACE_INLINE zFnType zFnMeshVectorField::getType()
	{
		return zFnType::zMeshFieldFn;
	}

	ZSPACE_INLINE void zFnMeshVectorField::getBounds(zPoint& minBB, zPoint& maxBB)
	{
		getBoundingBox(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnMeshVectorField::clear()
	{
		fnMesh.clear();
		if (fieldObj)
		{
			zFieldObjectStorage::get(*fieldObj).fieldValues.clear();
		}
		ringNeighbours.clear();
		adjacentNeighbours.clear();
	}

	ZSPACE_INLINE void zFnMeshVectorField::create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField2D<zVector>(_minBB, _maxBB, _n_X, _n_Y);
		setValuesperVertex = _setValuesperVertex;
		field.valuesperVertex = _setValuesperVertex;
		triMesh = _triMesh;
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createFieldMesh();
	}

	ZSPACE_INLINE void zFnMeshVectorField::create(double _unit_X, double _unit_Y, int _n_X, int _n_Y, zPoint _minBB, int _NR, bool _setValuesperVertex, bool _triMesh)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField2D<zVector>(_unit_X, _unit_Y, _n_X, _n_Y, _minBB);
		setValuesperVertex = _setValuesperVertex;
		field.valuesperVertex = _setValuesperVertex;
		triMesh = _triMesh;
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createFieldMesh();
	}

	ZSPACE_INLINE void zFnMeshVectorField::createVectorFromScalarField(zObjectMeshScalarField& scalarFieldObj)
	{
		zFnMeshScalarField inFnScalarField(scalarFieldObj);

		zPoint minBB, maxBB;
		inFnScalarField.getBoundingBox(minBB, maxBB);

		int n_X = 0, n_Y = 0;
		inFnScalarField.getResolution(n_X, n_Y);

		const bool valuesPerVertex = inFnScalarField.getValuesPerVertexBoolean();
		const bool isTri = inFnScalarField.getTriMeshBoolean();

		create(minBB, maxBB, n_X, n_Y, 1, valuesPerVertex, isTri);
		zFieldObjectStorage::get(*fieldObj).fieldValues = inFnScalarField.getGradients();
	}

	ZSPACE_INLINE zObjectMesh* zFnMeshVectorField::getRawMesh()
	{
		return fieldObj;
	}

	ZSPACE_INLINE int zFnMeshVectorField::getIndex(int index_X, int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return index_X * field.n_Y + index_Y;
	}

	ZSPACE_INLINE void zFnMeshVectorField::getIndices(int index, int& index_X, int& index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		index_X = (field.n_Y > 0) ? (index / field.n_Y) : 0;
		index_Y = (field.n_Y > 0) ? (index % field.n_Y) : 0;
	}

	ZSPACE_INLINE zPoint zFnMeshVectorField::getPosition(int index) const
	{
		int ix = 0, iy = 0;
		getIndices(index, ix, iy);
		return getPosition(ix, iy);
	}

	ZSPACE_INLINE zPoint zFnMeshVectorField::getPosition(int index_X, int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return gridCellPosition2D(field.minBB, field.unit_X, field.unit_Y, field.valuesperVertex, index_X, index_Y);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildRingNeighbours2D(index, numRings, field.n_X, field.n_Y, outRingNeighbours);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildAdjacentNeighbours2D(index, field.n_X, field.n_Y, outAdjacentNeighbours);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildContainedNeighbours2D(pos, field.minBB, field.unit_X, field.unit_Y, field.n_X, field.n_Y, field.valuesperVertex, containedNeighbour);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour)
	{
		containedNeighbour.clear();
		zIntArray indices;
		getNeighbour_Contained(pos, indices);
		containedNeighbour.reserve(indices.size());
		for (int id : indices)
		{
			containedNeighbour.push_back(getPosition(id));
		}
	}

	ZSPACE_INLINE int zFnMeshVectorField::numFieldValues() const
	{
		return static_cast<int>(zFieldObjectStorage::get(*fieldObj).fieldValues.size());
	}

	ZSPACE_INLINE void zFnMeshVectorField::getResolution(int& _n_X, int& _n_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_n_X = field.n_X;
		_n_Y = field.n_Y;
	}

	ZSPACE_INLINE void zFnMeshVectorField::getUnitDistances(double& _unit_X, double& _unit_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_unit_X = field.unit_X;
		_unit_Y = field.unit_Y;
	}

	ZSPACE_INLINE void zFnMeshVectorField::getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_minBB = field.minBB;
		_maxBB = field.maxBB;
	}

	ZSPACE_INLINE void zFnMeshVectorField::getPositions(zPointArray& positions)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		if (field.valuesperVertex)
		{
			positions = zMeshObjectStorage::read(*fieldObj).positions;
			return;
		}
		const int total = field.n_X * field.n_Y;
		positions.resize(static_cast<std::size_t>(total));
		int idx = 0;
		for (int i = 0; i < field.n_X; ++i)
		{
			for (int j = 0; j < field.n_Y; ++j)
			{
				positions[idx++] = gridCellPosition2D(field.minBB, field.unit_X, field.unit_Y, false, i, j);
			}
		}
	}

	ZSPACE_INLINE zVector zFnMeshVectorField::getFieldValue(int index) const
	{
		return zFieldObjectStorage::get(*fieldObj).fieldValues[index];
	}

	ZSPACE_INLINE bool zFnMeshVectorField::getFieldValue(zPoint& samplePos, zFieldValueType type, zVector& fieldValue)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return sampleField2D(
			field.fieldValues.data(),
			field.fieldValues.size(),
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.n_X,
			field.n_Y,
			field.valuesperVertex,
			samplePos,
			type,
			fieldValue);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getFieldValues(zVectorArray& fieldValues)
	{
		fieldValues = zFieldObjectStorage::get(*fieldObj).fieldValues;
	}

	ZSPACE_INLINE zVector* zFnMeshVectorField::getRawFieldValues()
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (vals.empty()) throw std::invalid_argument(" error: null pointer.");
		return vals.data();
	}

	ZSPACE_INLINE bool zFnMeshVectorField::getValuesPerVertexBoolean() const
	{
		return setValuesperVertex;
	}

	ZSPACE_INLINE bool zFnMeshVectorField::getTriMeshBoolean() const
	{
		return triMesh;
	}

	ZSPACE_INLINE void zFnMeshVectorField::getFieldColorDomain(zDomainColor& colDomain)
	{
		colDomain = fieldColorDomain;
	}

	ZSPACE_INLINE void zFnMeshVectorField::setFieldColorDomain(zDomainColor& colDomain)
	{
		fieldColorDomain = colDomain;
	}

	ZSPACE_INLINE void zFnMeshVectorField::setBoundingBox(zPoint& _minBB, zPoint& _maxBB)
	{
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field.minBB = _minBB;
		field.maxBB = _maxBB;
	}

	ZSPACE_INLINE void zFnMeshVectorField::setFieldValue(int index, const zVector& val, bool append)
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (append) vals[index] += val;
		else vals[index] = val;
	}

	ZSPACE_INLINE void zFnMeshVectorField::setFieldValues(zVectorArray& fValues, zFieldColorType type, float sdfWidth)
	{
		(void)type;
		(void)sdfWidth;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		if (fValues.size() != field.fieldValues.size())
		{
			throw std::invalid_argument("input fValues size not field vectors.");
		}
		field.fieldValues = fValues;
	}

	//---- 2D IDW FIELD METHODS (Vector)

	ZSPACE_INLINE void zFnMeshVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectMesh& inMeshObj, zVector meshValue, double influence, double power, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(const_cast<zPoint&>(inData.positions[j]));
				if (r < tempDist)
				{
					const double w = std::pow(r, power);
					wSum += w;
					const double val = (w > 0.0) ? ((r * influence) / w) : 0.0;
					d = meshValue * static_cast<float>(val);
					tempDist = r;
				}
			}
			fieldValues[i] = (wSum > 0.0) ? (d / static_cast<float>(wSum)) : zVector();
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectGraph& inGraphObj, zVector graphValue, double influence, double power, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(const_cast<zPoint&>(inData.positions[j]));
				if (r < tempDist)
				{
					const double w = std::pow(r, power);
					wSum += w;
					const double val = (w > 0.0) ? ((r * influence) / w) : 0.0;
					d = graphValue * static_cast<float>(val);
					tempDist = r;
				}
			}
			fieldValues[i] = (wSum > 0.0) ? (d / static_cast<float>(wSum)) : zVector();
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectPointCloud& inPointsObj, zVector value, double influence, double power, bool normalise)
	{
		getFieldValuesAsVertexDistance_IDW(fieldValues, inPointsObj.impl->storage.vertexPositions, value, influence, power, normalise);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectPointCloud& inPointsObj, zVectorArray& values, vector<double>& influences, double power, bool normalise)
	{
		getFieldValuesAsVertexDistance_IDW(fieldValues, inPointsObj.impl->storage.vertexPositions, values, influences, power, normalise);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zPointArray& inPositions, zVector value, double influence, double power, bool normalise)
	{
		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(inPositions[j]);
				const double w = std::pow(r, power);
				wSum += w;
				const double val = (w > 0.0) ? ((r * influence) / w) : 0.0;
				d += value * static_cast<float>(val);
			}
			fieldValues[i] = (wSum > 0.0) ? (d / static_cast<float>(wSum)) : zVector();
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE void zFnMeshVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zPointArray& inPositions, zVectorArray& values, vector<double>& influences, double power, bool normalise)
	{
		if (inPositions.size() != values.size()) throw std::invalid_argument(" error: size of inPositions and values dont match.");
		if (inPositions.size() != influences.size()) throw std::invalid_argument(" error: size of inPositions and influences dont match.");

		const auto& gridData = zMeshObjectStorage::read(*fieldObj);
		const std::size_t nGrid = gridData.positions.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			zPoint gp = gridData.positions[i];

			for (std::size_t j = 0; j < nIn; ++j)
			{
				const double r = gp.distanceTo(inPositions[j]);
				const double w = std::pow(r, power);
				wSum += w;
				const double val = (w > 0.0) ? ((r * influences[j]) / w) : 0.0;
				d += values[j] * static_cast<float>(val);
			}
			fieldValues[i] = (wSum > 0.0) ? (d / static_cast<float>(wSum)) : zVector();
		}
		if (normalise) normliseValues(fieldValues);
	}

	ZSPACE_INLINE bool zFnMeshVectorField::checkPositionBounds(zPoint& pos, int& index)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		int ix = -1, iy = -1;
		const bool inBounds = computeCellCoords2D(pos, field.minBB, field.unit_X, field.unit_Y, field.n_X, field.n_Y, ix, iy);
		index = ix * field.n_Y + iy;
		return inBounds;
	}

	ZSPACE_INLINE bool zFnMeshVectorField::checkBounds_X(int index_X) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_X >= 0 && index_X < field.n_X);
	}

	ZSPACE_INLINE bool zFnMeshVectorField::checkBounds_Y(int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_Y >= 0 && index_Y < field.n_Y);
	}

	ZSPACE_INLINE void zFnMeshVectorField::computeDomain(zVectorArray& values, zDomain<zVector>& domain)
	{
		domain.min = coreUtils.zMin(values);
		domain.max = coreUtils.zMax(values);
	}

	ZSPACE_INLINE void zFnMeshVectorField::normliseValues(zVectorArray& fieldValues)
	{
		for (std::size_t i = 0; i < fieldValues.size(); ++i)
		{
			fieldValues[i].normalize();
		}
	}

	ZSPACE_INLINE void zFnMeshVectorField::computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions)
	{
		fieldIndexPositions.clear();
		fieldIndexPositions.resize(static_cast<std::size_t>(numFieldValues()));
		for (std::size_t i = 0; i < positions.size(); ++i)
		{
			int fieldIndex = -1;
			if (checkPositionBounds(positions[i], fieldIndex) && fieldIndex >= 0 && fieldIndex < numFieldValues())
			{
				fieldIndexPositions[fieldIndex].push_back(positions[i]);
			}
		}
	}

	ZSPACE_INLINE void zFnMeshVectorField::computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies)
	{
		fieldIndexPositionIndicies.clear();
		fieldIndexPositionIndicies.resize(static_cast<std::size_t>(numFieldValues()));
		for (std::size_t i = 0; i < positions.size(); ++i)
		{
			int fieldIndex = -1;
			if (checkPositionBounds(positions[i], fieldIndex) && fieldIndex >= 0 && fieldIndex < numFieldValues())
			{
				fieldIndexPositionIndicies[fieldIndex].push_back(static_cast<int>(i));
			}
		}
	}

	ZSPACE_INLINE double zFnMeshVectorField::F_of_r(double& r, double& a, double& b)
	{
		if (0 <= r && r <= b / 3.0) return (a * (1.0 - (3.0 * r * r) / (b * b)));
		if (b / 3.0 <= r && r <= b) return (3 * a / 2 * std::pow(1.0 - (r / b), 2.0));
		return 0.0;
	}

	ZSPACE_INLINE void zFnMeshVectorField::createFieldMesh()
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		zFieldObjectStorage::populateGridMesh2D(
			*fieldObj,
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.n_X,
			field.n_Y,
			setValuesperVertex,
			triMesh);
	}
}
