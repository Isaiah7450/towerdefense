#pragma once
// File Author: Isaiah Hoffman
// File Created: June 4, 2018
#include <string>
#include <vector>
#include <memory>
#include <array>
#include <variant>
#include <utility>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../pathfinding/grid.hpp"
#include "./game_object.hpp"
#include "./tower_types.hpp"
#include "./game_formulas.hpp"

namespace hoffman_isaiah {
	namespace winapi {
		class TowerUpgradeInfoDialog;
	}

	namespace game {
		// Forward declarations
		class Enemy;
		class Shot;

		/// <summary>Class that represents a tower (or a wall).</summary>
		class Tower : public GameObject {
			friend class winapi::TowerUpgradeInfoDialog;
		public:
			Tower(graphics::DX::DeviceResources2D* dev_res, const GameMap& my_map,
				const TowerType* ttype, graphics::Color o_color, double build_gx, double build_gy) :
				GameObject {dev_res, my_map, ttype->getShape(), o_color, ttype->getColor(),
				build_gx, build_gy, 0.7f, 0.7f},
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
			std::vector<std::unique_ptr<Shot>> update(const std::vector<std::unique_ptr<Enemy>>& enemies);
			
			/// <summary>Upgrades a tower from its previous level to the new level. (Note: Do not use
			///          to upgrade multiple times. Use setTowerUpgradeStatus instead.)</summary>
			/// <param name="new_level">The new level of the tower.</param>
			/// <param name="upgrade_option">The upgrade option of the tower.</param>
			void upgradeTower(int new_level, TowerUpgradeOption upgrade_option) {
				const auto my_upgrades = this->getBaseType()->getUpgrades();
				this->level = new_level;
				this->upgrade_path |= (1U << static_cast<unsigned>(new_level - 1)) * static_cast<int>(upgrade_option);
				for (const auto& upgrade : my_upgrades) {
					if (upgrade.getLevel() == new_level && upgrade.getOption() == upgrade_option) {
						this->dmg_multiplier *= upgrade.getDamageMultiplier();
						this->fire_speed_multiplier *= upgrade.getSpeedMultiplier();
						this->fire_range_multiplier *= upgrade.getRangeMultiplier();
						this->volley_shots_multiplier *= upgrade.getAmmoMultiplier();
						this->reload_delay_multiplier *= upgrade.getDelayMultiplier();
						// Update the special with the highest values.
						const auto my_iterator = this->upgrade_specials.find(upgrade.getSpecial());
						if (my_iterator == this->upgrade_specials.end()) {
							this->upgrade_specials.emplace(upgrade.getSpecial(), std::make_pair<double, double>(upgrade.getSpecialChance(),
								upgrade.getSpecialPower()));
						}
						else {
							if (my_iterator->second.first < upgrade.getSpecialChance()) {
								my_iterator->second.first = upgrade.getSpecialChance();
							}
							if (my_iterator->second.second < upgrade.getSpecialPower()) {
								my_iterator->second.second = upgrade.getSpecialPower();
							}
						}
						this->updateRating();
						this->updateValue();
						break;
					}
				}
			}
			// Setters
			void setTowerUpgradeStatus(int new_level, unsigned int new_path) noexcept {
				for (int i = 2; i <= new_level; ++i) {
					const TowerUpgradeOption my_option = ((new_path >> (i - 1)) & 1) == 0
						? TowerUpgradeOption::One : TowerUpgradeOption::Two;
					this->upgradeTower(i, my_option);
				}
			}
			// Getters
			const TowerType* getBaseType() const noexcept {
				return this->base_type;
			}
			// Upgrade getters:
			int getLevel() const noexcept {
				return this->level;
			}
			unsigned int getUpgradePath() const noexcept {
				return this->upgrade_path;
			}
			/// <returns>The tower's special abilities gained from upgrades. For the pair:
			/// 1st => Chance, 2nd => Power.</returns>
			const std::map<TowerUpgradeSpecials, std::pair<double, double>>& getUpgradeSpecials() {
				return this->upgrade_specials;
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
				return towers::getFiringArea(this->getFiringRange(), this->getBaseType()->getFiringMethod(), this->getBaseType()->isWall());
			}
			/// <returns>The expected amount of raw damage output by the tower per shot on average.</returns>
			double getAverageDamagePerShot() const noexcept {
				return towers::getAverageDamagePerShot(this->getBaseType()->getShotTypes(), this->getDamageMultiplier());
			}
			/// <returns>The weighted average effect rating of the tower's shot types.</returns>
			double getAverageShotEffectRating() const noexcept {
				return towers::getAverageShotEffectRating(this->getBaseType()->getShotTypes());
			}
			/// <returns>The weighted average rating of the tower's shot types.</returns>
			double getAverageShotRating() const noexcept {
				return towers::getAverageShotRating(this->getBaseType()->getShotTypes());
			}
			/// <returns>The tower's average rate of fire as the average number of shots fired per second.</returns>
			double getRateOfFire() const noexcept {
				return towers::getRateOfFire(this->getFiringSpeed(), this->getVolleyShots(), this->getReloadDelay(), this->getBaseType()->isWall());
			}
			/// <returns>The tower's expected DPS.</returns>
			double getExpectedDPS() const noexcept {
				return towers::getExpectedDPS(this->getAverageDamagePerShot(), this->getRateOfFire());
			}
			/// <summary>Updates the stored rating value of the tower.</summary>
			void updateRating() noexcept {
				this->rating = towers::getRating(this->getRateOfFire(), this->getFiringRange(), this->getFiringArea(),
					this->getBaseType()->getFiringMethod(), this->getBaseType()->getTargetingStrategy(),
					this->getAverageDamagePerShot(), this->getAverageShotEffectRating(), this->getBaseType()->isWall());
			}
			/// <summary>Updates the stored cost (i.e.: value) of the tower.</summary>
			void updateValue() noexcept {
				this->value = towers::getCost(this->getRating(), this->getBaseType()->getCostAdjustment(),
					this->getBaseType()->isWall());
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
			// Getters
			graphics::DX::DeviceResources2D* getDeviceResources() const noexcept {
				return this->device_resources;
			}
			// Fields
			/// <summary>Pointer to the game's device resources.</summary>
			graphics::DX::DeviceResources2D* device_resources;
			/// <summary>The template type of this projectile.</summary>
			const TowerType* base_type;
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
			/// <summary>The choices the user has made regarding upgrading the tower stored in binary.</summary>
			unsigned int upgrade_path {0U};
			/// <summary>The tower's upgrades specials. For the pair: 1st => Chance, 2nd => Power</summary>
			std::map<TowerUpgradeSpecials, std::pair<double, double>> upgrade_specials {};
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