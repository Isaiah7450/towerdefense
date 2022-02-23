// File Author: Isaiah Hoffman
// File Created: March 24, 2018
#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../terrain/editor.hpp"
#include "./graph_node.hpp"
#include "./grid.hpp"
using namespace std::literals::string_literals;

namespace hoffman_isaiah {
	namespace pathfinding {
		std::wistream& operator>>(std::wistream& is, Grid& graph) {
			// It is important to clear any nodes that may already be
			// in the graph!
			graph.nodes.clear();
			graph.start_node = nullptr;
			graph.goal_node = nullptr;
			// First, read in the number of rows and columns of the grid
			int grid_rows = 0, grid_cols = 0;
			is >> grid_rows >> grid_cols;
			// Next, read the contents of the grid
			for (int i = 0; i < grid_rows; ++i) {
				std::vector<GraphNode> new_row {};
				for (int j = 0; j < grid_cols; ++j) {
					int w = 0;
					is >> w;
					new_row.emplace_back(j, i, w);
				}
				graph.nodes.emplace_back(new_row);
			}
			// Check that we read the required number of nodes
			if (graph.getRows() != grid_rows ||
				graph.getColumns() != grid_cols ||
				is.fail()) {
				throw std::runtime_error {"Error reading graph: not enough nodes were read "
					"or input file does not exist."};
			}
			// Finally, read the start and end nodes
			int sx = 0;
			int sy = 0;
			int gx = 0;
			int gy = 0;
			is >> sx >> sy >> gx >> gy;
			if (sx > -1 && sy > -1) {
				graph.start_node = &graph.getNode(sx, sy);
				graph.goal_node = &graph.getNode(gx, gy);
			}
			return is;
		}

		std::wostream& operator<<(std::wostream& os, const Grid& graph) {
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
			if (!graph.getStartNode()) {
				return os << L"-1 -1\n-1 -1\n";
			}
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
						const auto total_weight = terrain_node->getWeight() + filter_node->getWeight();
						if (total_weight < GraphNode::blocked_space_weight
							|| terrain_node == this->getStartNode()
							|| terrain_node == this->getGoalNode()) {
							my_neighbors.emplace_back(terrain_node);
						}
					}
				} // End inner for
			} // End outer for
			// Return set of neighbors
			return my_neighbors;
		}
	}

	namespace game {
		void GameMap::draw(const graphics::Renderer2D& renderer) const noexcept {
			this->drawTerrain(renderer);
			this->drawStartGoal(renderer);
			this->drawMarkedTiles(renderer);
		}

		void GameMap::draw(const graphics::Renderer2D& renderer, bool in_editor) const noexcept {
			this->draw(renderer);
			if (in_editor) {
				constexpr const graphics::Color my_color = graphics::Color {1.f, 1.f, 1.f, 0.75f};
				const auto ground_active = terrain_editor::g_my_editor->areGroundWeightsActive();
				const auto air_active = terrain_editor::g_my_editor->areAirWeightsActive();
				if (!ground_active && !air_active) {
					return;
				}
				const auto node_width = this->getGameSquareWidth<float>();
				const auto node_height = this->getGameSquareHeight<float>();
				for (int gx = 0; gx < this->getWidth(); ++gx) {
					for (int gy = 0; gy < this->getHeight(); ++gy) {
						const auto& gnode = this->getTerrainGraph(false).getNode(gx, gy);
						const auto& anode = this->getTerrainGraph(true).getNode(gx, gy);
						if (gnode.isBlocked() || anode.isBlocked()) {
							continue;
						}
						const auto node_lsx = static_cast<float>(this->convertToScreenX(gnode.getGameX()));
						const auto node_tsy = static_cast<float>(this->convertToScreenY(gnode.getGameY()));
						if (ground_active && air_active) {
							const auto weight_string = std::to_wstring(gnode.getWeight()) + L"|"
								+ std::to_wstring(anode.getWeight());
							renderer.drawText(weight_string, my_color, renderer.createRectangle(
								node_lsx, node_tsy, node_width, node_height));
						}
						else if (ground_active) {
							const auto weight_string = std::to_wstring(gnode.getWeight());
							renderer.drawText(weight_string, my_color, renderer.createRectangle(
								node_lsx, node_tsy, node_width, node_height));
						}
						else {
							const auto weight_string = std::to_wstring(anode.getWeight());
							renderer.drawText(weight_string, my_color, renderer.createRectangle(
								node_lsx, node_tsy, node_width, node_height));
						}
					} // End inner for.
				} // End outer for.
			}
		}

		void GameMap::drawTerrain(const graphics::Renderer2D& renderer) const noexcept {
			constexpr const graphics::Color outline_color = graphics::Color {0.10f, 0.10f, 0.10f, 1.0f};
			constexpr const graphics::Color grass_color = graphics::Color {0.f, 0.75f, 0.f, 1.0f};
			constexpr const graphics::Color forest_color = graphics::Color {0.15f, 0.50f, 0.f, 1.0f};
			constexpr const graphics::Color ocean_color = graphics::Color {0.f, 0.25f, 0.60f, 1.0f};
			constexpr const graphics::Color mountain_color = graphics::Color {0.85f, 0.85f, 0.f, 1.0f};
			constexpr const graphics::Color swamp_color = graphics::Color {0.f, 0.50f, 0.40f, 1.0f};
			constexpr const graphics::Color cave_color = graphics::Color {0.65f, 0.20f, 0.80f, 1.0f};
			for (int gx = 0; gx < this->getWidth(); ++gx) {
				for (int gy = 0; gy < this->getHeight(); ++gy) {
					const auto& gnode = this->getTerrainGraph(false).getNode(gx, gy);;
					const auto& anode = this->getTerrainGraph(true).getNode(gx, gy);
					const auto weight_diff = gnode.getWeight() - anode.getWeight();
					if (gnode.isBlocked() && anode.isBlocked()) {
						// Mountains: Blocked to all
						renderer.paintSquare(*this, gx, gy, outline_color, mountain_color);
					}
					else if (gnode.isBlocked()) {
						// Ocean: Blocked to ground
						renderer.paintSquare(*this, gx, gy, outline_color, ocean_color);
					}
					else if (anode.isBlocked()) {
						// Cave: Blocked to air
						renderer.paintSquare(*this, gx, gy, outline_color, cave_color);
					}
					else if (weight_diff > 0) {
						// Swamp: More difficult for ground troops
						renderer.paintSquare(*this, gx, gy, outline_color, swamp_color);
					}
					else if (weight_diff < 0) {
						// Forest: More difficult for air troops
						renderer.paintSquare(*this, gx, gy, outline_color, forest_color);
					}
					else {
						// Grass: Equal weights
						renderer.paintSquare(*this, gx, gy, outline_color, grass_color);
					}
				} // End inner for
			} // End outer for
		}

		void GameMap::drawStartGoal(const graphics::Renderer2D& renderer) const noexcept {
			constexpr const graphics::Color transparent_color = graphics::Color {0.f, 0.f, 0.f, 0.f};
			constexpr const graphics::Color ground_start_color = graphics::Color {0.40f, 0.f, 0.f, 0.65f};
			constexpr const graphics::Color ground_end_color = graphics::Color {0.95f, 0.f, 0.f, 0.65f};
			constexpr const graphics::Color air_start_color = graphics::Color {0.40f, 0.40f, 0.40f, 0.65f};
			constexpr const graphics::Color air_end_color = graphics::Color {0.90f, 0.90f, 0.90f, 0.65f};
			constexpr const graphics::Color white_color = graphics::Color {1.f, 1.f, 1.f, 0.75f};
			const auto* ground_start_node = this->getTerrainGraph(false).getStartNode();
			const auto* ground_end_node = this->getTerrainGraph(false).getGoalNode();
			const auto* air_start_node = this->getTerrainGraph(true).getStartNode();
			const auto* air_end_node = this->getTerrainGraph(true).getGoalNode();
			renderer.paintSquare(*this, ground_start_node->getGameX(), ground_start_node->getGameY(),
				transparent_color, ground_start_color);
			renderer.paintSquare(*this, ground_end_node->getGameX(), ground_end_node->getGameY(),
				transparent_color, ground_end_color);
			renderer.paintSquare(*this, air_start_node->getGameX(), air_start_node->getGameY(),
				transparent_color, air_start_color);
			renderer.paintSquare(*this, air_end_node->getGameX(), air_end_node->getGameY(),
				transparent_color, air_end_color);
			// Make it clearer which points are the start locations.
			const auto cs_width = this->getGameSquareWidth<FLOAT>();
			const auto cs_height = this->getGameSquareHeight<FLOAT>();
			const auto ground_start_lsx = static_cast<FLOAT>(this->convertToScreenX(ground_start_node->getGameX()));
			const auto ground_start_tsy = static_cast<FLOAT>(this->convertToScreenY(ground_start_node->getGameY()));
			const auto air_start_lsx = static_cast<FLOAT>(this->convertToScreenX(air_start_node->getGameX()));
			const auto air_start_tsy = static_cast<FLOAT>(this->convertToScreenY(air_start_node->getGameY()));
			if (this->getColumns() <= 40 && this->getRows() <= 40) {
				renderer.drawText(L"GS", white_color, renderer.createRectangle(ground_start_lsx,
					ground_start_tsy, cs_width, cs_height), false);
				renderer.drawText(L"AS", white_color, renderer.createRectangle(air_start_lsx,
					air_start_tsy, cs_width, cs_height), false);
			}
			else {
				renderer.drawSmallText(L"GS", white_color, renderer.createRectangle(ground_start_lsx,
					ground_start_tsy, cs_width, cs_height), false);
				renderer.drawSmallText(L"AS", white_color, renderer.createRectangle(air_start_lsx,
					air_start_tsy, cs_width, cs_height), false);
			}
		}

		void GameMap::drawMarkedTiles(const graphics::Renderer2D& renderer) const noexcept {
			constexpr const graphics::Color o_color {0.f, 0.f, 1.f, 0.9f};
			constexpr const graphics::Color f_color {0.8f, 0.8f, 0.8f, 0.2f};
			for (int gx = 0; gx < this->getWidth(); ++gx) {
				for (int gy = 0; gy < this->getHeight(); ++gy) {
					if (this->getHighlightGraph().getNode(gx, gy).isBlocked()) {
						renderer.paintSquare(*this, gx, gy, o_color, f_color);
					}
				}
			}
		}
	}
}