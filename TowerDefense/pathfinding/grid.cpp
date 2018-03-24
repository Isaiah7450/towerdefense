// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <iostream>
#include <string>
#include <stdexcept>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./graph_node.hpp"
#include "./grid.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace pathfinding {
		std::wistream& operator>>(std::wistream& is, Grid& graph) {
			// First, read in the width and height of the grid
			// and set the global variables appropriately
			is >> graphics::grid_width >> graphics::grid_height;
			// Next, read the contents of the grid
			for (int i = 0; i < graphics::grid_height; ++i) {
				std::vector<GraphNode> new_row {};
				for (int j = 0; j < graphics::grid_width; ++j) {
					int w = 0;
					is >> w;
					new_row.emplace_back(j, i, w);
				}
			}
			// Check that we read the required number of nodes
			if (graph.getWidth() != graphics::grid_width ||
				graph.getHeight() != graphics::grid_height ||
				is.fail()) {
				throw std::runtime_error {"Error reading graph: not enough nodes were read."};
			}
			// Finally, read the start and end nodes
			int sx = 0;
			int sy = 0;
			int gx = 0;
			int gy = 0;
			is >> sx >> sy >> gx >> gy;
			graph.start_node = &graph.getNode(sx, sy);
			graph.goal_node = &graph.getNode(gx, gy);
			return is;
		}

		std::wostream& operator<<(std::wostream& os, const Grid& graph) {
			if (!graph.getStartNode() || !graph.getGoalNode()) {
				throw std::runtime_error {"Cannot output a graph that lacks a starting and ending location."};
			}
			// Width and height
			os << graph.getWidth() << L" " << graph.getHeight() << L"\n";
			// Output grid contents
			for (int i = 0; i < graph.getRows(); ++i) {
				for (int j = 0; j < graph.getColumns(); ++j) {
					os << graph.getNode(j, i).getWeight() << ((j < graph.getColumns() - 1)
						? L" " : L"");
				}
				os << L"\n";
			}
			// Finally, output the start and end nodes
			return os << graph.getStartNode()->getGameX() << L" "
				<< graph.getStartNode()->getGameY() << L"\n"
				<< graph.getGoalNode()->getGameX() << L" "
				<< graph.getGoalNode()->getGameY() << L"\n";
		}
	}
}