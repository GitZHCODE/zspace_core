#ifndef ZSPACE_DISPLAY_GRAPH_H
#define ZSPACE_DISPLAY_GRAPH_H

#pragma once

#include <zspace/export.h>
#include <zspace/fwd.h>

#include <memory>
#include <vector>

namespace zSpace
{
	class zDisplayScene;

	class ZSPACE_DISPLAY zDisplayGraph
	{
	public:
		zDisplayGraph();
		~zDisplayGraph();

		zDisplayGraph(const zDisplayGraph& other);
		zDisplayGraph(zDisplayGraph&& other) noexcept;
		zDisplayGraph& operator=(const zDisplayGraph& other);
		zDisplayGraph& operator=(zDisplayGraph&& other) noexcept;

		void setObjectVisible(bool visible);
		void setTransformVisible(bool visible);
		void setElements(bool vertices, bool edges);
		void setElementIds(bool vertices, bool edges);
		void setVerticesVisible(bool visible);
		void setEdgesVisible(bool visible);
		void setEdgeCenters(const std::vector<zVector>& centers);

		bool objectVisible() const;
		bool transformVisible() const;
		bool verticesVisible() const;
		bool edgesVisible() const;
		bool vertexIdsVisible() const;
		bool edgeIdsVisible() const;
		const std::vector<zVector>& edgeCenters() const;

	private:
		void draw(zObjectGraph& object, zDisplayScene& scene) const;
		void appendToBuffer(zObjectGraph& object, zDisplayScene& scene) const;
		void drawElements(zObjectGraph& object, zDisplayScene& scene) const;

		friend class zDisplayScene;

		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> impl_;
	};
}

#endif
