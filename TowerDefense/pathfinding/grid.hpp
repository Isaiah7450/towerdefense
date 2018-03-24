#pragma once
// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <iosfwd>
#include <vector>
#include "./../globals.hpp"
#include "./graph_node.hpp"

namespace hoffman::isaiah {
	namespace pathfinding {
		/// <summary>Class representing a rectangular graph of nodes.</summary>
		class Grid {
		public:
			/// <summary>Creates an empty grid with a height and width
			/// specified by the global psuedo-constants grid_width and
			/// grid_height in the graphics namespace.</summary>
			Grid() :
				nodes {} {
				// Reset the grid nodes
				for (int i = 0; i < graphics::grid_height; ++i) {
					std::vector<GraphNode> new_row {};
					for (int j = 0; j < graphics::grid_width; ++j) {
						new_row.emplace_back(j, i, 1);
					}
					this->nodes.emplace_back(new_row);
				}
			}
			/// <summary>Constructor that also initializes the start
			/// and end nodes in addition to initializing the grid.</summary>
			/// <param name="start_x">The starting node's x-coordinate.</param>
			/// <param name="start_y">The starting node's y-coordinate.</param>
			/// <param name="goal_x">The destination node's x-coordinate.</param>
			/// <param name="goal_y">The destination node's y-coordinate.</param>
			Grid(int start_x, int start_y, int goal_x, int goal_y) :
				Grid::Grid() {
				this->start_node = &this->getNode(start_x, start_y);
				this->goal_node = &this->getNode(goal_x, goal_y);
			}
			/// <summary>Constructs the graph from data stored in a file.</summary>
			Grid(std::wistream& is) :
				nodes {} {
				// Defer to input operator
				is >> *this;
			}

			// Getters
			/// <param name="x">The x-coordinate (column number) of the node in the grid.</param>
			/// <param name="y">The y-coordinate (row number) of the node in the grid.</param>
			/// <returns>A reference to the node located at (y,x) in the grid.</returns>
			GraphNode& getNode(int x, int y) {
				return this->nodes.at(y).at(x);
			}
			/// <param name="x">The x-coordinate (column number) of the node in the grid.</param>
			/// <param name="y">The y-coordinate (row number) of the node in the grid.</param>
			/// <returns>A constant reference to the node located at (y,x) in the grid.</returns>
			const GraphNode& getNode(int x, int y) const {
				return this->nodes.at(y).at(x);
			}
			/// <returns>A pointer to the node marked as the starting node in the graph.</returns>
			const GraphNode* getStartNode() const {
				return this->start_node;
			}
			/// <returns>A pointer to the node marked as the ending node in the graph.</returns>
			const GraphNode* getGoalNode() const {
				return this->goal_node;
			}
			/// <returns>The width of the grid.</returns>
			int getWidth() const noexcept {
				return static_cast<int>(this->nodes.at(0).size());
			}
			/// <returns>The height of the grid.</returns>
			int getHeight() const noexcept {
				return static_cast<int>(this->nodes.size());
			}
			/// <returns>The number of rows in the grid. (This is equivalent to querying
			/// the height of the grid; however, this may be useful in some casees.)</returns>
			int getRows() const noexcept {
				return this->getHeight();
			}
			/// <returns>The number of columns in the grid. (This is equivalent to querying
			/// the width of the grid; however, this may be useful in some cases.)</returns>
			int getColumns() const noexcept {
				return this->getWidth();
			}
			// Input/Output
			friend std::wostream& operator<<(std::wostream& os, const Grid& graph);
			friend std::wistream& operator>>(std::wistream& is, Grid& graph);
		private:
			/// <summary>2-dimensional array that stores all the nodes in this grid.
			/// Nodes are stored in the order of row then column.</summary>
			std::vector<std::vector<GraphNode>> nodes;
			/// <summary>Pointer to the starting node (for pathfinding and enemy
			/// generation) if relevant. If this is irrelevant, then this value
			/// should be nullptr.</summary>
			GraphNode* start_node {nullptr};
			/// <summary>Pointer to the ending node (for pathfinding) if relevant.
			/// If this is irrelevant, then this value should be nullptr.</summary>
			GraphNode* goal_node {nullptr};
		};
	}
}