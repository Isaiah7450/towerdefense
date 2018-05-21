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
			gmap.getTerrainGraph(etype->isFlying()).getStartNode()->getGameX(),
			gmap.getTerrainGraph(etype->isFlying()).getStartNode()->getGameY(), 0.8, 0.8},
			base_type {etype},
			my_pathfinder {gmap, etype->isFlying(), etype->canMoveDiagonally(), etype->getDefaultStrategy()},
			my_path {},
			current_node {nullptr},
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
			double dx = this->getGameX() - this->getNextNode()->getGameX();
			double dy = this->getGameY() - this->getNextNode()->getGameY();
			double theta = std::atan2(dy, dx);
			this->translate(this->getCurrentSpeed() * std::cos(theta) / game::logic_framerate,
				this->getCurrentSpeed() * std::sin(theta) / game::logic_framerate);
			dx = math::get_abs(this->getGameX() - this->getNextNode()->getGameX());
			dy = math::get_abs(this->getGameY() - this->getNextNode()->getGameY());
			if (dx <= 0.005 && dy <= 0.005) {
				// Change path
				if (this->my_path.empty()) {
					return true;
				}
				this->current_node = this->getNextNode();
				this->my_path.pop();
			}
			return !this->isAlive();
		}
	}
}