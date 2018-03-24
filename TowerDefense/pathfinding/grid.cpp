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
			// It is important to clear any nodes that may already be
			// in the graph!
			graph.nodes.clear();
			graph.start_node = nullptr;
			graph.goal_node = nullptr;
			// First, read in the number of rows and columns of the grid
			// and set the global variables appropriately
			is >> graphics::grid_height >> graphics::grid_width;
			// Next, read the contents of the grid
			for (int i = 0; i < graphics::grid_height; ++i) {
				std::vector<GraphNode> new_row {};
				for (int j = 0; j < graphics::grid_width; ++j) {
					int w = 0;
					is >> w;
					new_row.emplace_back(j, i, w);
				}
				graph.nodes.emplace_back(new_row);
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
			os << graph.getRows() << L" " << graph.getColumns() << L"\n";
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

		std::vector<const GraphNode*> Grid::getNeighbors(int gx, int gy, const Grid& filter_graph, bool include_diag) const {
#if defined(DEBUG) || defined(_DEBUG)
			// Worthwhile check but may be a bit costly considering
			// how much this function will be called
			if (this->getWidth() != filter_graph.getWidth()
				|| this->getHeight() != filter_graph.getHeight()) {
				throw std::invalid_argument {"Both the terrain map and the filter graph "
					"should have the same dimensions."};
			}
#endif // DEBUG or _DEBUG
			std::vector<const GraphNode*> my_neighbors {};
			// Loop bounds
			const int min_dx = gx > 0 ? -1 : 0;
			const int max_dx = (gx < this->getWidth() - 1) ? 1 : 0;
			const int min_dy = gy > 0 ? -1 : 0;
			const int max_dy = (gy < this->getHeight() - 1) ? 1 : 0;
			// Find neighbors --> Maybe algorithm provides a way to do this cleaner?
			for (int dx = min_dx; dx <= max_dx; ++dx) {
				for (int dy = min_dy; dy <= max_dy; ++dy) {
					// So a very simple (as well as flexible) way of resolving the terrain graph
					// (this graph) and the filter graph is to simply add the filter graph's
					// node's weight to the terrain graph's node's weight.
					if ((include_diag || dx == 0 || dy == 0) && !(dx == dy && dx == 0)) {
						const auto* terrain_node = &this->getNode(gx + dx, gy + dy);
						const auto* filter_node = &filter_graph.getNode(gx + dx, gy + dy);
						auto total_weight = terrain_node->getWeight() + filter_node->getWeight();
						if (total_weight < GraphNode::blocked_space_weight
							|| terrain_node == this->getStartNode()
							|| terrain_node == this->getGoalNode()) {
							my_neighbors.push_back(terrain_node);
						}
					}
				} // End inner for
			} // End outer for
			// Return set of neighbors
			return my_neighbors;
		}
	}
}