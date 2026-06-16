#include <zspace/interop.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace
{
	using namespace zSpace;

	void require(bool condition, const char* message)
	{
		if (!condition) throw std::runtime_error(message);
	}

	void requireSuccess(const zIOResult& result)
	{
		if (!result) throw std::runtime_error(result.message());
	}

	void testPlane(const std::filesystem::path& directory)
	{
		zObjectPlane source;
		zFnPlane sourceFn(source);
		require(sourceFn.create(zPoint(1, 2, 3), zVector(1, 0, 0), zVector(0, 1, 0)), "plane create");

		const auto path = directory / "plane.json";
		requireSuccess(zIO::writePlane(path.string(), source));

		zObjectPlane roundTrip;
		requireSuccess(zIO::readPlane(path.string(), roundTrip));

		require(roundTrip.origin.x == 1.0f && roundTrip.origin.z == 3.0f, "plane origin round trip");
		require(roundTrip.xAxis.x > 0.99f && roundTrip.yAxis.y > 0.99f, "plane axes round trip");
	}

	void testArc(const std::filesystem::path& directory)
	{
		zObjectPlane plane;
		zFnPlane planeFn(plane);
		require(planeFn.create(zPoint(0, 0, 0), zVector(1, 0, 0), zVector(0, 1, 0)), "arc plane create");

		zObjectArc source;
		zFnArc sourceFn(source);
		require(sourceFn.create(plane, 4.0, Z_PI), "arc create");

		const auto path = directory / "arc.json";
		requireSuccess(zIO::writeArc(path.string(), source));

		zObjectArc roundTrip;
		requireSuccess(zIO::readArc(path.string(), roundTrip));

		require(roundTrip.radius == 4.0, "arc radius round trip");
		require(roundTrip.angle == Z_PI, "arc angle round trip");
		require(!roundTrip.arcPositions.empty(), "arc display positions rebuilt");
	}

	void testNurbsCurve(const std::filesystem::path& directory)
	{
		zObjectNurbsCurve source;
		zFnNurbsCurve sourceFn(source);
		zPointArray controlPoints = {
			zPoint(0, 0, 0),
			zPoint(1, 2, 0),
			zPoint(3, 2, 0),
			zPoint(4, 0, 0)
		};
		sourceFn.create(controlPoints, 3, false, false, 12);

		const auto path = directory / "curve.json";
		requireSuccess(zIO::writeNurbsCurve(path.string(), source));

		zObjectNurbsCurve roundTrip;
		requireSuccess(zIO::readNurbsCurve(path.string(), roundTrip));

		zFnNurbsCurve roundTripFn(roundTrip);
		require(roundTripFn.numControlVertices() == 4, "nurbs curve control vertices round trip");
		require(roundTrip.getNumDisplayPositions() == 12, "nurbs curve display count round trip");
	}
}

int main()
{
	try
	{
		const auto directory = std::filesystem::temp_directory_path() / "zspace_interop_io_smoke";
		std::filesystem::create_directories(directory);

		testPlane(directory);
		testArc(directory);
		testNurbsCurve(directory);

		std::filesystem::remove_all(directory);
		std::cout << "zspace InterOp IO smoke tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "zspace InterOp IO smoke tests failed: " << error.what() << '\n';
		return 1;
	}
}
