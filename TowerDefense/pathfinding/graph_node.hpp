#pragma once
// File Author: Isaiah Hoffman
// File Created: March 23, 2018
#include <memory>
#include <cmath>
#include "./../globals.hpp"
#include "./../ih_math.hpp"

namespace hoffman::isaiah {
	namespace pathfinding {
		/// <summary>Class that represents a graph node. (Primarily for grid-based graphs,
		/// but it may be possible to use it for other types of graphs.)</summary>
		class GraphNode {
		public:
			/// <param name="gx">The game x-coordinate represented by the node.</param>
			/// <param name="gy">The game y-coordinate represented by the node.</param>
			/// <param name="w">The weight of the node. A value of 100+ indicates a blocked node.</param>
			GraphNode(int gx, int gy, int w) :
				x {gx},
				y {gy},
				weight {w} {
			}
			// Getters
			/// <returns>The node's game x-coordinate.</returns>
			int getGameX() const noexcept {
				return this->x;
			}
			/// <returns>The node's game y-coordinate.</returns>
			int getGameY() const noexcept {
				return this->y;
			}
			/// <returns>The node's current weight.</returns>
			int getWeight() const noexcept {
				return this->weight;
			}
			/// <returns>True if the node is impassible; otherwise, false.</returns>
			bool isBlocked() const noexcept {
				return this->weight >= GraphNode::blocked_space_weight;
			}
			// Setters
			/// <param name="block_space">Set this to true if the node should be blocked.</param>
			void setBlockage(bool block_space) noexcept {
				if (block_space && !this->isBlocked()) {
					if (this->getWeight() < GraphNode::blocked_space_weight) {
						this->previous_weight = this->getWeight();
					}
					else {
						this->previous_weight = 1;
					}
					this->weight = GraphNode::blocked_space_weight;
				}
				else if (!block_space && this->isBlocked()) {
					if (this->previous_weight < GraphNode::blocked_space_weight) {
						this->weight = this->previous_weight;
					}
					else {
						this->weight = 1;
					}
					this->previous_weight = GraphNode::blocked_space_weight;
				}
				// Otherwise, nothing happens
			}
			/// <param name="new_weight">The node's new weight.</param>
			void setWeight(int new_weight) noexcept {
				this->previous_weight = this->getWeight();
				this->weight = new_weight;
			}

			// This number indicates the weight of a blocked space.
			constexpr const static int blocked_space_weight {100};
		private:
			/// <summary>The game x-coordinate of the node on a grid.
			/// Coordinates should be zero-based.</summary>
			int x;
			/// <summary>The game y-coordinate of the node on a grid.
			/// Coordinates should be zero-based.</summary>
			int y;
			/// <summary>The weight associated with this node in the grid.
			/// A weight greater than or equal to 100 indicates that the node is blocked.</summary>
			int weight;
			/// <summary>The previous weight associated with this node in the grid.
			/// (This value exists primarily for the convenience of things like terrain editors.)</summary>
			int previous_weight {0};
		};

		/// <summary>Class that represents a node used by a pathfinder to find optimal paths.</summary>
		class PathFinderNode {
		public:
			PathFinderNode(const GraphNode& me_node, std::shared_ptr<PathFinderNode> my_parent_node,
				const GraphNode& my_goal_node, HeuristicStrategies h_strat, double j_cost,
				double j_multiplier, double h_modifier = 1.0) noexcept :
				parent_node {my_parent_node},
				graph_node {std::make_shared<GraphNode>(me_node)},
				j {j_cost} {
				double multiplier = 1.0;
				if (this->parent_node) {
					this->g = this->parent_node->g;
					int dx = math::get_abs(this->getGameX() - this->parent_node->getGameX());
					int dy = math::get_abs(this->getGameY() - this->parent_node->getGameY());
					if (dx != 0 && dy != 0) {
						// Diagonal movement
						multiplier = std::sqrt(2);
					}
				}
				// Add movement cost
				this->g += (this->getGraphNode()->getWeight()) * multiplier;
				this->calculateHeuristic(&my_goal_node, h_strat, h_modifier);
				this->f = this->g + this->h + this->j * j_multiplier;
			}
			// Getters
			const std::shared_ptr<PathFinderNode> getParentNode() const noexcept {
				return this->parent_node;
			}
			const std::shared_ptr<GraphNode> getGraphNode() const noexcept {
				return this->graph_node;
			}
			int getGameX() const noexcept {
				return this->getGraphNode()->getGameX();
			}
			int getGameY() const noexcept {
				return this->getGraphNode()->getGameY();
			}
			double getF() const noexcept {
				return this->f;
			}
		protected:
			/// <summary>Calculates the h score of this node using the provided strategy.</summary>
			/// <param name="my_goal_node">The destination one is trying to reach.</param>
			/// <param name="strat">The strategy being used to estimate the distance to the goal.</param>
			/// <param name="h_modifier">A value that can be adjusted to increase/decrease the heuristic.
			/// Higher heuristic values result in faster processing but less optimal paths.</param>
			void calculateHeuristic(const GraphNode* my_goal_node, HeuristicStrategies strat, double h_modifier = 1.0) {
				const int dx = math::get_abs(this->getGameX() - my_goal_node->getGameX());
				const int dy = math::get_abs(this->getGameY() - my_goal_node->getGameY());
				switch (strat) {
				case HeuristicStrategies::Euclidean:
					// Basically the distance formula for the shortest distance
					// between any two points (d = sqrt((x2 - x1)^2 + (y2 - y1)^2))
					// The only problem is that one cannot move in all directions in this game
					// so using Euclidean distances actually wastes time!
					this->h = std::sqrt(dx * dx + dy * dy) * h_modifier;
					break;
				case HeuristicStrategies::Diagonal:
					// Diagonal shortcut which is generally used
					// when one can move in 8 directions.
					this->h = ((dx + dy) + (std::sqrt(2) - 2) * math::get_min(dx, dy)) * h_modifier;
					break;
				case HeuristicStrategies::Max_Dx_Dy:
					// A variant on diagonal shortcut where the cost of moving diagonally is
					// the same as the cost of moving horizontally/vertically.
					this->h = math::get_max(dx, dy) * h_modifier;
					break;
				case HeuristicStrategies::Manhattan:
				default:
					// Default behavior is to use Manhattan which is
					// generally used when one can travel only in the 4 cardinal directions.
					this->h = (dx + dy) * h_modifier;
					break;
				}
			}
		protected:
		private:
			/// <summary>This node's parent node or nullptr if this node does not have a parent node.</summary>
			std::shared_ptr<PathFinderNode> parent_node;
			/// <summary>The graph node represented by this pathfinding node.</summary>
			std::shared_ptr<GraphNode> graph_node;
			/// <summary>The estimated total cost of reaching some destination node from some starting node.
			/// Note that f = g + h + j.</summary>
			double f {0};
			/// <summary>The exact cost of reaching this location from some starting node.</summary>
			double g {0};
			/// <summary>The estimated movement cost of reaching some destination node from this starting node.</summary>
			double h {0};
			/// <summary>This value is used to discourage certain paths that may be dangerous.</summary>
			double j;
		};
	}
}