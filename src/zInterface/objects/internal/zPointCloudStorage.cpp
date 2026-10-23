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


#include <src/zInterface/objects/internal/zPointCloudStorage.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zPointCloud::zPointCloud()
	{
		n_v = 0;
		positionMapDirty = false;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zPointCloud::~zPointCloud() {}

	//---- CREATE METHODS

	ZSPACE_INLINE void zPointCloud::create(zPointArray &_positions)
	{
		clear();

		const int count = static_cast<int>(_positions.size());
		n_v = count;
		vertices.resize(count);
		for (int i = 0; i < count; i++)
		{
			vertices[i].setId(i);
		}
		vertexPositions = _positions;
		vertexColors.assign(count, zColor(1, 0, 0, 1));
		vertexWeights.assign(count, 2.0);
		positionMapDirty = true;
	}

	ZSPACE_INLINE void zPointCloud::clear()
	{
		vertices.clear();
		vertexPositions.clear();
		vertexColors.clear();
		vertexWeights.clear();
		positionVertex.clear();
		positionMapDirty = false;

		n_v = 0;
	}

	//---- VERTEX METHODS

	ZSPACE_INLINE bool zPointCloud::addVertex(zPoint &pos)
	{
		bool out = false;

		if (!positionMapDirty)
		{
			addToPositionMap(pos, n_v);
		}

		zItVertex newV = vertices.insert(vertices.end(), zVertex());
		newV->setId(n_v);


		vertexPositions.push_back(pos);
		n_v++;



		// default Attibute values			
		vertexColors.push_back(zColor(1, 0, 0, 1));
		vertexWeights.push_back(2.0);

		return out;
	}

	ZSPACE_INLINE bool zPointCloud::vertexExists(zPoint &pos, int &outVertexId, int precisionfactor)
	{
		bool out = false;
		outVertexId = -1;

		if (positionMapDirty)
		{
			positionVertex.clear();
			positionVertex.reserve(vertexPositions.size());
			for (int i = 0; i < static_cast<int>(vertexPositions.size()); ++i)
			{
				if (i < static_cast<int>(vertices.size()) && !vertices[i].isActive()) continue;
				addToPositionMap(vertexPositions[i], i, precisionfactor);
			}
			positionMapDirty = false;
		}

		double factor = pow(10, precisionfactor);
		double x = std::round(pos.x *factor) / factor;
		double y = std::round(pos.y *factor) / factor;
		double z = std::round(pos.z *factor) / factor;

		string hashKey = (to_string(x) + "," + to_string(y) + "," + to_string(z));
		std::unordered_map<std::string, int>::const_iterator got = positionVertex.find(hashKey);


		if (got != positionVertex.end())
		{
			out = true;
			outVertexId = got->second;
		}


		return out;
	}

	ZSPACE_INLINE void zPointCloud::setNumVertices(int _n_v)
	{
		n_v = _n_v;
	}

	//---- MAP METHODS

	ZSPACE_INLINE void zPointCloud::addToPositionMap(zPoint &pos, int index, int precisionfactor)
	{
		double factor = pow(10, precisionfactor);
		double x = std::round(pos.x *factor) / factor;
		double y = std::round(pos.y *factor) / factor;
		double z = std::round(pos.z *factor) / factor;

		string hashKey = (to_string(x) + "," + to_string(y) + "," + to_string(z));
		positionVertex[hashKey] = index;
	}

	ZSPACE_INLINE void zPointCloud::removeFromPositionMap(zPoint &pos, int precisionfactor)
	{
		if (positionMapDirty)
		{
			positionVertex.clear();
			positionVertex.reserve(vertexPositions.size());
			for (int i = 0; i < static_cast<int>(vertexPositions.size()); ++i)
			{
				if (i < static_cast<int>(vertices.size()) && !vertices[i].isActive()) continue;
				addToPositionMap(vertexPositions[i], i, precisionfactor);
			}
			positionMapDirty = false;
		}

		double factor = pow(10, precisionfactor);
		double x = std::round(pos.x *factor) / factor;
		double y = std::round(pos.y *factor) / factor;
		double z = std::round(pos.z *factor) / factor;

		string removeHashKey = (to_string(x) + "," + to_string(y) + "," + to_string(z));
		positionVertex.erase(removeHashKey);
	}

}
