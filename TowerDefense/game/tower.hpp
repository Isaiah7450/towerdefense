#pragma once
// File Author: Isaiah Hoffman
// File Created: June 4, 2018
#include <string>
#include <vector>
#include <memory>
#include <array>
#include <variant>
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
				base_type {ttype},
				rating {ttype->getRating()},
				value {ttype->getCost()} {
			}
			// Overrides GameObject::draw()
			void draw(const graphics::Renderer2D& renderer) const noexcept override;
			/// <summary>Toggles the showing of the area that the tower covers.</summary>
			void toggleShowCoverage() noexcept {
				this->show_coverage = !this->show_coverage;
			}

			/// <summary>Reset's the tower's state to its initial state.</summary>
			void resetTower() noexcept {
				this->frames_til_next_shot = 0;
				this->shots_fired_since_reload = 0;
				this->must_reload = false;
				this->frames_to_reload = 0;
			}

			/// <summary>Advances the tower's state by one logical frame.</summary>
			/// <param name="enemies">The list of living enemies.</param>
			/// <returns>The shot created by the tower or nullptr if no shot was created.</returns>
			std::unique_ptr<Shot> update(const std::vector<std::unique_ptr<Enemy>>& enemies);
			// Getters
			const std::shared_ptr<TowerType>& getBaseType() const noexcept {
				return this->base_type;
			}
			// Upgrade getters:
			int getLevel() const noexcept {
				return this->level;
			}
			double getDamageMultiplier() const noexcept {
				return this->dmg_multiplier;
			}
			double getFiringSpeed() const noexcept {
				return this->getBaseType()->getFiringSpeed() * this->fire_speed_multiplier;
			}
			double getFiringRange() const noexcept {
				return this->getBaseType()->getFiringRange() * this->fire_range_multiplier;
			}
			int getVolleyShots() const noexcept {
				return static_cast<int>(std::round(this->getBaseType()->getVolleyShots()
					* this->volley_shots_multiplier));
			}
			int getReloadDelay() const noexcept {
				return static_cast<int>(std::round(this->getBaseType()->getReloadDelay()
					* this->reload_delay_multiplier));
			}
			double getRating() const noexcept {
				return this->rating;
			}
			double getCost() const noexcept {
				return this->value;
			}
			// Computations
			// Note: For the most part, all of these should be very close to the same
			//       as the versions in TowerType.
			double getFiringArea() const noexcept {
				return this->getBaseType()->getFiringMethod().getMethod() == FiringMethodTypes::Default
					? this->getFiringRange() * this->getFiringRange() * math::pi
					: this->getFiringRange() * this->getFiringRange()
					* (this->getBaseType()->getFiringMethod().getMaximumAngle()
						- this->getBaseType()->getFiringMethod().getMinimumAngle()) / 2.0;
			}
			/// <returns>The expected amount of raw damage output by the tower per shot on average.</returns>
			double getAverageDamagePerShot() const noexcept;
			/// <returns>The weighted average of the tower's shot types.</returns>
			double getAverageShotRating() const noexcept;
			/// <returns>The tower's average rate of fire as the average number of shots fired per second.</returns>
			double getRateOfFire() const noexcept {
				if (this->getBaseType()->isWall()) {
					return 0;
				}
				if (this->getVolleyShots() == 0 || this->getReloadDelay() == 0) {
					return this->getFiringSpeed();
				}
				const double secs_to_reload = this->getReloadDelay() / 1000.0;
				const double total_cycle_time = (1.0 / this->getFiringSpeed())
					* this->getVolleyShots() + secs_to_reload;
				return static_cast<double>(this->getVolleyShots()) / total_cycle_time;
			}
			/// <returns>The tower's expected DPS.</returns>
			double getExpectedDPS() const noexcept {
				return this->getAverageDamagePerShot() * this->getRateOfFire();
			}
			/// <summary>Updates the stored rating value of the tower.</summary>
			void updateRating() noexcept;
			/// <summary>Updates the stored cost (i.e.: value) of the tower.</summary>
			void updateValue() noexcept {
				if (this->getBaseType()->isWall()) {
					return;
				}
				this->value = this->getBaseType()->getCostAdjustment() + this->getRating() / 10.5 + 1.0;
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
			/// <summary>Shows the tower's coverage.</summary>
			bool show_coverage {false};
			/// <summary>The index of the last angle that the tower fired its shot from.</summary>
			mutable int angle_index {-1};
			// Upgrade related stuff.
			/// <summary>The level of the tower.</summary>
			int level {1};
			/// <summary>The tower's rating.</summary>
			double rating;
			/// <summary>The tower's value.</summary>
			double value;
			// Note that these values are effectively "cache" values.
			/// <summary>Multiplies the tower's damage by a %.</summary>
			double dmg_multiplier {1.0};
			/// <summary>Multiplies the tower's firing speed by a %.</summary>
			double fire_speed_multiplier {1.0};
			/// <summary>Multiplies the tower's firing range by a %.</summary>
			double fire_range_multiplier {1.0};
			/// <summary>Multiplies the tower's volley shots by a %.</summary>
			double volley_shots_multiplier {1.0};
			/// <summary>Multiplies the tower's reload delay by a %.</summary>
			double reload_delay_multiplier {1.0};
		};
	}
}