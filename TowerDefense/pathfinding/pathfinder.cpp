// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <queue>
#include <set>
#include <vector>
#include <map>
#include <utility>
#include "./../globals.hpp"
#include "./graph_node.hpp"
#include "./grid.hpp"
#include "./pathfinder.hpp"

namespace hoffman::isaiah {
	namespace pathfinding {
		bool Pathfinder::checkPathExists() const noexcept {
			// This is an algorithm called Breadth_First_Search.
			// It is very efficient for finding if a path exists (though
			// it doesn't work if you are looking for an optimal path.)
			std::queue<const GraphNode*> frontier {};
			frontier.push(this->start_node);
			std::set<const GraphNode*> visited {};
			visited.insert(this->start_node);
			while (!frontier.empty()) {
				auto current = frontier.front();
				// We know a path exists because we started with the starting node
				// and have ended up at the goal node.
				if (current->getGameX() == this->goal_node->getGameX()
					&& current->getGameY() == this->goal_node->getGameY()) {
					return true;
				}
				frontier.pop();
				auto my_neighbors = this->terrain_graph.getNeighbors(current->getGameX(), current->getGameY(),
					this->filter_graph, this->move_diag);
				for (const auto*& next : my_neighbors) {
					if (visited.find(next) == visited.end()) {
						frontier.push(next);
						visited.insert(next);
					}
				}
			}
			return false;
		}

		std::queue<const GraphNode*> Pathfinder::findPath(double h_modifier, int start_x,
			int start_y, int goal_x, int goal_y) {
			std::queue<const GraphNode*> ret_value {};
			class PathFinderComparator {
			public:
				bool operator()(std::shared_ptr<PathFinderNode> a, std::shared_ptr<PathFinderNode> b) const {
					return a->getF() > b->getF();
				}
			};
			const auto determine_node_index = [](int x, int y) {
				return y * graphics::grid_width + x;
			};
			// Some path edits to make before beginning...
			if (start_x != -1 && start_y != -1) {
				this->start_node = &this->terrain_graph.getNode(start_x, start_y);
			}
			if (goal_x != -1 && goal_y != -1) {
				this->goal_node = &this->terrain_graph.getNode(goal_x, goal_y);
			}
			std::priority_queue<std::shared_ptr<PathFinderNode>,
				std::vector<std::shared_ptr<PathFinderNode>>, PathFinderComparator> my_set {};
			std::map<int, double> previous_costs {};
			// Yes, I am actually going to do the search in reverse order so
			// that the beginning of the queue that will be returned will be the start node.
			auto my_goal = std::make_shared<PathFinderNode>(this->goal_node, nullptr, this->start_node,
				this->heuristic_strategy, 0, h_modifier);
			my_set.push(my_goal);
			previous_costs.emplace(determine_node_index(my_goal->getGameX(), my_goal->getGameY()), my_goal->getF());
			while (!my_set.empty()) {
				auto current_node = my_set.top();
				// If this is the start node, we have found the path
				if (&current_node->getGraphNode() == this->start_node) {
					break;
				}
				else if (current_node->getGameX() == this->start_node->getGameX() &&
					current_node->getGameY() == this->start_node->getGameY()) {
					break;
				}
				my_set.pop();
				auto my_neighbors = this->terrain_graph.getNeighbors(current_node->getGameX(), current_node->getGameY(),
					this->filter_graph, this->move_diag);
				// Look at neighbors
				for (auto& neighbor_node : my_neighbors) {
					auto next_node = PathFinderNode {neighbor_node, current_node, this->start_node, this->heuristic_strategy,
						static_cast<double>(influence_graph.getNode(neighbor_node->getGameX(),
							neighbor_node->getGameY()).getWeight()), h_modifier};
					auto next_node_index = determine_node_index(next_node.getGameX(), next_node.getGameY());
					// Check F score and only add if F score is lower
					if (previous_costs.find(next_node_index) == previous_costs.end()) {
						previous_costs.emplace(next_node_index, next_node.getF());
						my_set.push(std::make_shared<PathFinderNode>(next_node));
					}
					else if (previous_costs.at(next_node_index) > next_node.getF()) {
						previous_costs.erase(next_node_index);
						previous_costs.emplace(next_node_index, next_node.getF());
						my_set.push(std::make_shared<PathFinderNode>(next_node));
					}
				}
			}
			if (my_set.empty()) {
				throw std::runtime_error {"Queue is empty; check that a path exists."};
			}
			// Construct path by following the parent nodes
			auto path_node = my_set.top();
			ret_value.push(&path_node->getGraphNode());
			std::shared_ptr<PathFinderNode> path_parent = nullptr;
			while ((path_parent = path_node->getParentNode()) != nullptr) {
				ret_value.push(&path_node->getGraphNode());
				path_node = path_parent;
			}
			ret_value.push(&path_node->getGraphNode());
			// Clear the priority queue
			while (!my_set.empty()) {
				my_set.pop();
			}
			// Return the found path
			return ret_value;
		}
	}
}