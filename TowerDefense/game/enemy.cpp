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
	}
}