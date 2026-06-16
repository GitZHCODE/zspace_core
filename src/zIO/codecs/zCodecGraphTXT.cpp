#include <src/zIO/codecs/zCodecGraphTXT.h>

#include <fstream>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

namespace zSpace::io_detail
{
	zIOResult readGraphTXT(const std::string& path, GraphData& data)
	{
		std::ifstream input(path);
		if (!input) return zIOResult::error("Could not open graph TXT file: " + path);

		data = {};
		std::vector<std::pair<int, int>> edges;
		int minimumIndex = std::numeric_limits<int>::max();
		std::string line;
		int lineNumber = 0;

		while (std::getline(input, line))
		{
			++lineNumber;
			std::istringstream stream(line);
			std::string tag;
			stream >> tag;

			if (tag.empty() || tag[0] == '#') continue;

			if (tag == "v")
			{
				zPoint position;
				if (!(stream >> position.x >> position.y >> position.z))
					return zIOResult::error("Invalid graph vertex at line " + std::to_string(lineNumber));
				data.positions.push_back(position);
			}
			else if (tag == "e")
			{
				int first = -1;
				int second = -1;
				if (!(stream >> first >> second))
					return zIOResult::error("Invalid graph edge at line " + std::to_string(lineNumber));
				minimumIndex = std::min(minimumIndex, std::min(first, second));
				edges.emplace_back(first, second);
			}
		}

		const bool oneBased = minimumIndex > 0;
		for (const auto& edge : edges)
		{
			const int first = edge.first - (oneBased ? 1 : 0);
			const int second = edge.second - (oneBased ? 1 : 0);
			if (first < 0 || second < 0 ||
				first >= static_cast<int>(data.positions.size()) ||
				second >= static_cast<int>(data.positions.size()))
				return zIOResult::error("Graph edge references an invalid vertex.");

			data.edgeConnects.push_back(first);
			data.edgeConnects.push_back(second);
		}

		if (data.positions.empty()) return zIOResult::error("Graph TXT file contains no vertices: " + path);
		return zIOResult::ok();
	}

	zIOResult writeGraphTXT(const std::string& path, const GraphData& data)
	{
		std::ofstream output(path);
		if (!output) return zIOResult::error("Could not create graph TXT file: " + path);

		for (const auto& position : data.positions)
			output << "v " << position.x << ' ' << position.y << ' ' << position.z << '\n';

		if (data.edgeConnects.size() % 2 != 0)
			return zIOResult::error("Graph contains incomplete edge connectivity.");

		for (std::size_t i = 0; i < data.edgeConnects.size(); i += 2)
			output << "e " << data.edgeConnects[i] + 1 << ' ' << data.edgeConnects[i + 1] + 1 << '\n';

		return output ? zIOResult::ok() : zIOResult::error("Failed while writing graph TXT file: " + path);
	}
}
