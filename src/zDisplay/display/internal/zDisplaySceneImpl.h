#ifndef ZSPACE_DISPLAY_SCENE_IMPL_H
#define ZSPACE_DISPLAY_SCENE_IMPL_H

#pragma once

#include <src/zDisplay/display/internal/zUtilsDisplay.h>
#include <zspace/zDisplay/display/zDisplayGraph.h>
#include <zspace/zDisplay/display/zDisplayMesh.h>
#include <zspace/zDisplay/display/zDisplayScene.h>

#include <unordered_map>

namespace zSpace
{
	struct zDisplayBufferOffsets
	{
		int vertex = -1;
		int edge = -1;
		int face = -1;
		int vertexColor = -1;
	};

	class zDisplayScene::Impl
	{
	public:
		Impl() = default;
		explicit Impl(int bufferSize) : backend(bufferSize) {}

		zUtilsDisplay backend;
		std::unordered_map<const zObjectMesh*, zDisplayMesh> meshes;
		std::unordered_map<const zObjectGraph*, zDisplayGraph> graphs;
		std::unordered_map<const zObjectMesh*, zDisplayBufferOffsets> meshBuffers;
		std::unordered_map<const zObjectGraph*, zDisplayBufferOffsets> graphBuffers;
	};
}

#endif
