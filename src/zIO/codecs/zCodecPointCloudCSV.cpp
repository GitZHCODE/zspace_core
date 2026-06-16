#include <src/zIO/codecs/zCodecPointCloudCSV.h>

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace zSpace::io_detail
{
	zIOResult readPointCloudCSV(const std::string& path, PointCloudData& data)
	{
		std::ifstream input(path);
		if (!input) return zIOResult::error("Could not open point-cloud CSV file: " + path);

		data = {};
		std::string line;
		int lineNumber = 0;
		while (std::getline(input, line))
		{
			++lineNumber;
			if (line.empty() || line[0] == '#') continue;
			for (char& value : line)
				if (value == ',' || value == ';' || value == '\t') value = ' ';

			std::istringstream stream(line);
			std::string first;
			stream >> first;
			if (first.empty()) continue;

			zPoint position;
			if (first == "v")
			{
				if (!(stream >> position.x >> position.y >> position.z))
					return zIOResult::error("Invalid point-cloud row at line " + std::to_string(lineNumber));
			}
			else
			{
				try
				{
					position.x = std::stof(first);
				}
				catch (...)
				{
					if (lineNumber == 1) continue;
					return zIOResult::error("Invalid point-cloud x coordinate at line " + std::to_string(lineNumber));
				}
				if (!(stream >> position.y >> position.z))
					return zIOResult::error("Invalid point-cloud row at line " + std::to_string(lineNumber));
			}

			data.positions.push_back(position);

			zColor color;
			if (stream >> color.r >> color.g >> color.b)
			{
				if (!(stream >> color.a)) color.a = 1.0f;
				data.colors.push_back(color);
			}
		}

		if (data.positions.empty())
			return zIOResult::error("Point-cloud CSV contains no positions: " + path);
		if (!data.colors.empty() && data.colors.size() != data.positions.size())
			data.colors.clear();
		return zIOResult::ok();
	}

	zIOResult writePointCloudCSV(const std::string& path, const PointCloudData& data)
	{
		std::ofstream output(path);
		if (!output) return zIOResult::error("Could not create point-cloud CSV file: " + path);

		output << std::setprecision(std::numeric_limits<float>::max_digits10);
		const bool writeColors = data.colors.size() == data.positions.size();
		output << (writeColors ? "x,y,z,r,g,b,a\n" : "x,y,z\n");

		for (std::size_t i = 0; i < data.positions.size(); ++i)
		{
			const auto& point = data.positions[i];
			output << point.x << ',' << point.y << ',' << point.z;
			if (writeColors)
			{
				const auto& color = data.colors[i];
				output << ',' << color.r << ',' << color.g << ',' << color.b << ',' << color.a;
			}
			output << '\n';
		}

		return output ? zIOResult::ok() : zIOResult::error("Failed while writing point-cloud CSV file: " + path);
	}
}
