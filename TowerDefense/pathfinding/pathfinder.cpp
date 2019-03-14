// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <queue>
#include <set>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
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
			const auto* start_node = this->terrain_graph.getStartNode();
			const auto* goal_node = this->terrain_graph.getGoalNode();
			frontier.push(start_node);
			std::set<const GraphNode*> visited {};
			visited.insert(start_node);
			while (!frontier.empty()) {
				auto current = frontier.front();
				// We know a path exists because we started with the starting node
				// and have ended up at the goal node.
				if (current->getGameX() == goal_node->getGameX()
					&& current->getGameY() == goal_node->getGameY()) {
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

		std::queue<GraphNode> Pathfinder::findPath(double j_multiplier, int start_x,
			int start_y, int goal_x, int goal_y, double h_modifier) {
			class PathFinderComparator {
			public:
				bool operator()(PathFinderNode a, PathFinderNode b) const {
					return a.getF() > b.getF();
				}
			};
			const auto determine_node_index = [](int x, int y) {
				return y * graphics::grid_width + x;
			};
			// Get starts and ends of paths
			auto& start_node = (start_x > -1 && start_y > -1)
				? this->terrain_graph.getNode(start_x, start_y)
				: *this->terrain_graph.getStartNode();
			const auto oldStartWeight = start_node.getWeight();
			[[maybe_unused]] auto oldFilterStartWeight = 0;
			if (start_x > -1 && start_y > -1) {
				oldFilterStartWeight = this->filter_graph.getNode(start_x, start_y).getWeight();
				this->filter_graph.getNode(start_x, start_y).setBlockage(false);
			}
			const auto& goal_node = (goal_x > -1 && goal_y > -1)
				? this->terrain_graph.getNode(goal_x, goal_y)
				: *this->terrain_graph.getGoalNode();
			std::priority_queue<PathFinderNode, std::vector<PathFinderNode>, PathFinderComparator> my_set {};
			std::vector<std::pair<int, double>> previous_costs {};
			// Yes, I am actually going to do the search in reverse order so
			// that the beginning of the queue has the start node.
			auto my_goal = PathFinderNode {goal_node, nullptr, start_node,
				this->heuristic_strategy, 0, j_multiplier, h_modifier};
			my_set.push(my_goal);
			previous_costs.emplace_back(determine_node_index(my_goal.getGameX(), my_goal.getGameY()), my_goal.getF());
			while (!my_set.empty()) {
				auto current_node = std::make_shared<PathFinderNode>(my_set.top());
				if (current_node->getGameX() == start_node.getGameX() &&
					current_node->getGameY() == start_node.getGameY()) {
					break;
				}
				my_set.pop();
				auto my_neighbors = this->terrain_graph.getNeighbors(current_node->getGameX(), current_node->getGameY(),
					this->filter_graph, this->move_diag);
				// Look at neighbors
				for (auto& neighbor_node : my_neighbors) {
					auto next_node = PathFinderNode {*neighbor_node, current_node,
						start_node, this->heuristic_strategy,
						static_cast<double>(influence_graph.getNode(neighbor_node->getGameX(),
							neighbor_node->getGameY()).getWeight()), j_multiplier, h_modifier};
					const auto next_node_index = determine_node_index(next_node.getGameX(), next_node.getGameY());
					// Check F score and only add if F score is lower
					struct MyCompare {
						bool operator()(const std::pair<int, double>& a, const int& b) {
							return a.first < b;
						}
						bool operator()(const int& a, const std::pair<int, double>& b) {
							return a < b.first;
						}
					};
					// Binary Search
					size_t low_bound = 0;
					size_t high_bound = previous_costs.size();
					size_t my_index = high_bound / 2;
					bool found = false;
					while (true) {
						const auto node_index = previous_costs.at(my_index).first;
						if (node_index == next_node_index) {
							// Found it!
							if (previous_costs.at(my_index).second > next_node.getF()) {
								previous_costs.at(my_index).second = next_node.getF();
								my_set.push(next_node);
							}
							found = true;
							break;
						}
						else if (low_bound >= high_bound) {
							// Does not exist!
							break;
						}
						else if (node_index > next_node_index) {
							// Too high!
							high_bound = my_index > 0 ? my_index - 1 : 0;
							my_index = (my_index + low_bound) / 2;
						}
						else {
							// Too low!
							low_bound = my_index + 1;
							my_index = (my_index + high_bound) / 2;
						}
					}
					if (!found) {
						const auto insert_loc = std::find_if(previous_costs.begin(), previous_costs.end(), [next_node_index](const auto& elem) {
							return elem.first > next_node_index;
						});
						previous_costs.insert(insert_loc, std::make_pair<>(next_node_index, next_node.getF()));
						my_set.push(next_node);
					}
				}
			}
			if (my_set.empty()) {
				throw std::runtime_error {"Queue is empty; check that a path exists."};
			}
			// Clear old path
			while (!this->my_path.empty()) {
				this->my_path.pop();
			}
			// Construct path by following the parent nodes
			const auto* path_node = &my_set.top();
			// We really don't need the starting node because we know
			// where we are starting. (It seems it gets in the queue twice
			// for some reason.)
			PathFinderNode* path_parent = nullptr;
			while ((path_parent = path_node->getParentNode().get()) != nullptr) {
				if (path_parent->getGameX() != path_node->getGameX()
					|| path_parent->getGameY() != path_node->getGameY()) {
					this->my_path.emplace(*path_node->getGraphNode());
				}
				path_node = path_parent;
			}
			this->my_path.emplace(*path_node->getGraphNode());
			// Clear the priority queue
			while (!my_set.empty()) {
				my_set.pop();
			}
			// Reset the start and end nodes
			if (start_x != -1 && start_y != -1) {
				start_node.setWeight(oldStartWeight);
				this->filter_graph.getNode(start_x, start_y).setWeight(oldFilterStartWeight);
			}
			return this->my_path;
		}
	}
}