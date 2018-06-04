#pragma once
// File Author: Isaiah Hoffman
// File Created: June 4, 2018
#include <string>
#include <vector>
#include <memory>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../pathfinding/grid.hpp"
#include "./game_object.hpp"
#include "./shot_types.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations
		class Enemy;
		class Tower;

		/// <summary>Represents a projectile.</summary>
		class Shot : public GameObject {
		public:
			Shot(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
				std::shared_ptr<ShotBaseType> stype, graphics::Color o_color, const Tower& ot, double angle);

			/// <summary>Advances the projectile's state by one logical frame.</summary>
			/// <param name"enemies">The list of enemies currently present in the game.</param>
			/// <returns>True if this shot should be deleted; otherwise, false.</returns>
			bool update(const GameMap& gmap, std::vector<std::unique_ptr<Enemy>>& enemies);
		private:
			/// <summary>The template type of this projectile.</summary>
			std::shared_ptr<ShotBaseType> base_type;
			/// <summary>The tower that the projectile originated from.</summary>
			const Tower& origin_tower;
			/// <summary>The angle (in radians) that the projectile moves in each turn.</summary>
			double theta;
		};
	}
}