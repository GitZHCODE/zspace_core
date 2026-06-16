#ifndef ZSPACE_DISPLAY_MESH_H
#define ZSPACE_DISPLAY_MESH_H

#pragma once

#include <zspace/export.h>
#include <zspace/fwd.h>

#include <memory>
#include <vector>

namespace zSpace
{
	class zDisplayScene;

	class ZSPACE_DISPLAY zDisplayMesh
	{
	public:
		zDisplayMesh();
		~zDisplayMesh();

		zDisplayMesh(const zDisplayMesh& other);
		zDisplayMesh(zDisplayMesh&& other) noexcept;
		zDisplayMesh& operator=(const zDisplayMesh& other);
		zDisplayMesh& operator=(zDisplayMesh&& other) noexcept;

		void setDisplayObject(bool visible);
		void setDisplayTransform(bool visible);
		void setObjectVisible(bool visible);
		void setTransformVisible(bool visible);

		void setDisplayElements(bool vertices, bool edges, bool faces);
		void setDisplayElementIds(bool vertices, bool edges, bool faces);
		void setDisplayVertices(bool visible);
		void setDisplayEdges(bool visible);
		void setDisplayFaces(bool visible);
		void setDisplayDihedralEdges(bool visible, double threshold = 45.0);
		void setDisplayVertexNormals(bool visible, double scale = 1.0);
		void setDisplayFaceNormals(bool visible, double scale = 1.0);

		void setElements(bool vertices, bool edges, bool faces);
		void setElementIds(bool vertices, bool edges, bool faces);
		void setVerticesVisible(bool visible);
		void setEdgesVisible(bool visible);
		void setFacesVisible(bool visible);
		void setDihedralEdgesVisible(bool visible, double threshold = 45.0);
		void setVertexNormalsVisible(bool visible, double scale = 1.0);
		void setFaceNormalsVisible(bool visible, double scale = 1.0);
		void setFaceCenters(const std::vector<zVector>& centers);
		void setEdgeCenters(const std::vector<zVector>& centers);
		void setDihedralAngles(const std::vector<double>& angles);

		bool objectVisible() const;
		bool transformVisible() const;
		bool getDisplayObject() const;
		bool getDisplayTransform() const;
		bool verticesVisible() const;
		bool edgesVisible() const;
		bool facesVisible() const;
		bool vertexIdsVisible() const;
		bool edgeIdsVisible() const;
		bool faceIdsVisible() const;
		bool dihedralEdgesVisible() const;
		bool vertexNormalsVisible() const;
		bool faceNormalsVisible() const;
		double dihedralAngleThreshold() const;
		double normalScale() const;
		const std::vector<zVector>& faceCenters() const;
		const std::vector<zVector>& edgeCenters() const;
		const std::vector<double>& dihedralAngles() const;

	private:
		void draw(zObjectMesh& object, zDisplayScene& scene) const;
		void appendToBuffer(
			zObjectMesh& object,
			zDisplayScene& scene,
			const std::vector<double>& dihedralAngles,
			bool dihedralEdgesOnly,
			double angleThreshold) const;
		void drawElements(zObjectMesh& object, zDisplayScene& scene) const;
		void drawDihedralEdges(zObjectMesh& object, zDisplayScene& scene) const;
		void drawVertexNormals(zObjectMesh& object, zDisplayScene& scene) const;
		void drawFaceNormals(zObjectMesh& object, zDisplayScene& scene) const;

		friend class zDisplayScene;

		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> impl_;
	};
}

#endif
