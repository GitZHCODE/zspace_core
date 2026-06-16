#ifndef ZSPACE_IO_H
#define ZSPACE_IO_H

#pragma once

#include <zspace/zIO/zIOResult.h>

#include <string>

namespace zSpace
{
	class zObjectGraph;
	class zObjectMesh;
	class zObjectPointCloud;
	using zObjGraph = zObjectGraph;
	using zObjMesh = zObjectMesh;
	using zObjPointCloud = zObjectPointCloud;
#if defined(ZSPACE_RHINO_INTEROP)
	class zObjectArc;
	class zObjectNurbsCurve;
	class zObjectPlane;
	using zObjArc = zObjectArc;
	using zObjNurbsCurve = zObjectNurbsCurve;
	using zObjPlane = zObjectPlane;
#endif

	class zIO
	{
	public:
		static ZSPACE_IO zIOResult readMesh(const std::string& path, zObjectMesh& mesh, bool staticGeometry = false);
		static ZSPACE_IO zIOResult writeMesh(const std::string& path, zObjectMesh& mesh);

		static ZSPACE_IO zIOResult readGraph(const std::string& path, zObjectGraph& graph, bool staticGeometry = false);
		static ZSPACE_IO zIOResult writeGraph(const std::string& path, zObjectGraph& graph);

		static ZSPACE_IO zIOResult readPointCloud(const std::string& path, zObjectPointCloud& points);
		static ZSPACE_IO zIOResult writePointCloud(const std::string& path, zObjectPointCloud& points);

#if defined(ZSPACE_RHINO_INTEROP)
		static ZSPACE_INTEROP zIOResult readPlane(const std::string& path, zObjectPlane& plane);
		static ZSPACE_INTEROP zIOResult writePlane(const std::string& path, zObjectPlane& plane);

		static ZSPACE_INTEROP zIOResult readArc(const std::string& path, zObjectArc& arc);
		static ZSPACE_INTEROP zIOResult writeArc(const std::string& path, zObjectArc& arc);

		static ZSPACE_INTEROP zIOResult readNurbsCurve(const std::string& path, zObjectNurbsCurve& curve);
		static ZSPACE_INTEROP zIOResult writeNurbsCurve(const std::string& path, zObjectNurbsCurve& curve);
#endif
	};
}

#endif
