// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include "./../globals.hpp"
#include "./graph_node.hpp"
#include "./grid.hpp"
#include "./pathfinder.hpp"

namespace hoffman_isaiah {
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
			const auto determine_node_index = [this](int x, int y) {
				return y * this->terrain_graph.getWidth() + x;
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
			const auto max_index = static_cast<size_t>(determine_node_index(this->terrain_graph.getColumns(),
				this->terrain_graph.getRows()));
			const auto max_fscore = 8.0 * static_cast<double>(max_index);
			std::vector<double> previous_costs(max_index, max_fscore);
			// Yes, I am actually going to do the search in reverse order so
			// that the beginning of the queue has the start node.
			auto my_goal = PathFinderNode {goal_node, nullptr, start_node,
				this->heuristic_strategy, 0, j_multiplier, h_modifier};
			my_set.push(my_goal);
			previous_costs.at(determine_node_index(my_goal.getGameX(), my_goal.getGameY())) = my_goal.getF();
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
					if (previous_costs.at(next_node_index) > next_node.getF()) {
						previous_costs.at(next_node_index) = next_node.getF();
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