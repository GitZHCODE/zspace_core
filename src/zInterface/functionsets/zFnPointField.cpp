// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//

#include <zspace/zInterface/functionsets/zFnPointField.h>
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
		struct EdgeSegment3D
		{
			zPoint p0;
			zVector ab;
			double ab2;
		};

		inline zPoint gridCellPosition3D(const zVector& minBB, double unit_X, double unit_Y, double unit_Z, int i, int j, int k) noexcept
		{
			return zPoint(
				minBB.x + static_cast<float>(i * unit_X),
				minBB.y + static_cast<float>(j * unit_Y),
				minBB.z + static_cast<float>(k * unit_Z));
		}

		inline bool computeCellCoords3D(
			const zPoint& pos,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			double unit_Z,
			int n_X,
			int n_Y,
			int n_Z,
			int& index_X,
			int& index_Y,
			int& index_Z) noexcept
		{
			if (n_X <= 0 || n_Y <= 0 || n_Z <= 0 || unit_X == 0.0 || unit_Y == 0.0 || unit_Z == 0.0)
			{
				index_X = -1;
				index_Y = -1;
				index_Z = -1;
				return false;
			}
			index_X = static_cast<int>(std::floor((pos.x - minBB.x) / unit_X));
			index_Y = static_cast<int>(std::floor((pos.y - minBB.y) / unit_Y));
			index_Z = static_cast<int>(std::floor((pos.z - minBB.z) / unit_Z));
			return (index_X >= 0 && index_X < n_X &&
				index_Y >= 0 && index_Y < n_Y &&
				index_Z >= 0 && index_Z < n_Z);
		}

		template<typename T, typename ValueT>
		bool sampleField3D(
			const ValueT* values,
			std::size_t valueCount,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			double unit_Z,
			int n_X,
			int n_Y,
			int n_Z,
			const zPoint& samplePos,
			zFieldValueType type,
			T& fieldValue)
		{
			int ix = -1, iy = -1, iz = -1;
			if (!computeCellCoords3D(samplePos, minBB, unit_X, unit_Y, unit_Z, n_X, n_Y, n_Z, ix, iy, iz))
			{
				return false;
			}
			const int strideX = n_Y * n_Z;
			const int centerId = ix * strideX + iy * n_Z + iz;
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
				const int kMin = std::max(0, iz - 1);
				const int kMax = std::min(n_Z - 1, iz + 1);

				double wSum = 0.0;
				T weightedSum = T();
				for (int i = iMin; i <= iMax; ++i)
				{
					for (int j = jMin; j <= jMax; ++j)
					{
						for (int k = kMin; k <= kMax; ++k)
						{
							const zPoint nPos = gridCellPosition3D(minBB, unit_X, unit_Y, unit_Z, i, j, k);
							const double dx = static_cast<double>(samplePos.x) - nPos.x;
							const double dy = static_cast<double>(samplePos.y) - nPos.y;
							const double dz = static_cast<double>(samplePos.z) - nPos.z;
							const double distSq = dx * dx + dy * dy + dz * dz;
							if (distSq <= 0.0)
							{
								fieldValue = values[i * strideX + j * n_Z + k];
								return true;
							}
							const double w = 1.0 / distSq;
							ValueT v = values[i * strideX + j * n_Z + k];
							weightedSum += v * static_cast<float>(w);
							wSum += w;
						}
					}
				}
				if (wSum > 0.0) weightedSum /= static_cast<float>(wSum);
				fieldValue = weightedSum;
				return true;
			}

			if (type == zFieldAdjacentWeighted)
			{
				const int iMin = std::max(0, ix - 1);
				const int iMax = std::min(n_X - 1, ix + 1);
				const int jMin = std::max(0, iy - 1);
				const int jMax = std::min(n_Y - 1, iy + 1);
				const int kMin = std::max(0, iz - 1);
				const int kMax = std::min(n_Z - 1, iz + 1);

				double wSum = 0.0;
				T weightedSum = T();
				for (int i = iMin; i <= iMax; ++i)
				{
					for (int j = jMin; j <= jMax; ++j)
					{
						for (int k = kMin; k <= kMax; ++k)
						{
							if (i != ix && j != iy && k != iz) continue;
							const zPoint nPos = gridCellPosition3D(minBB, unit_X, unit_Y, unit_Z, i, j, k);
							const double dx = static_cast<double>(samplePos.x) - nPos.x;
							const double dy = static_cast<double>(samplePos.y) - nPos.y;
							const double dz = static_cast<double>(samplePos.z) - nPos.z;
							const double distSq = dx * dx + dy * dy + dz * dz;
							if (distSq <= 0.0)
							{
								fieldValue = values[i * strideX + j * n_Z + k];
								return true;
							}
							const double w = 1.0 / distSq;
							ValueT v = values[i * strideX + j * n_Z + k];
							weightedSum += v * static_cast<float>(w);
							wSum += w;
						}
					}
				}
				if (wSum > 0.0) weightedSum /= static_cast<float>(wSum);
				fieldValue = weightedSum;
				return true;
			}

			if (type == zFieldContainedWeighted)
			{
				const zPoint cPos = gridCellPosition3D(minBB, unit_X, unit_Y, unit_Z, ix, iy, iz);
				const int stepX = (samplePos.x - cPos.x < 0.0f) ? -1 : 1;
				const int stepY = (samplePos.y - cPos.y < 0.0f) ? -1 : 1;
				const int stepZ = (samplePos.z - cPos.z < 0.0f) ? -1 : 1;

				const int xs[2] = { ix, ix + stepX };
				const int ys[2] = { iy, iy + stepY };
				const int zs[2] = { iz, iz + stepZ };

				double wSum = 0.0;
				T weightedSum = T();
				for (int a = 0; a < 2; ++a)
				{
					const int i = xs[a];
					if (i < 0 || i >= n_X) continue;
					for (int b = 0; b < 2; ++b)
					{
						const int j = ys[b];
						if (j < 0 || j >= n_Y) continue;
						for (int c = 0; c < 2; ++c)
						{
							const int k = zs[c];
							if (k < 0 || k >= n_Z) continue;
							const zPoint nPos = gridCellPosition3D(minBB, unit_X, unit_Y, unit_Z, i, j, k);
							const double dx = static_cast<double>(samplePos.x) - nPos.x;
							const double dy = static_cast<double>(samplePos.y) - nPos.y;
							const double dz = static_cast<double>(samplePos.z) - nPos.z;
							const double distSq = dx * dx + dy * dy + dz * dz;
							if (distSq <= 0.0)
							{
								fieldValue = values[i * strideX + j * n_Z + k];
								return true;
							}
							const double w = 1.0 / distSq;
							ValueT v = values[i * strideX + j * n_Z + k];
							weightedSum += v * static_cast<float>(w);
							wSum += w;
						}
					}
				}
				if (wSum > 0.0) weightedSum /= static_cast<float>(wSum);
				fieldValue = weightedSum;
				return true;
			}

			return false;
		}

		void buildContainedNeighbours3D(
			const zPoint& pos,
			const zVector& minBB,
			double unit_X,
			double unit_Y,
			double unit_Z,
			int n_X,
			int n_Y,
			int n_Z,
			zIntArray& containedNeighbour)
		{
			containedNeighbour.clear();
			int ix = -1, iy = -1, iz = -1;
			if (!computeCellCoords3D(pos, minBB, unit_X, unit_Y, unit_Z, n_X, n_Y, n_Z, ix, iy, iz)) return;

			const zPoint cPos = gridCellPosition3D(minBB, unit_X, unit_Y, unit_Z, ix, iy, iz);
			const int stepX = (pos.x - cPos.x < 0.0f) ? -1 : 1;
			const int stepY = (pos.y - cPos.y < 0.0f) ? -1 : 1;
			const int stepZ = (pos.z - cPos.z < 0.0f) ? -1 : 1;

			const int xs[2] = { ix, ix + stepX };
			const int ys[2] = { iy, iy + stepY };
			const int zs[2] = { iz, iz + stepZ };
			const int strideX = n_Y * n_Z;

			for (int a = 0; a < 2; ++a)
			{
				const int i = xs[a];
				if (i < 0 || i >= n_X) continue;
				for (int b = 0; b < 2; ++b)
				{
					const int j = ys[b];
					if (j < 0 || j >= n_Y) continue;
					for (int c = 0; c < 2; ++c)
					{
						const int k = zs[c];
						if (k < 0 || k >= n_Z) continue;
						containedNeighbour.push_back(i * strideX + j * n_Z + k);
					}
				}
			}
		}

		void buildRingNeighbours3D(int index, int numRings, int n_X, int n_Y, int n_Z, zIntArray& out)
		{
			out.clear();
			const int total = n_X * n_Y * n_Z;
			if (n_X <= 0 || n_Y <= 0 || n_Z <= 0 || index < 0 || index >= total) return;
			const int strideX = n_Y * n_Z;
			const int ix = index / strideX;
			const int rem = index % strideX;
			const int iy = rem / n_Z;
			const int iz = rem % n_Z;

			const int iMin = std::max(0, ix - numRings);
			const int iMax = std::min(n_X - 1, ix + numRings);
			const int jMin = std::max(0, iy - numRings);
			const int jMax = std::min(n_Y - 1, iy + numRings);
			const int kMin = std::max(0, iz - numRings);
			const int kMax = std::min(n_Z - 1, iz + numRings);

			for (int i = iMin; i <= iMax; ++i)
			{
				for (int j = jMin; j <= jMax; ++j)
				{
					for (int k = kMin; k <= kMax; ++k)
					{
						out.push_back(i * strideX + j * n_Z + k);
					}
				}
			}
		}

		void buildAdjacentNeighbours3D(int index, int n_X, int n_Y, int n_Z, zIntArray& out)
		{
			out.clear();
			const int total = n_X * n_Y * n_Z;
			if (n_X <= 0 || n_Y <= 0 || n_Z <= 0 || index < 0 || index >= total) return;
			const int strideX = n_Y * n_Z;
			const int ix = index / strideX;
			const int rem = index % strideX;
			const int iy = rem / n_Z;
			const int iz = rem % n_Z;

			for (int di = -1; di <= 1; ++di)
			{
				for (int dj = -1; dj <= 1; ++dj)
				{
					for (int dk = -1; dk <= 1; ++dk)
					{
						if (di != 0 && dj != 0 && dk != 0) continue;
						const int i = ix + di;
						const int j = iy + dj;
						const int k = iz + dk;
						if (i >= 0 && i < n_X && j >= 0 && j < n_Y && k >= 0 && k < n_Z)
						{
							out.push_back(i * strideX + j * n_Z + k);
						}
					}
				}
			}
		}
	}

	//==========================================================================
	// zFnPointScalarField
	//==========================================================================

	ZSPACE_INLINE zFnPointScalarField::zFnPointScalarField()
		: fnType(zFnType::zPointFieldFn), fieldObj(nullptr)
	{
	}

	ZSPACE_INLINE zFnPointScalarField::zFnPointScalarField(zObjectPointScalarField& _fieldObj)
		: fnType(zFnType::zPointFieldFn), fieldObj(&_fieldObj), fnPoints(_fieldObj)
	{
	}

	ZSPACE_INLINE zFnPointScalarField::~zFnPointScalarField() = default;

	ZSPACE_INLINE zFnType zFnPointScalarField::getType()
	{
		return zFnType::zPointFieldFn;
	}

	ZSPACE_INLINE void zFnPointScalarField::getBounds(zPoint& minBB, zPoint& maxBB)
	{
		getBoundingBox(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnPointScalarField::clear()
	{
		fnPoints.clear();
		if (fieldObj)
		{
			zFieldObjectStorage::get(*fieldObj).fieldValues.clear();
		}
		ringNeighbours.clear();
		adjacentNeighbours.clear();
	}

	ZSPACE_INLINE void zFnPointScalarField::create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _n_Z, int _NR)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField3D<zScalar>(_minBB, _maxBB, _n_X, _n_Y, _n_Z);
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createPointCloud();
	}

	ZSPACE_INLINE void zFnPointScalarField::create(double _unit_X, double _unit_Y, double _unit_Z, int _n_X, int _n_Y, int _n_Z, zPoint _minBB, int _NR)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField3D<zScalar>(_unit_X, _unit_Y, _unit_Z, _n_X, _n_Y, _n_Z, _minBB);
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createPointCloud();
	}

	ZSPACE_INLINE int zFnPointScalarField::getIndex(int index_X, int index_Y, int index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return index_X * field.n_Y * field.n_Z + index_Y * field.n_Z + index_Z;
	}

	ZSPACE_INLINE void zFnPointScalarField::getIndices(int index, int& index_X, int& index_Y, int& index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		const int strideX = field.n_Y * field.n_Z;
		if (strideX <= 0 || field.n_Z <= 0)
		{
			index_X = 0; index_Y = 0; index_Z = 0;
			return;
		}
		index_X = index / strideX;
		const int rem = index % strideX;
		index_Y = rem / field.n_Z;
		index_Z = rem % field.n_Z;
	}

	ZSPACE_INLINE zPoint zFnPointScalarField::getPosition(int index) const
	{
		int ix = 0, iy = 0, iz = 0;
		getIndices(index, ix, iy, iz);
		return getPosition(ix, iy, iz);
	}

	ZSPACE_INLINE zPoint zFnPointScalarField::getPosition(int index_X, int index_Y, int index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return gridCellPosition3D(field.minBB, field.unit_X, field.unit_Y, field.unit_Z, index_X, index_Y, index_Z);
	}

	ZSPACE_INLINE void zFnPointScalarField::getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildRingNeighbours3D(index, numRings, field.n_X, field.n_Y, field.n_Z, outRingNeighbours);
	}

	ZSPACE_INLINE void zFnPointScalarField::getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildAdjacentNeighbours3D(index, field.n_X, field.n_Y, field.n_Z, outAdjacentNeighbours);
	}

	ZSPACE_INLINE void zFnPointScalarField::getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildContainedNeighbours3D(pos, field.minBB, field.unit_X, field.unit_Y, field.unit_Z, field.n_X, field.n_Y, field.n_Z, containedNeighbour);
	}

	ZSPACE_INLINE void zFnPointScalarField::getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour)
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

	ZSPACE_INLINE int zFnPointScalarField::numFieldValues() const
	{
		return static_cast<int>(zFieldObjectStorage::get(*fieldObj).fieldValues.size());
	}

	ZSPACE_INLINE void zFnPointScalarField::getResolution(int& _n_X, int& _n_Y, int& _n_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_n_X = field.n_X;
		_n_Y = field.n_Y;
		_n_Z = field.n_Z;
	}

	ZSPACE_INLINE void zFnPointScalarField::getUnitDistances(double& _unit_X, double& _unit_Y, double& _unit_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_unit_X = field.unit_X;
		_unit_Y = field.unit_Y;
		_unit_Z = field.unit_Z;
	}

	ZSPACE_INLINE void zFnPointScalarField::getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_minBB = field.minBB;
		_maxBB = field.maxBB;
	}

	ZSPACE_INLINE void zFnPointScalarField::getPositions(zPointArray& positions)
	{
		fnPoints.getVertexPositions(positions);
	}

	ZSPACE_INLINE zScalar zFnPointScalarField::getFieldValue(int index) const
	{
		return zFieldObjectStorage::get(*fieldObj).fieldValues[index];
	}

	ZSPACE_INLINE bool zFnPointScalarField::getFieldValue(zPoint& samplePos, zFieldValueType type, zScalar& fieldValue)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return sampleField3D(
			field.fieldValues.data(),
			field.fieldValues.size(),
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.unit_Z,
			field.n_X,
			field.n_Y,
			field.n_Z,
			samplePos,
			type,
			fieldValue);
	}

	ZSPACE_INLINE void zFnPointScalarField::getFieldValues(zScalarArray& fieldValues)
	{
		fieldValues = zFieldObjectStorage::get(*fieldObj).fieldValues;
	}

	ZSPACE_INLINE zScalar* zFnPointScalarField::getRawFieldValues()
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (vals.empty()) throw std::invalid_argument(" error: null pointer.");
		return vals.data();
	}

	ZSPACE_INLINE zVector zFnPointScalarField::getGradient(int index, float epsilon)
	{
		(void)epsilon;
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		const int nX = field.n_X;
		const int nY = field.n_Y;
		const int nZ = field.n_Z;
		const int strideX = nY * nZ;
		if (strideX <= 0 || nZ <= 0) return zVector();
		const int id_X = index / strideX;
		const int rem = index % strideX;
		const int id_Y = rem / nZ;
		const int id_Z = rem % nZ;
		if (id_X <= 0 || id_Y <= 0 || id_Z <= 0 || id_X >= nX - 1 || id_Y >= nY - 1 || id_Z >= nZ - 1)
		{
			return zVector();
		}
		const float invX = (field.unit_X != 0.0) ? static_cast<float>(1.0 / (3.0 * field.unit_X)) : 0.0f;
		const float invY = (field.unit_Y != 0.0) ? static_cast<float>(1.0 / (3.0 * field.unit_Y)) : 0.0f;
		const float invZ = (field.unit_Z != 0.0) ? static_cast<float>(1.0 / (3.0 * field.unit_Z)) : 0.0f;
		const zScalar fieldVal = field.fieldValues[index];
		const zScalar fieldVal1 = field.fieldValues[index + strideX];
		const zScalar fieldVal2 = field.fieldValues[index + nZ];
		const zScalar fieldVal3 = field.fieldValues[index + 1];
		return zVector((fieldVal1 - fieldVal) * invX, (fieldVal2 - fieldVal) * invY, (fieldVal3 - fieldVal) * invZ);
	}

	ZSPACE_INLINE vector<zVector> zFnPointScalarField::getGradients(float epsilon)
	{
		(void)epsilon;
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		const int nX = field.n_X;
		const int nY = field.n_Y;
		const int nZ = field.n_Z;
		const int strideX = nY * nZ;
		const int total = static_cast<int>(field.fieldValues.size());
		vector<zVector> out(static_cast<std::size_t>(total), zVector());
		if (nX <= 2 || nY <= 2 || nZ <= 2 || total != nX * nY * nZ) return out;

		const float invX = (field.unit_X != 0.0) ? static_cast<float>(1.0 / (3.0 * field.unit_X)) : 0.0f;
		const float invY = (field.unit_Y != 0.0) ? static_cast<float>(1.0 / (3.0 * field.unit_Y)) : 0.0f;
		const float invZ = (field.unit_Z != 0.0) ? static_cast<float>(1.0 / (3.0 * field.unit_Z)) : 0.0f;
		const zScalar* vals = field.fieldValues.data();
		zVector* dst = out.data();

		for (int i = 1; i < nX - 1; ++i)
		{
			const int baseI = i * strideX;
			for (int j = 1; j < nY - 1; ++j)
			{
				const int baseIJ = baseI + j * nZ;
				for (int k = 1; k < nZ - 1; ++k)
				{
					const int idx = baseIJ + k;
					const zScalar v = vals[idx];
					dst[idx] = zVector(
						(vals[idx + strideX] - v) * invX,
						(vals[idx + nZ] - v) * invY,
						(vals[idx + 1] - v) * invZ);
				}
			}
		}
		return out;
	}

	ZSPACE_INLINE void zFnPointScalarField::setFieldColorDomain(zDomainColor& colDomain)
	{
		fieldColorDomain = colDomain;
	}

	ZSPACE_INLINE void zFnPointScalarField::setBoundingBox(zPoint& _minBB, zPoint& _maxBB)
	{
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field.minBB = _minBB;
		field.maxBB = _maxBB;
	}

	ZSPACE_INLINE void zFnPointScalarField::setFieldValue(int index, zScalar val, bool append)
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (append) vals[index] += val;
		else vals[index] = val;
	}

	ZSPACE_INLINE void zFnPointScalarField::setFieldValues(zScalarArray& fValues)
	{
		auto& field = zFieldObjectStorage::get(*fieldObj);
		if (fValues.size() != field.fieldValues.size())
		{
			throw std::invalid_argument("input fValues size not field scalars.");
		}
		field.fieldValues = fValues;
	}

	//---- 3D IDW FIELD METHODS (Scalar)

	ZSPACE_INLINE void zFnPointScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectMesh& inMeshObj, zScalar meshValue, double influence, double power, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zScalar d = 0.0f;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridPts[i];

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

	ZSPACE_INLINE void zFnPointScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectGraph& inGraphObj, zScalar graphValue, double influence, double power, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zScalar d = 0.0f;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridPts[i];

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

	ZSPACE_INLINE void zFnPointScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalar value, double influence, double power, bool normalise)
	{
		getFieldValuesAsVertexDistance_IDW(fieldValues, inPointsObj.impl->storage.vertexPositions, value, influence, power, normalise);
	}

	ZSPACE_INLINE void zFnPointScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalarArray& values, vector<double>& influences, double power, bool normalise)
	{
		getFieldValuesAsVertexDistance_IDW(fieldValues, inPointsObj.impl->storage.vertexPositions, values, influences, power, normalise);
	}

	ZSPACE_INLINE void zFnPointScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalar value, double influence, double power, bool normalise)
	{
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			double d = 0.0;
			double wSum = 0.0;
			zPoint gp = gridPts[i];

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

	ZSPACE_INLINE void zFnPointScalarField::getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalarArray& values, vector<double>& influences, double power, bool normalise)
	{
		if (inPositions.size() != values.size()) throw std::invalid_argument(" error: size of inPositions and values dont match.");
		if (inPositions.size() != influences.size()) throw std::invalid_argument(" error: size of inPositions and influences dont match.");

		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			double d = 0.0;
			double wSum = 0.0;
			zPoint gp = gridPts[i];

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

	//---- 3D SCALAR DISTANCE METHODS

	ZSPACE_INLINE void zFnPointScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zObjectPointCloud& inPointsObj, bool normalise)
	{
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const auto& srcPts = inPointsObj.impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = srcPts.size();
		scalars.resize(nGrid);
		if (nIn == 0)
		{
			std::fill(scalars.begin(), scalars.end(), 0.0f);
			return;
		}

		std::vector<double> distVals(nGrid, 10000.0);
		double dMin = 100000.0;
		double dMax = 0.0;

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			const float gx = gridPts[i].x;
			const float gy = gridPts[i].y;
			const float gz = gridPts[i].z;
			double minSq = 10000.0;
			for (std::size_t j = 0; j < nIn; ++j)
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

	ZSPACE_INLINE void zFnPointScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, double offset, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		const auto& gridPtsVec = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPtsVec.size();
		const std::size_t nIn = inData.positions.size();
		scalars.resize(nGrid);
		if (nIn == 0)
		{
			std::fill(scalars.begin(), scalars.end(), 0.0f);
			return;
		}

		const zPoint* gridPts = gridPtsVec.data();
		const zPoint* srcPts = inData.positions.data();
		const float offsetF = static_cast<float>(offset);

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
					scalars[i] = std::sqrt(minDistSq) - offsetF;
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
					scalars[i] = std::sqrt(minDistSq) - offsetF;
				}
			});
		}

		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnPointScalarField::getScalarsAsVertexDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, double offset, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		const auto& gridPtsVec = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPtsVec.size();
		const std::size_t nIn = inData.positions.size();
		scalars.resize(nGrid);
		if (nIn == 0)
		{
			std::fill(scalars.begin(), scalars.end(), 0.0f);
			return;
		}

		const zPoint* gridPts = gridPtsVec.data();
		const zPoint* srcPts = inData.positions.data();
		const float offsetF = static_cast<float>(offset);

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
					scalars[i] = std::sqrt(minDistSq) - offsetF;
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
					scalars[i] = std::sqrt(minDistSq) - offsetF;
				}
			});
		}

		if (normalise) normliseValues(scalars);
	}

	ZSPACE_INLINE void zFnPointScalarField::getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, double offset, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		const auto& gridPtsVec = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPtsVec.size();
		const int nEdges = inData.numEdges();
		scalars.resize(nGrid);

		std::vector<EdgeSegment3D> segments;
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

		const zPoint* gridPts = gridPtsVec.data();
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

	ZSPACE_INLINE void zFnPointScalarField::getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, double offset, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		const auto& gridPtsVec = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPtsVec.size();
		const int nEdges = inData.numEdges();
		scalars.resize(nGrid);

		std::vector<EdgeSegment3D> segments;
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

		const zPoint* gridPts = gridPtsVec.data();
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

	//---- COMPUTE & SMOOTH METHODS

	ZSPACE_INLINE bool zFnPointScalarField::checkPositionBounds(zPoint& pos, int& index) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		int ix = -1, iy = -1, iz = -1;
		const bool inBounds = computeCellCoords3D(pos, field.minBB, field.unit_X, field.unit_Y, field.unit_Z, field.n_X, field.n_Y, field.n_Z, ix, iy, iz);
		index = ix * field.n_Y * field.n_Z + iy * field.n_Z + iz;
		return inBounds;
	}

	ZSPACE_INLINE bool zFnPointScalarField::checkBounds_X(int index_X) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_X >= 0 && index_X < field.n_X);
	}

	ZSPACE_INLINE bool zFnPointScalarField::checkBounds_Y(int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_Y >= 0 && index_Y < field.n_Y);
	}

	ZSPACE_INLINE bool zFnPointScalarField::checkBounds_Z(int index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_Z >= 0 && index_Z < field.n_Z);
	}

	ZSPACE_INLINE void zFnPointScalarField::computeMinMaxOfScalars(zScalarArray& values, zScalar& dMin, zScalar& dMax)
	{
		if (values.empty())
		{
			dMin = 0.0f;
			dMax = 0.0f;
			return;
		}
		auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());
		dMin = *minIt;
		dMax = *maxIt;
	}

	ZSPACE_INLINE void zFnPointScalarField::computeDomain(zScalarArray& values, zDomain<zScalar>& domain)
	{
		computeMinMaxOfScalars(values, domain.min, domain.max);
	}

	ZSPACE_INLINE void zFnPointScalarField::normliseValues(zScalarArray& fieldValues)
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

	ZSPACE_INLINE void zFnPointScalarField::smoothField(int numSmooth, double diffuseDamp, zDiffusionType type)
	{
		if (numSmooth <= 0) return;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		auto& scalars = field.fieldValues;
		const int nX = field.n_X;
		const int nY = field.n_Y;
		const int nZ = field.n_Z;
		const int strideX = nY * nZ;
		if (static_cast<int>(scalars.size()) != nX * nY * nZ || scalars.empty()) return;

		zScalarArray tempValues(scalars.size());
		const float damp = static_cast<float>(diffuseDamp);

		for (int step = 0; step < numSmooth; ++step)
		{
			const zScalar* src = scalars.data();
			zScalar* dst = tempValues.data();

			if (type == zAverage)
			{
				detail::zFieldParallelFor(nX, 8, [&](int iBegin, int iEnd)
				{
					for (int i = iBegin; i < iEnd; ++i)
					{
						const int iMin = std::max(0, i - 1);
						const int iMax = std::min(nX - 1, i + 1);
						for (int j = 0; j < nY; ++j)
						{
							const int jMin = std::max(0, j - 1);
							const int jMax = std::min(nY - 1, j + 1);
							for (int k = 0; k < nZ; ++k)
							{
								const int kMin = std::max(0, k - 1);
								const int kMax = std::min(nZ - 1, k + 1);
								float sum = 0.0f;
								int count = 0;
								for (int ii = iMin; ii <= iMax; ++ii)
								{
									const int baseI = ii * strideX;
									for (int jj = jMin; jj <= jMax; ++jj)
									{
										const zScalar* row = src + baseI + jj * nZ;
										for (int kk = kMin; kk <= kMax; ++kk)
										{
											sum += row[kk];
											++count;
										}
									}
								}
								dst[i * strideX + j * nZ + k] = (sum != 0.0f && count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
							}
						}
					}
				});
			}
			else if (type == zLaplacian)
			{
				detail::zFieldParallelFor(nX, 8, [&](int iBegin, int iEnd)
				{
					for (int i = iBegin; i < iEnd; ++i)
					{
						const int iMin = std::max(0, i - 1);
						const int iMax = std::min(nX - 1, i + 1);
						for (int j = 0; j < nY; ++j)
						{
							const int jMin = std::max(0, j - 1);
							const int jMax = std::min(nY - 1, j + 1);
							for (int k = 0; k < nZ; ++k)
							{
								const int kMin = std::max(0, k - 1);
								const int kMax = std::min(nZ - 1, k + 1);
								const int centerId = i * strideX + j * nZ + k;
								float lapA = 0.0f;
								for (int ii = iMin; ii <= iMax; ++ii)
								{
									const int baseI = ii * strideX;
									for (int jj = jMin; jj <= jMax; ++jj)
									{
										const int baseIJ = baseI + jj * nZ;
										for (int kk = kMin; kk <= kMax; ++kk)
										{
											const int id = baseIJ + kk;
											if (id != centerId) lapA += src[id];
											else lapA += src[id] * -26.0f;
										}
									}
								}
								dst[centerId] = src[centerId] + lapA * damp;
							}
						}
					}
				});
			}

			scalars.swap(tempValues);
		}
		updateColors();
	}

	ZSPACE_INLINE void zFnPointScalarField::computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions)
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

	ZSPACE_INLINE void zFnPointScalarField::computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies)
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

	ZSPACE_INLINE double zFnPointScalarField::F_of_r(double& r, double& a, double& b)
	{
		if (0 <= r && r <= b / 3.0) return (a * (1.0 - (3.0 * r * r) / (b * b)));
		if (b / 3.0 <= r && r <= b) return (3 * a / 2 * std::pow(1.0 - (r / b), 2.0));
		return 0.0;
	}

	//---- BOOLEAN & UPDATE METHODS

	ZSPACE_INLINE void zFnPointScalarField::boolean_union(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		const std::size_t n = fieldValues_A.size();
		fieldValues_Result.resize(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			fieldValues_Result[i] = std::min(fieldValues_A[i], fieldValues_B[i]);
		}
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnPointScalarField::boolean_subtract(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		const std::size_t n = fieldValues_A.size();
		fieldValues_Result.resize(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			fieldValues_Result[i] = std::max(fieldValues_A[i], -fieldValues_B[i]);
		}
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnPointScalarField::boolean_intersect(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		const std::size_t n = fieldValues_A.size();
		fieldValues_Result.resize(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			fieldValues_Result[i] = std::max(fieldValues_A[i], fieldValues_B[i]);
		}
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnPointScalarField::boolean_difference(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise)
	{
		zScalarArray AUnionB;
		boolean_union(fieldValues_A, fieldValues_B, AUnionB, normalise);

		zScalarArray AIntersectB;
		boolean_intersect(fieldValues_B, fieldValues_A, AIntersectB, normalise);

		boolean_subtract(AUnionB, AIntersectB, fieldValues_Result, normalise);
		if (normalise) normliseValues(fieldValues_Result);
	}

	ZSPACE_INLINE void zFnPointScalarField::boolean_clipwithPlane(zScalarArray& scalars, zMatrix4& clipPlane)
	{
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		zVector O(clipPlane(3, 0), clipPlane(3, 1), clipPlane(3, 2));
		zVector Z(clipPlane(2, 0), clipPlane(2, 1), clipPlane(2, 2));
		const double zLen = Z.length();
		if (zLen <= 1.0e-12) return;

		const std::size_t n = std::min(scalars.size(), gridPts.size());
		for (std::size_t i = 0; i < n; ++i)
		{
			zPoint pos = gridPts[i];
			zVector A = pos - O;
			const double minDist_Plane = (A * Z) / zLen;
			if (minDist_Plane > 0.0)
			{
				scalars[i] = 1.0f;
			}
		}
	}

	ZSPACE_INLINE void zFnPointScalarField::updateColors()
	{
		auto& scalars = zFieldObjectStorage::get(*fieldObj).fieldValues;
		auto& pc = fieldObj->impl->storage;
		if (scalars.size() != pc.vertexColors.size() || scalars.empty()) return;

		computeDomain(scalars, contourValueDomain);
		if (contourValueDomain.min == contourValueDomain.max)
		{
			std::fill(pc.vertexColors.begin(), pc.vertexColors.end(), zColor(0.5f, 0.5f, 0.5f));
			return;
		}

		fieldColorDomain.min.toHSV();
		fieldColorDomain.max.toHSV();
		const float domSpan = contourValueDomain.max - contourValueDomain.min;
		const float invDomSpan = (domSpan != 0.0f) ? (1.0f / domSpan) : 0.0f;

		const std::size_t n = scalars.size();
		zColor* cols = pc.vertexColors.data();
		const zScalar* vals = scalars.data();
		const float cMin = contourValueDomain.min;
		const float cMax = contourValueDomain.max;
		const zColor colMin = fieldColorDomain.min;
		const zColor colMax = fieldColorDomain.max;

		detail::zFieldParallelFor(static_cast<int>(n), 4096, [&](int begin, int end)
		{
			for (int i = begin; i < end; ++i)
			{
				const float v = vals[i];
				if (v < cMin) cols[i] = colMin;
				else if (v > cMax) cols[i] = colMax;
				else cols[i] = detail::blendHSVPrecomputed(v, cMin, invDomSpan, colMin, colMax);
			}
		});
	}

	ZSPACE_INLINE void zFnPointScalarField::createPointCloud()
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		zFieldObjectStorage::populateGridPointCloud3D(
			*fieldObj,
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.unit_Z,
			field.n_X,
			field.n_Y,
			field.n_Z);
	}

	//==========================================================================
	// zFnPointVectorField
	//==========================================================================

	ZSPACE_INLINE zFnPointVectorField::zFnPointVectorField()
		: fnType(zFnType::zPointFieldFn), fieldObj(nullptr)
	{
	}

	ZSPACE_INLINE zFnPointVectorField::zFnPointVectorField(zObjectPointVectorField& _fieldObj)
		: fnType(zFnType::zPointFieldFn), fieldObj(&_fieldObj), fnPoints(_fieldObj)
	{
	}

	ZSPACE_INLINE zFnPointVectorField::~zFnPointVectorField() = default;

	ZSPACE_INLINE zFnType zFnPointVectorField::getType()
	{
		return zFnType::zPointFieldFn;
	}

	ZSPACE_INLINE void zFnPointVectorField::getBounds(zPoint& minBB, zPoint& maxBB)
	{
		getBoundingBox(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnPointVectorField::clear()
	{
		fnPoints.clear();
		if (fieldObj)
		{
			zFieldObjectStorage::get(*fieldObj).fieldValues.clear();
		}
		ringNeighbours.clear();
		adjacentNeighbours.clear();
	}

	ZSPACE_INLINE void zFnPointVectorField::create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _n_Z, int _NR)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField3D<zVector>(_minBB, _maxBB, _n_X, _n_Y, _n_Z);
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createPointCloud();
	}

	ZSPACE_INLINE void zFnPointVectorField::create(double _unit_X, double _unit_Y, double _unit_Z, int _n_X, int _n_Y, int _n_Z, zPoint _minBB, int _NR)
	{
		(void)_NR;
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field = zField3D<zVector>(_unit_X, _unit_Y, _unit_Z, _n_X, _n_Y, _n_Z, _minBB);
		ringNeighbours.clear();
		adjacentNeighbours.clear();
		createPointCloud();
	}

	ZSPACE_INLINE void zFnPointVectorField::createVectorFromScalarField(zFnPointScalarField& inFnScalarField)
	{
		zPoint minBB, maxBB;
		inFnScalarField.getBoundingBox(minBB, maxBB);

		int n_X = 0, n_Y = 0, n_Z = 0;
		inFnScalarField.getResolution(n_X, n_Y, n_Z);

		create(minBB, maxBB, n_X, n_Y, n_Z, 1);
		zFieldObjectStorage::get(*fieldObj).fieldValues = inFnScalarField.getGradients();
	}

	ZSPACE_INLINE void zFnPointVectorField::createVectorFromScalarField(zObjectPointScalarField& scalarFieldObj)
	{
		zFnPointScalarField inFn(scalarFieldObj);
		createVectorFromScalarField(inFn);
	}

	ZSPACE_INLINE int zFnPointVectorField::getIndex(int index_X, int index_Y, int index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return index_X * field.n_Y * field.n_Z + index_Y * field.n_Z + index_Z;
	}

	ZSPACE_INLINE void zFnPointVectorField::getIndices(int index, int& index_X, int& index_Y, int& index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		const int strideX = field.n_Y * field.n_Z;
		if (strideX <= 0 || field.n_Z <= 0)
		{
			index_X = 0; index_Y = 0; index_Z = 0;
			return;
		}
		index_X = index / strideX;
		const int rem = index % strideX;
		index_Y = rem / field.n_Z;
		index_Z = rem % field.n_Z;
	}

	ZSPACE_INLINE zPoint zFnPointVectorField::getPosition(int index) const
	{
		int ix = 0, iy = 0, iz = 0;
		getIndices(index, ix, iy, iz);
		return getPosition(ix, iy, iz);
	}

	ZSPACE_INLINE zPoint zFnPointVectorField::getPosition(int index_X, int index_Y, int index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return gridCellPosition3D(field.minBB, field.unit_X, field.unit_Y, field.unit_Z, index_X, index_Y, index_Z);
	}

	ZSPACE_INLINE void zFnPointVectorField::getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildRingNeighbours3D(index, numRings, field.n_X, field.n_Y, field.n_Z, outRingNeighbours);
	}

	ZSPACE_INLINE void zFnPointVectorField::getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildAdjacentNeighbours3D(index, field.n_X, field.n_Y, field.n_Z, outAdjacentNeighbours);
	}

	ZSPACE_INLINE void zFnPointVectorField::getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		buildContainedNeighbours3D(pos, field.minBB, field.unit_X, field.unit_Y, field.unit_Z, field.n_X, field.n_Y, field.n_Z, containedNeighbour);
	}

	ZSPACE_INLINE void zFnPointVectorField::getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour)
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

	ZSPACE_INLINE int zFnPointVectorField::numFieldValues() const
	{
		return static_cast<int>(zFieldObjectStorage::get(*fieldObj).fieldValues.size());
	}

	ZSPACE_INLINE void zFnPointVectorField::getResolution(int& _n_X, int& _n_Y, int& _n_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_n_X = field.n_X;
		_n_Y = field.n_Y;
		_n_Z = field.n_Z;
	}

	ZSPACE_INLINE void zFnPointVectorField::getUnitDistances(double& _unit_X, double& _unit_Y, double& _unit_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_unit_X = field.unit_X;
		_unit_Y = field.unit_Y;
		_unit_Z = field.unit_Z;
	}

	ZSPACE_INLINE void zFnPointVectorField::getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		_minBB = field.minBB;
		_maxBB = field.maxBB;
	}

	ZSPACE_INLINE void zFnPointVectorField::getPositions(zPointArray& positions)
	{
		fnPoints.getVertexPositions(positions);
	}

	ZSPACE_INLINE zVector zFnPointVectorField::getFieldValue(int index) const
	{
		return zFieldObjectStorage::get(*fieldObj).fieldValues[index];
	}

	ZSPACE_INLINE bool zFnPointVectorField::getFieldValue(zPoint& samplePos, zFieldValueType type, zVector& fieldValue)
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return sampleField3D(
			field.fieldValues.data(),
			field.fieldValues.size(),
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.unit_Z,
			field.n_X,
			field.n_Y,
			field.n_Z,
			samplePos,
			type,
			fieldValue);
	}

	ZSPACE_INLINE void zFnPointVectorField::getFieldValues(zVectorArray& fieldValues)
	{
		fieldValues = zFieldObjectStorage::get(*fieldObj).fieldValues;
	}

	ZSPACE_INLINE zVector* zFnPointVectorField::getRawFieldValues()
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (vals.empty()) throw std::invalid_argument(" error: null pointer.");
		return vals.data();
	}

	ZSPACE_INLINE void zFnPointVectorField::setFieldColorDomain(zDomainColor& colDomain)
	{
		fieldColorDomain = colDomain;
	}

	ZSPACE_INLINE void zFnPointVectorField::setBoundingBox(zPoint& _minBB, zPoint& _maxBB)
	{
		auto& field = zFieldObjectStorage::get(*fieldObj);
		field.minBB = _minBB;
		field.maxBB = _maxBB;
	}

	ZSPACE_INLINE void zFnPointVectorField::setFieldValue(int index, const zVector& val, bool append)
	{
		auto& vals = zFieldObjectStorage::get(*fieldObj).fieldValues;
		if (append) vals[index] += val;
		else vals[index] = val;
	}

	ZSPACE_INLINE void zFnPointVectorField::setFieldValues(zVectorArray& fValues)
	{
		auto& field = zFieldObjectStorage::get(*fieldObj);
		if (fValues.size() != field.fieldValues.size())
		{
			throw std::invalid_argument("input fValues size not field vectors.");
		}
		field.fieldValues = fValues;
	}

	//---- 3D IDW FIELD METHODS (Vector)

	ZSPACE_INLINE void zFnPointVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectMesh& inMeshObj, zVector meshValue, double influence, double power, bool normalise)
	{
		const auto& inData = zMeshObjectStorage::read(inMeshObj);
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridPts[i];

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

	ZSPACE_INLINE void zFnPointVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectGraph& inGraphObj, zVector graphValue, double influence, double power, bool normalise)
	{
		const auto& inData = zGraphObjectStorage::read(inGraphObj);
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inData.positions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			double tempDist = 10000.0;
			zPoint gp = gridPts[i];

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

	ZSPACE_INLINE void zFnPointVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectPointCloud& inPointsObj, zVector value, double influence, double power, bool normalise)
	{
		getFieldValuesAsVertexDistance_IDW(fieldValues, inPointsObj.impl->storage.vertexPositions, value, influence, power, normalise);
	}

	ZSPACE_INLINE void zFnPointVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectPointCloud& inPointsObj, zVectorArray& values, vector<double>& influences, double power, bool normalise)
	{
		getFieldValuesAsVertexDistance_IDW(fieldValues, inPointsObj.impl->storage.vertexPositions, values, influences, power, normalise);
	}

	ZSPACE_INLINE void zFnPointVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zPointArray& inPositions, zVector value, double influence, double power, bool normalise)
	{
		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			zPoint gp = gridPts[i];

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

	ZSPACE_INLINE void zFnPointVectorField::getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zPointArray& inPositions, zVectorArray& values, vector<double>& influences, double power, bool normalise)
	{
		if (inPositions.size() != values.size()) throw std::invalid_argument(" error: size of inPositions and values dont match.");
		if (inPositions.size() != influences.size()) throw std::invalid_argument(" error: size of inPositions and influences dont match.");

		const auto& gridPts = fieldObj->impl->storage.vertexPositions;
		const std::size_t nGrid = gridPts.size();
		const std::size_t nIn = inPositions.size();
		fieldValues.resize(nGrid);

		for (std::size_t i = 0; i < nGrid; ++i)
		{
			zVector d;
			double wSum = 0.0;
			zPoint gp = gridPts[i];

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

	ZSPACE_INLINE bool zFnPointVectorField::checkPositionBounds(zPoint& pos, int& index) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		int ix = -1, iy = -1, iz = -1;
		const bool inBounds = computeCellCoords3D(pos, field.minBB, field.unit_X, field.unit_Y, field.unit_Z, field.n_X, field.n_Y, field.n_Z, ix, iy, iz);
		index = ix * field.n_Y * field.n_Z + iy * field.n_Z + iz;
		return inBounds;
	}

	ZSPACE_INLINE bool zFnPointVectorField::checkBounds_X(int index_X) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_X >= 0 && index_X < field.n_X);
	}

	ZSPACE_INLINE bool zFnPointVectorField::checkBounds_Y(int index_Y) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_Y >= 0 && index_Y < field.n_Y);
	}

	ZSPACE_INLINE bool zFnPointVectorField::checkBounds_Z(int index_Z) const
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		return (index_Z >= 0 && index_Z < field.n_Z);
	}

	ZSPACE_INLINE void zFnPointVectorField::computeMinMaxOfScalars(zVectorArray& values, zVector& dMin, zVector& dMax)
	{
		dMin = coreUtils.zMin(values);
		dMax = coreUtils.zMax(values);
	}

	ZSPACE_INLINE void zFnPointVectorField::computeDomain(zVectorArray& values, zDomain<zVector>& domain)
	{
		domain.min = coreUtils.zMin(values);
		domain.max = coreUtils.zMax(values);
	}

	ZSPACE_INLINE void zFnPointVectorField::normliseValues(zVectorArray& fieldValues)
	{
		for (std::size_t i = 0; i < fieldValues.size(); ++i)
		{
			fieldValues[i].normalize();
		}
	}

	ZSPACE_INLINE void zFnPointVectorField::computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions)
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

	ZSPACE_INLINE void zFnPointVectorField::computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies)
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

	ZSPACE_INLINE double zFnPointVectorField::F_of_r(double& r, double& a, double& b)
	{
		if (0 <= r && r <= b / 3.0) return (a * (1.0 - (3.0 * r * r) / (b * b)));
		if (b / 3.0 <= r && r <= b) return (3 * a / 2 * std::pow(1.0 - (r / b), 2.0));
		return 0.0;
	}

	ZSPACE_INLINE void zFnPointVectorField::createPointCloud()
	{
		const auto& field = zFieldObjectStorage::get(*fieldObj);
		zFieldObjectStorage::populateGridPointCloud3D(
			*fieldObj,
			field.minBB,
			field.unit_X,
			field.unit_Y,
			field.unit_Z,
			field.n_X,
			field.n_Y,
			field.n_Z);
	}
}
