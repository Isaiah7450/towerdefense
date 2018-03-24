#pragma once
// File Author: Isaiah Hoffman
// File Created: March 23, 2018
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
					this->previous_weight = this->weight;
					this->weight = GraphNode::blocked_space_weight;
				}
				else if (!block_space && this->isBlocked()) {
					this->weight = this->previous_weight;
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
			PathFinderNode(GraphNode me_node, PathFinderNode* my_parent_node, int j_cost) noexcept :
				parent_node {my_parent_node},
				graph_node {me_node},
				j {j_cost} {
				if (this->parent_node) {
					this->g = this->parent_node->f;
				}
				this->g += this->getGraphNode().getWeight();
				this->f = this->g + this->h + this->j;
			}
			// Getters
			const GraphNode& getGraphNode() const noexcept {
				return this->graph_node;
			}
			// Setters
			/// <summary>Sets the path costs to get from some starting node to a goal node
			/// while passing through this node.</summary>
			void setPathCosts(int g_cost, int h_cost, int j_cost) noexcept {
				this->g = g_cost;
				this->h = h_cost;
				this->j = j_cost;
				this->f = this->g + this->h + this->j;
			}
		private:
			/// <summary>This node's parent node or nullptr if this node does not have a parent node.</summary>
			PathFinderNode* parent_node;
			/// <summary>The graph node represented by this pathfinding node.</summary>
			GraphNode graph_node;
			/// <summary>The estimated total cost of reaching some destination node from some starting node.
			/// Note that f = g + h + j.</summary>
			int f {0};
			/// <summary>The exact cost of reaching this location from some starting node.</summary>
			int g {0};
			/// <summary>The estimated movement cost of reaching some destination node from this starting node.</summary>
			int h {0};
			/// <summary>This value is used to discourage certain paths that may be dangerous.</summary>
			int j;
		};
	}
}