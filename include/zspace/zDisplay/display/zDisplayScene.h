#ifndef ZSPACE_DISPLAY_SCENE_H
#define ZSPACE_DISPLAY_SCENE_H

#pragma once

#include <zspace/zCore/base/zColor.h>
#include <zspace/export.h>
#include <zspace/fwd.h>

#include <memory>
#include <vector>

namespace zSpace
{
	class ZSPACE_DISPLAY zDisplayScene
	{
	public:
		zDisplayScene();
		explicit zDisplayScene(int bufferSize);
		~zDisplayScene();

		zDisplayScene(const zDisplayScene&) = delete;
		zDisplayScene& operator=(const zDisplayScene&) = delete;
		zDisplayScene(zDisplayScene&&) noexcept;
		zDisplayScene& operator=(zDisplayScene&&) noexcept;

		zDisplayMesh& mesh(zObjectMesh& object);
		zDisplayGraph& graph(zObjectGraph& object);

		void draw(zObjectMesh& object);
		void draw(zObjectGraph& object);
		void draw(zObjectMesh& object, const zDisplayMesh& display);
		void draw(zObjectGraph& object, const zDisplayGraph& display);

		void appendToBuffer(
			zObjectMesh& object,
			const std::vector<double>& dihedralAngles = {},
			bool dihedralEdgesOnly = false,
			double angleThreshold = 45.0);
		void appendToBuffer(zObjectGraph& object);

		void drawBufferedPoints(bool colors = true);
		void drawBufferedLines(bool colors = true);
		void drawBufferedTriangles(bool colors = true);
		void drawBufferedQuads(bool colors = true);

		void drawPoint(zVector& position, const zColor& color = zColor(1, 0, 0, 1), double weight = 1.0);
		void drawPoints(zVector* positions, zColor* colors, double* weights, int count);
		void drawPoints(zVector* positions, zColor color, double weight, int count);
		void drawLine(zVector& start, zVector& end, const zColor& color = zColor(1, 0, 0, 1), double weight = 1.0);
		void drawCurve(zVector* positions, zColor color, double weight, int count, bool closed);
		void drawCube(zVector& minBounds, zVector& maxBounds, const zColor& color = zColor(1, 0, 0, 1), double weight = 1.0);
		void drawTransform(zTransformationMatrix& transform, float scale = 0.1f);

		int appendVertexAttributes(zVector* positions, zVector* normals, int count);
		int appendVertexColors(zColor* colors, int count);

	private:
		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> impl_;

		friend class zDisplayMesh;
		friend class zDisplayGraph;
	};
}

#endif
