#pragma once
// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <queue>
#include "./../globals.hpp"
#include "./graph_node.hpp"
#include "./grid.hpp"

namespace hoffman::isaiah {
	namespace pathfinding {
		/// <summary>Class that represents a pathfinder used to find
		/// paths for enemies.</summary>
		class Pathfinder {
		public:
			Pathfinder(Grid tgraph, const Grid& fgraph, const Grid& igraph,
				bool allow_diag, HeuristicStrategies h_strat) :
				terrain_graph {tgraph},
				filter_graph {fgraph},
				influence_graph {igraph},
				move_diag {allow_diag},
				heuristic_strategy {h_strat} {
			}
			// Probably the one I'll use more often
			Pathfinder(const game::GameMap& gmap, bool find_air, bool allow_diag, HeuristicStrategies h_strat) :
				Pathfinder::Pathfinder {gmap.getTerrainGraph(find_air), gmap.getFiterGraph(find_air),
					gmap.getInfluenceGraph(find_air), allow_diag, h_strat} {
			}
			~Pathfinder() = default;
			Pathfinder(const Pathfinder& rhs) = default;
			Pathfinder(Pathfinder&& rhs) = default;
			Pathfinder& operator=(const Pathfinder& rhs) = default;
			Pathfinder& operator=(Pathfinder&& rhs) = default;
			/// <summary>Checks if a path actually exists from the start location
			/// to the goal location.</summary>
			/// <returns>True if a path exists from the starting node to the goal
			/// node.</returns>
			bool checkPathExists() const noexcept;
			/// <summary>Attempts to find the shortest path to the goal using the A* method.</summary>
			/// <param name="h_modifier">The h-value of every node is multiplied by this value. Use
			/// this parameter to change the admissibility of the heuristic (and how optimal paths are).</param>
			void findPath(double h_modifier = 1.0, int start_x = -1, int start_y = -1, int goal_x = -1, int goal_y = -1);
			// Setters
			void setStrategy(HeuristicStrategies new_strat, bool diag_status) {
				this->heuristic_strategy = new_strat;
				this->move_diag = diag_status;
			}
			// Getters
			std::queue<GraphNode> getPath() const noexcept {
				return this->my_path;
			}
		private:
			/// <summary>The terrain graph used by the pathfinder.</summary>
			Grid terrain_graph;
			/// <summary>The filter graph used by the pathfinder.</summary>
			Grid filter_graph;
			/// <summary>A graph that marks areas that the enemy would like to avoid.</summary>
			Grid influence_graph;
			/// <summary>Determines whether diagonal movement should be considered.</summary>
			bool move_diag;
			/// <summary>The strategy to use when making heuristic estimates.</summary>
			HeuristicStrategies heuristic_strategy;
			/// <summary>Queue that contains the path last found by the pathfinder.</summary>
			std::queue<GraphNode> my_path;
		};
	}
}