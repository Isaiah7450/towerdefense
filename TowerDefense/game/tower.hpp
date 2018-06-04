#pragma once
// File Author: Isaiah Hoffman
// File Created: June 4, 2018
#include <string>
#include <vector>
#include <memory>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./game_object.hpp"
#include "./tower_types.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations
		class Enemy;
		class Shot;

		/// <summary>Class that represents a tower (or a wall).</summary>
		class Tower : public GameObject {
		public:
			Tower(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
				std::shared_ptr<TowerType> ttype, graphics::Color o_color, double build_gx, double build_gy) :
				GameObject {dev_res, ttype->getShape(), o_color, ttype->getColor(), build_gx, build_gy,
					0.7f, 0.7f},
				device_resources {dev_res},
				base_type {ttype} {
			}
			/// <summary>Advances the tower's state by one logical frame.</summary>
			/// <param name="enemies">The list of living enemies.</param>
			/// <returns>The shot created by the tower or nullptr if no shot was created.</returns>
			std::unique_ptr<Shot> update(const std::vector<std::unique_ptr<Enemy>>& enemies);
			// Getters
			const std::shared_ptr<TowerType>& getBaseType() const noexcept {
				return this->base_type;
			}
		protected:
			/// <summary>Finds a target enemy for the tower.</summary>
			/// <param name="enemies">The list of enemies currently in the game.</param>
			/// <returns>Nullptr if no valid target was found; otherwise, the selected target.</returns>
			const Enemy* findTarget(const std::vector<std::unique_ptr<Enemy>>& enemies) const;
			/// <summary>Creates and returns a new shot.</summary>
			/// <param name="target">The target enemy found by findTarget().</param>
			/// <returns>The newly created projectile.</returns>
			std::unique_ptr<Shot> createShot(const Enemy* target) const;
		private:
			/// <summary>Pointer to the game's device resources.</summary>
			std::shared_ptr<graphics::DX::DeviceResources2D> device_resources;
			/// <summary>The template type of this projectile.</summary>
			std::shared_ptr<TowerType> base_type;
			/// <summary>The number of shots that have been fired since reloading.</summary>
			int shots_fired_since_reload {0};
			/// <summary>The amount of frames remaining before the tower can fire again
			/// (unless it has to reload.)</summary>
			double frames_til_next_shot {0.0};
			/// <summary>The amount of frames remaining before the tower is fully reloaded.</summary>
			double frames_to_reload {0.0};
			/// <summary>Whether or not the tower must completely reload before firing again.</summary>
			bool must_reload {false};
			/// <summary>The index of the last angle that the tower fired its shot from.</summary>
			mutable int angle_index {-1};
		};
	}
}