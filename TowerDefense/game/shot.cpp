// File Author: Isaiah Hoffman
// File Created: June 4, 2018
#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../graphics/graphics.hpp"
#include "./../pathfinding/grid.hpp"
#include "./enemy.hpp"
#include "./game_object.hpp"
#include "./shot.hpp"
#include "./shot_types.hpp"
#include "./tower.hpp"

namespace hoffman::isaiah {
	namespace game {
		Shot::Shot(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
			std::shared_ptr<ShotBaseType> stype, graphics::Color o_color, const Tower& ot, double angle) :
			GameObject {dev_res, stype->getShape(), o_color, stype->getColor(), ot.getGameX(), ot.getGameY(),
				0.3f, 0.3f},
			base_type {stype},
			origin_tower {ot},
			theta {angle} {
		}

		bool Shot::update(const GameMap& gmap, std::vector<std::unique_ptr<Enemy>>& enemies) {
			// Update location
			const double r = this->base_type->getSpeed() / game::logic_framerate;
			this->translate(std::cos(this->theta) * r, std::sin(this->theta) * r);
			// Check for hits
			unsigned int i = 0;
			for (; i < enemies.size(); ++i) {
				auto& e = enemies[i];
				if (this->intersects(*e)) {
					this->base_type->doHit(*e);
					break;
				}
			}
			if (i < enemies.size()) {
				// Check for splash effects as well
				for (unsigned int j = 0; j < enemies.size(); ++j) {
					auto& e = enemies[i];
					if (j != i) {
						const double edx = std::abs(this->getGameX() - e->getGameX());
						const double edy = std::abs(this->getGameY() - e->getGameY());
						const double e_dist = std::sqrt(edx * edx + edy * edy);
						if (e_dist <= this->base_type->getImpactRadius()) {
							this->base_type->doSplashHit(*e);
						}
					}
				}
			}
			const double tdx = std::abs(this->getGameX() - this->origin_tower.getGameX());
			const double tdy = std::abs(this->getGameY() - this->origin_tower.getGameY());
			const int igx = static_cast<int>(std::floor(this->getGameX()));
			const int igy = static_cast<int>(std::floor(this->getGameY()));
			const bool is_on_blocked_space = gmap.getTerrainGraph(true).getNode(igx, igy).isBlocked();
			return i < enemies.size() || is_on_blocked_space
				|| std::sqrt(tdx * tdx + tdy * tdy) > this->origin_tower.getBaseType()->getFiringRange();
		}
	}
}