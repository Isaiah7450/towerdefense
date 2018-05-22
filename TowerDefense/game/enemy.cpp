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
			move_diagonally {etype->canMoveDiagonally()},
			status_effects {} {
			this->my_path = this->my_pathfinder.findPath();
			this->current_node = this->my_path.front();
			this->my_path.pop();
			this->changeDirection();
		}

		bool Enemy::update() {
			// Check if we are still alive
			if (!this->isAlive()) {
				return true;
			}
			std::vector<int> statuses_to_remove {};
			// Apply status effects
			for (unsigned int i = 0; i < this->status_effects.size(); ++i) {
				if (this->status_effects[i]->update(*this)) {
					statuses_to_remove.emplace_back(i);
				}
			}
			// Remove old status effects
			for (unsigned int i = 0; i < statuses_to_remove.size(); ++i) {
				this->status_effects.erase(this->status_effects.begin() + statuses_to_remove[i] - i);
			}
			// Perform movement
			const double my_speed = this->getCurrentSpeed() / game::logic_framerate /
				this->current_node->getWeight();
			const double rx = my_speed;
			const double ry = my_speed;
			if (!this->isStunned()) {
				this->translate(rx * std::cos(this->current_direction), ry * std::sin(this->current_direction));
			}
			const double dx = (this->getNextNode()->getGameX() + 0.5) - this->getGameX();
			const double dy = (this->getNextNode()->getGameY() + 0.5) - this->getGameY();
			bool update_next_node = math::get_sqrt(dx * dx + dy * dy) <= my_speed + 0.25 / game::logic_framerate;
			if (update_next_node) {
				// Change path
				if (this->my_path.size() == 1) {
					return true;
				}
				this->current_node = this->getNextNode();
				this->my_path.pop();
				this->changeDirection();
			}
			// Reset speed multipliers to normal
			this->speed_multiplier = 1.0;
			this->speed_boosts = {1.0, 1.0, 1.0};
			this->stun_active = false;
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

		void Enemy::takeDamage(double dmg, double wap, bool bypass_armor_completely) {
			if (this->hasArmor()) {
				// Damage to armor => Base Damage * ((1.0 - Armor Reduce) + Piercing)
				// to a maximum of the base damage amount
				const double ar_protect = math::get_max(this->getBaseType().getArmorReduce() - wap,
					0.0);
				const double ar_dmg = dmg * (1.0 - ar_protect);
				if (wap > 0) {
					// Damage to health => Armor Damage * Piercing
					const double hp_dmg = ar_dmg * wap;
					this->current_health -= hp_dmg;
				}
				if (!bypass_armor_completely) {
					this->current_armor_health -= ar_dmg;
				}
			}
			else {
				// Armor Piercing ignored
				this->current_health -= dmg;
			}
		}

		void Enemy::heal(double amt) {
			this->current_health = math::get_min(this->getMaxHealth(), this->getHealth() + amt);
		}

		void Enemy::addStatus(std::unique_ptr<StatusEffectBase>&& effect) {
			this->status_effects.emplace_back(std::move(effect));
		}

		void Enemy::changeStrategy(const GameMap& gmap,
			pathfinding::HeuristicStrategies new_strat, bool diag_move) {
			this->current_strat = new_strat;
			this->move_diagonally = diag_move;
			// Obtain new path
			this->my_pathfinder = pathfinding::Pathfinder {gmap, this->getBaseType().isFlying(),
				diag_move, new_strat};
			this->my_path = this->my_pathfinder.findPath(1.0,
				static_cast<int>(this->getGameX()),
				static_cast<int>(this->getGameY()));
			this->current_node = this->my_path.front();
			this->my_path.pop();
			this->changeDirection();
		}

		void Enemy::changeDirection() noexcept {
			const double dx = (this->getNextNode()->getGameX() + 0.5) - this->getGameX();
			const double dy = (this->getNextNode()->getGameY() + 0.5) - this->getGameY();
			this->current_direction = std::atan2(dy, dx);
		}
	}
}