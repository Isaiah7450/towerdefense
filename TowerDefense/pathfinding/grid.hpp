#pragma once
// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <iosfwd>
#include <vector>
#include <initializer_list>
#include <memory>
#include "./../globals.hpp"
#include "./graph_node.hpp"

namespace hoffman::isaiah {
	namespace pathfinding {
		/// <summary>Class representing a rectangular graph of nodes.</summary>
		class Grid {
		public:
			/// <summary>Creates an empty grid with grid_width columns and grid_height rows.
			/// (Those numbers are global psuedo-constants.)</summary>
			Grid() :
				Grid(graphics::grid_height, graphics::grid_width) {
			}
			/// <summary>Creates an empty grid with the specified width and height.</summary>
			/// <param name="rows">The number of rows in the grid.</param>
			/// <param name="cols">The number of columns in the grid.</param>
			Grid(int rows, int cols) :
				nodes {} {
				// Store values (for drawing purposes)
				graphics::grid_height = rows;
				graphics::grid_width = cols;
				// Reset the grid nodes
				for (int i = 0; i < rows; ++i) {
					std::vector<GraphNode> new_row {};
					for (int j = 0; j < cols; ++j) {
						new_row.emplace_back(j, i, 0);
					}
					this->nodes.emplace_back(new_row);
				}
			}
			/// <summary>Constructor that creates a new grid based on the given data.</summary>
			/// <param name="start_x">The starting node's x-coordinate. (Use -1 for nullptr.)</param>
			/// <param name="start_y">The starting node's y-coordinate. (Use -1 for nullptr.)</param>
			/// <param name="goal_x">The destination node's x-coordinate. (Use -1 for nullptr.)</param>
			/// <param name="goal_y">The destination node's y-coordinate. (Use -1 for nullptr.)</param>
			/// <param name="node_weights">The weights of the nodes to store in the grid.</param>
			Grid(int start_x, int start_y, int goal_x, int goal_y,
				std::initializer_list<std::initializer_list<int>> node_weights) :
				nodes {} {
				int i = 0, j = 0;
				for (auto& my_row_weights : node_weights) {
					std::vector<GraphNode> new_row {};
					j = 0;
					for (auto& my_weight : my_row_weights) {
						new_row.emplace_back(j, i, my_weight);
						++j;
					}
					this->nodes.emplace_back(new_row);
					++i;
				}
				if (start_x >= 0 && start_y >= 0 && goal_x >= 0 && goal_y >= 0) {
					this->start_node = &this->getNode(start_x, start_y);
					this->goal_node = &this->getNode(goal_x, goal_y);
				}
			}
			/// <summary>Constructs the graph from data stored in a file.</summary>
			/// <param name="is">Input file containing the graph data to load.</param>
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
			const GraphNode* getStartNode() const noexcept {
				return this->start_node;
			}
			/// <returns>A pointer to the node marked as the ending node in the graph.</returns>
			const GraphNode* getGoalNode() const noexcept {
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
			/// <param name="gx">The x-coordinate of the node to find the neighbors of.</param>
			/// <param name="gy">The y-coordinate of the node to find the neighbors of.</param>
			/// <param name="filter_graph">Another graph that provides additional information
			/// about nodes whose values are different from the original terrain values.</param>
			/// <param name="include_diag">Set this to true if diagonal movement is allowed.</param>
			/// <returns>A list of constant pointers to nodes that neighbor the specified node.</returns>
			std::vector<const GraphNode*> getNeighbors(int gx, int gy, const Grid& filter_graph, bool include_diag) const;
			/// <returns>True if both coordinates are valid, otherwise, false.</returns>
			/// <param name="gx">The game x-coordinate to verify.</param>
			/// <param name="gy">The game y-coordinate to verify.</param>
			bool verifyCoordinates(double gx, double gy) const noexcept {
				return gx >= 0 && gx < this->getWidth() && gy >= 0 && gy < this->getHeight();
			}
			// Setters
			void setStartNode(int gx, int gy) noexcept {
				this->start_node = &this->getNode(gx, gy);
			}
			void setGoalNode(int gx, int gy) noexcept {
				this->goal_node = &this->getNode(gx, gy);
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

	namespace game {
		/// <summary>Wrapper class that keeps track of the terrain (as well as the
		/// influence graphs and filter graphs).</summary>
		class GameMap : public graphics::IDrawable {
		public:
			/// <param name="ground_terrain_file">File containing the ground terrain data.</param>
			/// <param name="air_terrain_file">File containing the air terrain data.</param>
			GameMap(std::wistream& ground_terrain_file, std::wistream& air_terrain_file) :
				ground_terrain_graph {std::make_unique<pathfinding::Grid>(ground_terrain_file)},
				air_terrain_graph {std::make_unique<pathfinding::Grid>(air_terrain_file)},
				ground_filter_graph {std::make_unique<pathfinding::Grid>()},
				air_filter_graph {std::make_unique<pathfinding::Grid>()},
				ground_influence_graph {std::make_unique<pathfinding::Grid>()},
				air_influence_graph {std::make_unique<pathfinding::Grid>()} {
			}
			/// <param name="gt_graph">The graph containing the ground terrain information.</param>
			/// <param name="at_graph">The graph containing the air terrain information.</param>
			GameMap(pathfinding::Grid gt_graph, pathfinding::Grid at_graph) :
				ground_terrain_graph {std::make_unique<pathfinding::Grid>(gt_graph)},
				air_terrain_graph {std::make_unique<pathfinding::Grid>(at_graph)},
				ground_filter_graph {std::make_unique<pathfinding::Grid>()},
				air_filter_graph {std::make_unique<pathfinding::Grid>()},
				ground_influence_graph {std::make_unique<pathfinding::Grid>()},
				air_influence_graph {std::make_unique<pathfinding::Grid>()} {
			}
			// Overriding graphics::IDrawable
			void draw(const graphics::Renderer2D& renderer) const noexcept override;

			// Getters
			/// <param name="get_air_graph">Set this true to return the air graph; otherwise,
			/// the ground graph is returned.</param>
			/// <returns>A constant reference to the requested terrain graph.</returns>
			const pathfinding::Grid& getTerrainGraph(bool get_air_graph) const noexcept {
				return get_air_graph ? *this->air_terrain_graph : *this->ground_terrain_graph;
			}
			/// <param name="get_air_graph">Set this true to return the air graph; otherwise,
			/// the ground graph is returned.</param>
			/// <returns>A reference to the requested terrain graph.</returns>
			pathfinding::Grid& getTerrainGraph(bool get_air_graph) noexcept {
				return get_air_graph ? *this->air_terrain_graph : *this->ground_terrain_graph;
			}
			/// <param name="get_air_graph">Set this true to return the air graph; otherwise,
			/// the ground graph is returned.</param>
			/// <returns>A constant reference to the requested filter graph.</returns>
			const pathfinding::Grid& getFiterGraph(bool get_air_graph) const noexcept {
				return get_air_graph ? *this->air_filter_graph : *this->ground_filter_graph;
			}
			/// <param name="get_air_graph">Set this true to return the air graph; otherwise,
			/// the ground graph is returned.</param>
			/// <returns>A reference to the requested filter graph.</returns>
			pathfinding::Grid& getFiterGraph(bool get_air_graph) noexcept {
				return get_air_graph ? *this->air_filter_graph : *this->ground_filter_graph;
			}
			/// <param name="get_air_graph">Set this true to return the air graph; otherwise,
			/// the ground graph is returned.</param>
			/// <returns>A constant reference to the requested influence graph.</returns>
			const pathfinding::Grid& getInfluenceGraph(bool get_air_graph) const noexcept {
				return get_air_graph ? *this->air_influence_graph : *this->ground_influence_graph;
			}
			/// <param name="get_air_graph">Set this true to return the air graph; otherwise,
			/// the ground graph is returned.</param>
			/// <returns>A reference to the requested influence graph.</returns>
			pathfinding::Grid& getInfluenceGraph(bool get_air_graph) noexcept {
				return get_air_graph ? *this->air_influence_graph : *this->ground_influence_graph;
			}

		private:
			/// <summary>Graph that contains information about the basic terrain weights
			/// of each grid square for ground units.</summary>
			std::unique_ptr<pathfinding::Grid> ground_terrain_graph;
			/// <summary>Graph that contains information about the basic terrain weights
			/// of each grid square for air units.</summary>
			std::unique_ptr<pathfinding::Grid> air_terrain_graph;
			/// <summary>Graph that contains adjustments to the information found
			/// in the ground terrain graph. This includes stuff like towers and traps.</summary>
			std::unique_ptr<pathfinding::Grid> ground_filter_graph;
			/// <summary>Graph that contains adjustments to the information found
			/// in the air terrain graph. This includes stuff like towers and traps.</summary>
			std::unique_ptr<pathfinding::Grid> air_filter_graph;
			/// <summary>Graph that contains information about dangerous areas
			/// for ground units.</summary>
			std::unique_ptr<pathfinding::Grid> ground_influence_graph;
			/// <summary>Graph that contains information about dangerous areas
			/// for air units.</summary>
			std::unique_ptr<pathfinding::Grid> air_influence_graph;
		};
	}
}