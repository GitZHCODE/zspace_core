#ifndef ZSPACE_MESH_OBJECT_STORAGE_H
#define ZSPACE_MESH_OBJECT_STORAGE_H

#pragma once

#include <src/zCore/geometry/detail/zMeshStorage.h>
#include <zspace/zInterface/objects/zObjectMesh.h>

namespace zSpace
{
	class zObjectMesh::Impl
	{
	public:
		zMesh mesh;
	};

	class zMeshObjectStorage
	{
	public:
		static zMesh& get(zObjectMesh& object) { return object.impl->mesh; }
		static const zMesh& get(const zObjectMesh& object) { return object.impl->mesh; }
	};
}

#endif
