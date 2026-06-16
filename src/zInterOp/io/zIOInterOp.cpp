#include <zspace/zIO/zIO.h>

#if defined(ZSPACE_RHINO_INTEROP)

#include <zspace/zInterOp/functionsets/zFnArc.h>
#include <zspace/zInterOp/functionsets/zFnNurbsCurve.h>
#include <zspace/zInterOp/functionsets/zFnPlane.h>
#include <zspace/zInterOp/objects/zObjectArc.h>
#include <zspace/zInterOp/objects/zObjectNurbsCurve.h>
#include <zspace/zInterOp/objects/zObjectPlane.h>

#include <depends/nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

namespace zSpace
{
	namespace
	{
		using json = nlohmann::json;

		std::string extensionOf(const std::string& path)
		{
			std::string extension = std::filesystem::path(path).extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
			return extension;
		}

		zIOResult readJsonDocument(const std::string& path, json& document)
		{
			std::ifstream input(path);
			if (!input) return zIOResult::error("Could not open JSON file: " + path);
			try
			{
				input >> document;
			}
			catch (const std::exception& error)
			{
				return zIOResult::error("Could not parse JSON file " + path + ": " + error.what());
			}
			return zIOResult::ok();
		}

		zIOResult writeJsonDocument(const std::string& path, const json& document)
		{
			std::ofstream output(path);
			if (!output) return zIOResult::error("Could not create JSON file: " + path);
			output << document.dump(2) << '\n';
			return output ? zIOResult::ok() : zIOResult::error("Failed while writing JSON file: " + path);
		}

		zIOResult requireJsonExtension(const std::string& path, const char* typeName)
		{
			const std::string extension = extensionOf(path);
			if (extension == ".json") return zIOResult::ok();
			return zIOResult::error(std::string("Unsupported ") + typeName + " file extension: " + extension);
		}

		json pointToJson(const zPoint& point)
		{
			return json::array({ point.x, point.y, point.z });
		}

		json vectorToJson(const zVector& vector)
		{
			return json::array({ vector.x, vector.y, vector.z });
		}

		json colorToJson(const zColor& color)
		{
			return json::array({ color.r, color.g, color.b, color.a });
		}

		zPoint pointFromJson(const json& value)
		{
			if (!value.is_array() || value.size() < 3)
				throw std::runtime_error("Expected [x, y, z] point array.");
			return zPoint(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
		}

		zVector vectorFromJson(const json& value)
		{
			if (!value.is_array() || value.size() < 3)
				throw std::runtime_error("Expected [x, y, z] vector array.");
			return zVector(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
		}

		zColor colorFromJson(const json& value)
		{
			if (!value.is_array() || value.size() < 3)
				throw std::runtime_error("Expected [r, g, b, a] color array.");
			const float alpha = value.size() > 3 ? value[3].get<float>() : 1.0f;
			return zColor(value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), alpha);
		}

		zIOResult applyPlaneJson(const json& document, zObjectPlane& plane)
		{
			try
			{
				const zPoint origin = pointFromJson(document.at("origin"));
				const zVector xAxis = vectorFromJson(document.at("xAxis"));
				const zVector yAxis = vectorFromJson(document.at("yAxis"));
				zFnPlane functionSet(plane);
				if (!functionSet.create(origin, xAxis, yAxis))
					return zIOResult::error("Plane JSON contains invalid axes.");

				if (document.contains("displayAxis")) plane.displayAxis = document["displayAxis"].get<bool>();
				if (document.contains("displayRectangle")) plane.displayRectangle = document["displayRectangle"].get<bool>();
				if (document.contains("displayAxisScale")) plane.displayAxisScale = document["displayAxisScale"].get<double>();
				if (document.contains("displayRectangleScale")) plane.displayRectangleScale = document["displayRectangleScale"].get<double>();
				if (document.contains("displayWeight")) plane.displayWeight = document["displayWeight"].get<double>();
				if (document.contains("displayColor") && document["displayColor"].is_array())
				{
					plane.displayColor.clear();
					for (const auto& color : document["displayColor"])
						plane.displayColor.push_back(colorFromJson(color));
				}
			}
			catch (const std::exception& error)
			{
				return zIOResult::error(std::string("Invalid plane JSON: ") + error.what());
			}
			return zIOResult::ok();
		}

		json planeToJson(zObjectPlane& plane)
		{
			json document;
			document["schema"] = "zspace.plane.v1";
			document["origin"] = pointToJson(plane.origin);
			document["xAxis"] = vectorToJson(plane.xAxis);
			document["yAxis"] = vectorToJson(plane.yAxis);
			document["normal"] = vectorToJson(plane.normal);
			document["displayAxis"] = plane.displayAxis;
			document["displayRectangle"] = plane.displayRectangle;
			document["displayAxisScale"] = plane.displayAxisScale;
			document["displayRectangleScale"] = plane.displayRectangleScale;
			document["displayWeight"] = plane.displayWeight;
			document["displayColor"] = json::array();
			for (const auto& color : plane.displayColor)
				document["displayColor"].push_back(colorToJson(color));
			return document;
		}

		zIOResult applyArcJson(const json& document, zObjectArc& arc)
		{
			try
			{
				zObjectPlane plane;
				auto planeResult = applyPlaneJson(document.at("plane"), plane);
				if (!planeResult) return planeResult;

				const double radius = document.at("radius").get<double>();
				const double angle = document.value("angle", Z_TWO_PI);
				zFnArc functionSet(arc);
				if (!functionSet.create(plane, radius, angle))
					return zIOResult::error("Arc JSON contains invalid radius or angle.");

				if (document.contains("displayPlane")) arc.displayPlane = document["displayPlane"].get<bool>();
				if (document.contains("displayControlPoints")) arc.displayControlPoints = document["displayControlPoints"].get<bool>();
				if (document.contains("displayArc")) arc.displayArc = document["displayArc"].get<bool>();
				if (document.contains("arcDisplayColor")) arc.arcDisplayColor = colorFromJson(document["arcDisplayColor"]);
				if (document.contains("arcDisplayWeight")) arc.arcDisplayWeight = document["arcDisplayWeight"].get<double>();
				if (document.contains("controlPointsColor")) arc.controlPointsColor = colorFromJson(document["controlPointsColor"]);
				if (document.contains("controlPolyColor")) arc.controlPolyColor = colorFromJson(document["controlPolyColor"]);
				if (document.contains("controlPointsWeight")) arc.controlPointsWeight = document["controlPointsWeight"].get<double>();
				if (document.contains("controlPolyWeight")) arc.controlPolyWeight = document["controlPolyWeight"].get<double>();
			}
			catch (const std::exception& error)
			{
				return zIOResult::error(std::string("Invalid arc JSON: ") + error.what());
			}
			return zIOResult::ok();
		}

		json arcToJson(zObjectArc& arc)
		{
			json document;
			document["schema"] = "zspace.arc.v1";
			document["plane"] = planeToJson(arc.oPlane);
			document["radius"] = arc.radius;
			document["angle"] = arc.angle;
			document["isCircle"] = arc.isCircle;
			document["displayPlane"] = arc.displayPlane;
			document["displayControlPoints"] = arc.displayControlPoints;
			document["displayArc"] = arc.displayArc;
			document["arcDisplayColor"] = colorToJson(arc.arcDisplayColor);
			document["arcDisplayWeight"] = arc.arcDisplayWeight;
			document["controlPointsColor"] = colorToJson(arc.controlPointsColor);
			document["controlPolyColor"] = colorToJson(arc.controlPolyColor);
			document["controlPointsWeight"] = arc.controlPointsWeight;
			document["controlPolyWeight"] = arc.controlPolyWeight;
			return document;
		}

		zIOResult applyNurbsCurveJson(const json& document, zObjectNurbsCurve& curve)
		{
			try
			{
				zPointArray controlPoints;
				for (const auto& point : document.at("controlPoints"))
					controlPoints.push_back(pointFromJson(point));

				if (controlPoints.size() < 2)
					return zIOResult::error("Nurbs curve JSON contains fewer than two control points.");

				const int degree = document.value("degree", 1);
				const bool periodic = document.value("periodic", false);
				const bool interpolate = document.value("interpolate", false);
				const int displayNumPoints = document.value("displayNumPoints", 20);

				zFnNurbsCurve functionSet(curve);
				functionSet.create(controlPoints, degree, periodic, interpolate, displayNumPoints);

				if (document.contains("weights") && document["weights"].is_array())
				{
					zDoubleArray weights;
					for (const auto& weight : document["weights"])
						weights.push_back(weight.get<double>());
					curve.setControlPointWeights(weights);
				}
				if (document.contains("displayColor")) curve.displayColor = colorFromJson(document["displayColor"]);
				if (document.contains("displayWeight")) curve.displayWeight = document["displayWeight"].get<double>();
			}
			catch (const std::exception& error)
			{
				return zIOResult::error(std::string("Invalid nurbs curve JSON: ") + error.what());
			}
			return zIOResult::ok();
		}

		json nurbsCurveToJson(zObjectNurbsCurve& curve)
		{
			json document;
			document["schema"] = "zspace.nurbsCurve.v1";
			document["degree"] = curve.getDegree();
			document["periodic"] = curve.isPeriodic();
			document["interpolate"] = false;
			document["displayNumPoints"] = curve.getNumDisplayPositions();
			document["controlPoints"] = json::array();
			document["weights"] = json::array();

			for (int i = 0; i < curve.curve.CVCount(); ++i)
			{
				ON_4dPoint point;
				curve.curve.GetCV(i, point);
				const double weight = point.w == 0.0 ? 1.0 : point.w;
				document["controlPoints"].push_back(json::array({ point.x / weight, point.y / weight, point.z / weight }));
				document["weights"].push_back(weight);
			}

			zPointArray displayPositions;
			curve.getDisplayPositions(displayPositions);
			document["displayPositions"] = json::array();
			for (const auto& point : displayPositions)
				document["displayPositions"].push_back(pointToJson(point));

			return document;
		}
	}

	zIOResult zIO::readPlane(const std::string& path, zObjectPlane& plane)
	{
		auto extensionResult = requireJsonExtension(path, "plane");
		if (!extensionResult) return extensionResult;

		json document;
		auto result = readJsonDocument(path, document);
		if (!result) return result;
		return applyPlaneJson(document, plane);
	}

	zIOResult zIO::writePlane(const std::string& path, zObjectPlane& plane)
	{
		auto extensionResult = requireJsonExtension(path, "plane");
		if (!extensionResult) return extensionResult;
		return writeJsonDocument(path, planeToJson(plane));
	}

	zIOResult zIO::readArc(const std::string& path, zObjectArc& arc)
	{
		auto extensionResult = requireJsonExtension(path, "arc");
		if (!extensionResult) return extensionResult;

		json document;
		auto result = readJsonDocument(path, document);
		if (!result) return result;
		return applyArcJson(document, arc);
	}

	zIOResult zIO::writeArc(const std::string& path, zObjectArc& arc)
	{
		auto extensionResult = requireJsonExtension(path, "arc");
		if (!extensionResult) return extensionResult;
		return writeJsonDocument(path, arcToJson(arc));
	}

	zIOResult zIO::readNurbsCurve(const std::string& path, zObjectNurbsCurve& curve)
	{
		auto extensionResult = requireJsonExtension(path, "nurbs curve");
		if (!extensionResult) return extensionResult;

		json document;
		auto result = readJsonDocument(path, document);
		if (!result) return result;
		return applyNurbsCurveJson(document, curve);
	}

	zIOResult zIO::writeNurbsCurve(const std::string& path, zObjectNurbsCurve& curve)
	{
		auto extensionResult = requireJsonExtension(path, "nurbs curve");
		if (!extensionResult) return extensionResult;
		return writeJsonDocument(path, nurbsCurveToJson(curve));
	}
}

#endif
