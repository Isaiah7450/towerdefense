// File Author: Isaiah Hoffman
// File Created: May 21, 2018
#include <string>
#include <queue>
#include <array>
#include <vector>
#include <memory>
#include <cmath>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../graphics/graphics.hpp"
#include "./../pathfinding/graph_node.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./enemy_type.hpp"
#include "./enemy.hpp"
#include "./game_object.hpp"
#include "./status_effects.hpp"

namespace hoffman::isaiah {
	namespace game {
		Enemy::Enemy(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
			std::shared_ptr<EnemyType> etype, graphics::Color o_color,
			const GameMap& gmap, int level, int difficulty) :
			GameObject {dev_res, etype->getShape(), o_color, etype->getColor(),
			gmap.getTerrainGraph(etype->isFlying()).getStartNode()->getGameX() + 0.5,
			gmap.getTerrainGraph(etype->isFlying()).getStartNode()->getGameY() + 0.5, 0.7, 0.7},
			base_type {etype},
			my_pathfinder {gmap, etype->isFlying(), etype->canMoveDiagonally(), etype->getDefaultStrategy()},
			my_path {},
			current_node {nullptr},
			current_direction {0.0},
			current_health {etype->getBaseHP() + (5.0 * level * difficulty)},
			maximum_health {etype->getBaseHP() + (5.0 * level * difficulty)},
			current_armor_health {etype->getBaseArmorHP() * (1.0 + level * 0.001 * difficulty)},
			maximum_armor_health {etype->getBaseArmorHP() * (1.0 + level * 0.001 * difficulty)},
			current_strat {etype->getDefaultStrategy()},
			current_speeds {{etype->getBaseWalkingSpeed(), etype->getBaseRunningSpeed(), etype->getBaseInjuredSpeed()}},
			speed_multiplier {1.0},
			status_effects {} {
			this->my_path = this->my_pathfinder.findPath();
			this->current_node = this->my_path.front();
			this->my_path.pop();
			const double dx = (this->getNextNode()->getGameX() + 0.5) - this->getGameX();
			const double dy = (this->getNextNode()->getGameY() + 0.5) - this->getGameY();
			this->current_direction = std::atan2(dy, dx);
		}

		bool Enemy::update() {
			// Check if we are still alive
			if (!this->isAlive()) {
				return true;
			}
			// Apply status effects
			for (auto& status : this->status_effects) {
				status->update(*this);
			}
			// Perform movement
			const double my_speed = this->getCurrentSpeed() / game::logic_framerate /
				this->current_node->getWeight();
			const double rx = my_speed;
			const double ry = my_speed;
			this->translate(rx * std::cos(this->current_direction), ry * std::sin(this->current_direction));
			double dx = (this->getNextNode()->getGameX() + 0.5) - this->getGameX();
			double dy = (this->getNextNode()->getGameY() + 0.5) - this->getGameY();
			bool update_next_node = math::get_sqrt(dx * dx + dy * dy) <= my_speed;
			if (update_next_node) {
				// Change path
				if (this->my_path.size() == 1) {
					return true;
				}
				this->current_node = this->getNextNode();
				this->my_path.pop();
				dx = (this->getNextNode()->getGameX() + 0.5) - this->getGameX();
				dy = (this->getNextNode()->getGameY() + 0.5) - this->getGameY();
				this->current_direction = std::atan2(dy, dx);
			}
			return !this->isAlive();
		}

		void Enemy::draw(const graphics::Renderer2D& renderer) const noexcept {
			// Call base class member first
			GameObject::draw(renderer);
			// Draw health bars
			constexpr const graphics::Color bar_outline_color {0.2f, 0.2f, 0.2f, 1.0f};
			constexpr const graphics::Color bar_empty_color {0.7f, 0.7f, 0.7f, 1.0f};
			constexpr const graphics::Color health_fill_color {0.8f, 0.0f, 0.0f, 0.9f};
			constexpr const graphics::Color armor_fill_color {0.0f, 0.0f, 0.8f, 0.9f};
			const double hp_percent = this->getHealthPercentage();
			const float bar_max_width = 0.63f * graphics::getGameSquareWidth<float>();
			const float bar_height = 0.075f * graphics::getGameSquareHeight<float>();
			const float hp_bar_offset = 5.5f * graphics::screen_height / 645.f;
			const float hp_bar_start_x = static_cast<float>(this->getScreenX()) - bar_max_width / 2.f;
			const float hp_bar_end_x = static_cast<float>(hp_bar_start_x + hp_percent * bar_max_width);
			const float hp_bar_start_y = static_cast<float>(this->getScreenY()) - bar_height / 2.f - hp_bar_offset;
			const float hp_bar_end_y = hp_bar_start_y + bar_height;
			const D2D1_RECT_F hp_bar_filled_rc {hp_bar_start_x, hp_bar_start_y, hp_bar_end_x, hp_bar_end_y};
			const D2D1_RECT_F hp_bar_outline_rc {hp_bar_start_x, hp_bar_start_y, hp_bar_start_x + bar_max_width,
				hp_bar_end_y};
			renderer.setBrushColors(bar_outline_color, bar_empty_color);
			renderer.outlineRectangle(hp_bar_outline_rc);
			renderer.setFillColor(health_fill_color);
			renderer.fillRectangle(hp_bar_filled_rc);
			if (this->hasArmor()) {
				const double ahp_percent = this->getArmorPercentage();
				const float ahp_bar_offset = hp_bar_offset + bar_height + 0.85f * graphics::screen_height / 645.f;
				const float ahp_bar_start_x = hp_bar_start_x;
				const float ahp_bar_end_x = static_cast<float>(ahp_bar_start_x + ahp_percent * bar_max_width);
				const float ahp_bar_start_y = static_cast<float>(this->getScreenY()) - bar_height / 2.f - ahp_bar_offset;
				const float ahp_bar_end_y = ahp_bar_start_y + bar_height;
				const D2D1_RECT_F ahp_bar_filled_rc {ahp_bar_start_x, ahp_bar_start_y, ahp_bar_end_x, ahp_bar_end_y};
				const D2D1_RECT_F ahp_bar_outline_rc {ahp_bar_start_x, ahp_bar_start_y, ahp_bar_start_x + bar_max_width,
					ahp_bar_end_y};
				renderer.setFillColor(bar_empty_color);
				renderer.outlineRectangle(ahp_bar_outline_rc);
				renderer.setFillColor(armor_fill_color);
				renderer.fillRectangle(ahp_bar_filled_rc);
			}
		}
	}
}