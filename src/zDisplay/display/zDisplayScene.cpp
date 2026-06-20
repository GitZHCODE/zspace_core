#include <zspace/zDisplay/display/zDisplayScene.h>

#include <src/zDisplay/display/internal/zDisplaySceneImpl.h>
#include <zspace/zInterface/objects/zObjectGraph.h>
#include <zspace/zInterface/objects/zObjectMesh.h>

namespace zSpace
{
	ZSPACE_INLINE zDisplayScene::zDisplayScene()
		: impl_(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zDisplayScene::zDisplayScene(int bufferSize)
		: impl_(std::make_unique<Impl>(bufferSize))
	{
	}

	ZSPACE_INLINE zDisplayScene::~zDisplayScene() = default;
	ZSPACE_INLINE zDisplayScene::zDisplayScene(zDisplayScene&&) noexcept = default;
	ZSPACE_INLINE zDisplayScene& zDisplayScene::operator=(zDisplayScene&&) noexcept = default;

	ZSPACE_INLINE zDisplayMesh& zDisplayScene::mesh(zObjectMesh& object)
	{
		return impl_->meshes[&object];
	}

	ZSPACE_INLINE zDisplayGraph& zDisplayScene::graph(zObjectGraph& object)
	{
		return impl_->graphs[&object];
	}

	ZSPACE_INLINE void zDisplayScene::draw(zObjectMesh& object)
	{
		draw(object, mesh(object));
	}

	ZSPACE_INLINE void zDisplayScene::draw(zObjectGraph& object)
	{
		draw(object, graph(object));
	}

	ZSPACE_INLINE void zDisplayScene::draw(zObjectMesh& object, const zDisplayMesh& display)
	{
		display.draw(object, *this);
	}

	ZSPACE_INLINE void zDisplayScene::draw(zObjectGraph& object, const zDisplayGraph& display)
	{
		display.draw(object, *this);
	}

	ZSPACE_INLINE void zDisplayScene::appendToBuffer(
		zObjectMesh& object,
		const zDoubleArray& dihedralAngles,
		bool dihedralEdgesOnly,
		double angleThreshold)
	{
		zDisplayMesh().appendToBuffer(
			object,
			*this,
			dihedralAngles,
			dihedralEdgesOnly,
			angleThreshold);
	}

	ZSPACE_INLINE void zDisplayScene::appendToBuffer(zObjectGraph& object)
	{
		zDisplayGraph().appendToBuffer(object, *this);
	}

	ZSPACE_INLINE void zDisplayScene::drawBufferedPoints(bool colors)
	{
		impl_->backend.drawPointsFromBuffer(colors);
	}

	ZSPACE_INLINE void zDisplayScene::drawBufferedLines(bool colors)
	{
		impl_->backend.drawLinesFromBuffer(colors);
	}

	ZSPACE_INLINE void zDisplayScene::drawBufferedTriangles(bool colors)
	{
		impl_->backend.drawTrianglesFromBuffer(colors);
	}

	ZSPACE_INLINE void zDisplayScene::drawBufferedQuads(bool colors)
	{
		impl_->backend.drawQuadsFromBuffer(colors);
	}

	ZSPACE_INLINE void zDisplayScene::drawPoint(zPoint& position, const zColor& color, double weight)
	{
		impl_->backend.drawPoint(position, color, weight);
	}

	ZSPACE_INLINE void zDisplayScene::drawPoints(zPoint* positions, zColor* colors, double* weights, int count)
	{
		impl_->backend.drawPoints(positions, colors, weights, count);
	}

	ZSPACE_INLINE void zDisplayScene::drawPoints(zPoint* positions, zColor color, double weight, int count)
	{
		impl_->backend.drawPoints(positions, color, weight, count);
	}

	ZSPACE_INLINE void zDisplayScene::drawLine(
		zPoint& start,
		zPoint& end,
		const zColor& color,
		double weight)
	{
		impl_->backend.drawLine(start, end, color, weight);
	}

	ZSPACE_INLINE void zDisplayScene::drawCurve(
		zPoint* positions,
		zColor color,
		double weight,
		int count,
		bool closed)
	{
		impl_->backend.drawCurve(positions, color, weight, count, closed);
	}

	ZSPACE_INLINE void zDisplayScene::drawCube(
		zPoint& minBounds,
		zPoint& maxBounds,
		const zColor& color,
		double weight)
	{
		impl_->backend.drawCube(minBounds, maxBounds, color, weight);
	}

	ZSPACE_INLINE void zDisplayScene::drawTransform(zTransformationMatrix& transform, float scale)
	{
		impl_->backend.drawTransform(transform, scale);
	}

	ZSPACE_INLINE int zDisplayScene::appendVertexAttributes(
		zPoint* positions,
		zVector* normals,
		int count)
	{
		return impl_->backend.bufferObj.appendVertexAttributes(positions, normals, count);
	}

	ZSPACE_INLINE int zDisplayScene::appendVertexColors(zColor* colors, int count)
	{
		return impl_->backend.bufferObj.appendVertexColors(colors, count);
	}
}
