#pragma once
// File Author: Isaiah Hoffman
// File Created: June 2, 2018
#include <array>
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include <variant>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./game_object_type.hpp"
#include "./game_formulas.hpp"

namespace hoffman::isaiah {
	namespace winapi {
		class TowerInfoDialog;
		class TowerUpgradeInfoDialog;
	}

	namespace game {
		// Forward declarations
		class ShotBaseType;
		class Tower;
		// Enumrations
		/// <summary>Enumeration of upgrades that can be performed to a tower.</summary>
		enum class TowerUpgradeTypes {
			Damage, Speed, Range, Ammo, Delay, Strategy, Sentinel_DO_NOT_USE
		};
	}

	inline std::wstring operator*(game::TowerUpgradeTypes upgrade_type) noexcept {
		constexpr const std::array<const wchar_t*,
			static_cast<int>(game::TowerUpgradeTypes::Sentinel_DO_NOT_USE)> my_upgrade_strs {
			L"Damage", L"Speed", L"Range", L"Ammo", L"Reload Delay", L"Strategy"
		};
		return my_upgrade_strs.at(static_cast<int>(upgrade_type));
	}

	namespace game {
		/// <summary>Enumeration constants that represents the methods that
		/// a tower uses to fire projectiles.</summary>
		enum class FiringMethodTypes {
			Default, Static, Pulse
		};

		/// <summary>Class that represents the firing method for a tower.</summary>
		class FiringMethod {
		public:
			FiringMethod(std::wstring ref_name, FiringMethodTypes m, std::vector<double> a = {}, int d = 0) :
				reference_name {ref_name},
				method {m},
				angles {a},
				duration {d} {
			}
			// Getters
			std::wstring getReferenceName() const {
				return this->reference_name;
			}
			FiringMethodTypes getMethod() const noexcept {
				return this->method;
			}
			std::vector<double> getAngles() const noexcept {
				return this->angles;
			}
			int getDuration() const noexcept {
				return this->duration;
			}
			/// <returns>The minimum angle in the list of angles, or -20 if this does not apply.</returns>
			double getMinimumAngle() const noexcept {
				if (this->method != FiringMethodTypes::Default) {
					// (The angles are sorted during input.)
					return this->angles[0];
				}
				return -20;
			}
			/// <returns>The maximum angle in the list of angles, or 20 if this does not apply.</returns>
			double getMaximumAngle() const noexcept {
				if (this->method != FiringMethodTypes::Default) {
					// (The angles are sorted during input.)
					return this->angles.at(this->angles.size() - 1);
				}
				return 20;
			}
		private:
			/// <summary>The name used to refer to this method.</summary>
			std::wstring reference_name;
			/// <summary>The firing method being employed.</summary>
			FiringMethodTypes method;
			// The following two properties depend on the method selected.
			/// <summary>The angles that the tower fires at.</summary>
			std::vector<double> angles;
			/// <summary>The duration that projectiles remain active for
			/// before disappearing.</summary>
			int duration;
		};

		/// <summary>Set of enumeration constants that define the general strategy used.</summary>
		enum class TargetingStrategyTypes {
			Distances, Statistics, Names
		};
		/// <summary>Set of enumeration constants that define the targeting protocol used.</summary>
		enum class TargetingStrategyProtocols {
			Lowest, Highest
		};
		/// <summary>Set of enumeration constants that define the different parameters that
		/// can be utilized.</summary>
		enum class TargetingStrategyStatistics {
			Not_Applicable, Damage, Health, Armor_Health, Armor_Reduce, Speed, Buffs
		};
		/// <summary>Class that represents the targeting strategy for a tower.</summary>
		class TargetingStrategy {
		public:
			TargetingStrategy(std::wstring ref_name, TargetingStrategyTypes s, TargetingStrategyProtocols p,
				TargetingStrategyStatistics ts = TargetingStrategyStatistics::Not_Applicable) :
				reference_name {ref_name},
				strategy {s},
				protocol {p},
				test_stat {ts},
				target_names {} {
			}
			TargetingStrategy(std::wstring ref_name, TargetingStrategyTypes s, TargetingStrategyProtocols p,
				std::vector<std::wstring> tnames) :
				reference_name {ref_name},
				strategy {s},
				protocol {p},
				test_stat {TargetingStrategyStatistics::Not_Applicable},
				target_names {tnames} {
			}
			// Getters
			TargetingStrategyTypes getStrategy() const noexcept {
				return this->strategy;
			}
			TargetingStrategyProtocols getProtocol() const noexcept {
				return this->protocol;
			}
			TargetingStrategyStatistics getTestStatistic() const noexcept {
				return this->test_stat;
			}
			std::vector<std::wstring> getTargetNames() const noexcept {
				return this->target_names;
			}
			std::wstring getReferenceName() const noexcept {
				return this->reference_name;
			}
		private:
			/// <summary>The name used to refer to this strategy.</summary>
			std::wstring reference_name;
			/// <summary>The general targeting strategy to use.</summary>
			TargetingStrategyTypes strategy;
			/// <summary>The protocol to use.</summary>
			TargetingStrategyProtocols protocol;
			// These last two only apply for certain strategies.
			/// <summary>The statistic to test.</summary>
			TargetingStrategyStatistics test_stat;
			/// <summary>The list of names of preferred targets.</summary>
			std::vector<std::wstring> target_names;
		};

		/// <summary>Enumeration of the options that a player can select for a tower.</summary>
		enum class TowerUpgradeOption {
			One, Two
		};
		/// <summary>Enumeration of special effects for upgraded towers.</summary>
		enum class TowerUpgradeSpecials {
			None, Extra_Cash, Multishot, Mega_Missile, Fast_Reload
		};

		/// <summary>Represents information about an upgrade.</summary>
		class TowerUpgradeInfo {
		public:
			/// <param name="cp">The percentage of the tower's current cost that the upgrade costs.</param>
			TowerUpgradeInfo(int lv, TowerUpgradeOption path, double cp, double dmg, double spd, double rng, double ammo,
				double delay, TowerUpgradeSpecials special = TowerUpgradeSpecials::None, double schance = 0,
				double spower = 0) :
				level {lv},
				option {path},
				cost_percent {cp},
				special_effect {special},
				special_chance {schance},
				special_power {spower},
				damage_change {dmg},
				speed_change {spd},
				range_change {rng},
				ammo_change {ammo},
				delay_change {delay} {
			}

			// Getters
			int getLevel() const noexcept {
				return this->level;
			}
			TowerUpgradeOption getOption() const noexcept {
				return this->option;
			}
			double getCostPercent() const noexcept {
				return this->cost_percent;
			}
			TowerUpgradeSpecials getSpecial() const noexcept {
				return this->special_effect;
			}
			double getSpecialChance() const noexcept {
				return this->special_chance;
			}
			double getSpecialPower() const noexcept {
				return this->special_power;
			}
			// Note taht stacked upgrades give additive bonuses, not multiplicative.
			double getDamageChange() const noexcept {
				return this->damage_change;
			}
			double getSpeedChange() const noexcept {
				return this->speed_change;
			}
			double getRangeChange() const noexcept {
				return this->range_change;
			}
			double getAmmoChange() const noexcept {
				return this->ammo_change;
			}
			double getDelayChange() const noexcept {
				return this->delay_change;
			}
		private:
			/// <summary>The level of the upgrade.</summary>
			int level;
			/// <summary>Which user option the upgrade corresponds with.</summary>
			TowerUpgradeOption option;
			/// <summary>The percentage of the tower's current cost that the upgrade costs.</summary>
			double cost_percent;
			/// <summary>The special effect to add/upgrade.</summary>
			TowerUpgradeSpecials special_effect;
			/// <summary>The chance that the special effect will occur.</summary>
			double special_chance;
			/// <summary>The special effect's power.</summary>
			double special_power;
			// Note: Negative values are allowed. Effective range is -1.00 < x < infinity.
			// Also note that values are additive here when stacking upgrades.
			/// <summary>The % change in the tower's damage.</summary>
			double damage_change;
			/// <summary>The % change in the tower's firing speed.</summary>
			double speed_change;
			/// <summary>The % change in the tower's firing range.</summary>
			double range_change;
			/// <summary>The % change in the tower's volley shots.</summary>
			double ammo_change;
			/// <summary>The % change in the tower's reload delay.</summary>
			double delay_change;
		};

		/// <summary>The class for all towers in the game, including walls.</summary>
		class TowerType : public GameObjectType {
		public:
			/*
			// Friends
			friend class winapi::TowerInfoDialog;
			friend class winapi::TowerUpgradeInfoDialog;
			*/
			// Constructor
			TowerType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				std::shared_ptr<FiringMethod> fmethod, std::shared_ptr<TargetingStrategy> tstrategy,
				std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>&& stypes,
				double fs, double fr, int vs, int rd, int cost, int max_lv) :
				GameObjectType {n, d, c, st},
				firing_method {fmethod},
				targeting_strategy {tstrategy},
				shot_types {stypes},
				firing_speed {fs},
				firing_range {fr},
				volley_shots {vs},
				reload_delay {rd},
				cost_adjustment {cost},
				max_level {max_lv} {
			}

			// Setters/Changers:
			void addUpgradeInfo(TowerUpgradeInfo upgrade_info) noexcept {
				this->upgrades.emplace_back(upgrade_info);
			}

			// Getters and similar
			std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>> getShotTypes() const noexcept {
				return this->shot_types;
			}
			const FiringMethod& getFiringMethod() const noexcept {
				return *this->firing_method;
			}
			const TargetingStrategy& getTargetingStrategy() const noexcept {
				return *this->targeting_strategy;
			}
			double getFiringSpeed() const noexcept {
				return this->firing_speed;
			}
			double getFiringRange() const noexcept {
				return this->firing_range;
			}
			/// <returns>The approximate area covered by the tower.</returns>
			double getFiringArea() const noexcept {
				return towers::getFiringArea(this->getFiringRange(), this->getFiringMethod(), this->isWall());
			}
			int getVolleyShots() const noexcept {
				return this->volley_shots;
			}
			int getReloadDelay() const noexcept {
				return this->reload_delay;
			}
			/// <returns>True if this "tower" is a wall; otherwise, false.</returns>
			virtual bool isWall() const noexcept {
				return false;
			}
			int getCostAdjustment() const noexcept {
				return this->cost_adjustment;
			}
			int getMaxLevel() const noexcept {
				return this->max_level;
			}
			const std::vector<TowerUpgradeInfo>& getUpgrades() const noexcept {
				return this->upgrades;
			}
			// Note: For the most part, all of these should be very close to the same
			//       as the versions in Tower.
			/// <returns>The expected amount of raw damage output by the tower per shot on average.</returns>
			double getAverageDamagePerShot() const noexcept {
				return towers::getAverageDamagePerShot(this->getShotTypes(), 1.0);
			}
			/// <returns>The weighted average effect rating of the tower's shot types.</returns>
			double getAverageShotEffectRating() const noexcept {
				return towers::getAverageShotEffectRating(this->getShotTypes());
			}
			/// <returns>The weighted average rating of the tower's shot types.</returns>
			double getAverageShotRating() const noexcept {
				return towers::getAverageShotRating(this->getShotTypes());
			}
			/// <returns>The tower's average rate of fire as the average number of shots fired per second.</returns>
			double getRateOfFire() const noexcept {
				return towers::getRateOfFire(this->getFiringSpeed(), this->getVolleyShots(), this->getReloadDelay(), this->isWall());
			}
			/// <returns>The tower's expected DPS.</returns>
			double getExpectedDPS() const noexcept {
				return towers::getExpectedDPS(this->getAverageDamagePerShot(), this->getRateOfFire());
			}
			/// <returns>The tower's overall rating.</returns>
			double getRating() const noexcept {
				return towers::getRating(this->getRateOfFire(), this->getFiringRange(), this->getFiringArea(),
					this->getFiringMethod(), this->getTargetingStrategy(),
					this->getAverageDamagePerShot(), this->getAverageShotEffectRating(), this->isWall());
			}
			/// <returns>The cost of the tower.</returns>
			double getCost() const noexcept {
				return towers::getCost(this->getRating(), this->getCostAdjustment(), this->isWall());
			}
		protected:
		private:
			/// <summary>The firing method used by the tower.</summary>
			std::shared_ptr<FiringMethod> firing_method;
			/// <summary>The targeting strategy used by the tower.</summary>
			std::shared_ptr<TargetingStrategy> targeting_strategy;
			/// <summary>A list of the shots that can be fired that this tower
			/// alongside the frequency with which the tower fires each projectile.</summary>
			std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>> shot_types;
			/// <summary>The rate at which the tower creates new projectiles.
			/// (Given in number of shots per second.)</summary>
			double firing_speed;
			/// <summary>The maximum distance from the tower that projectiles
			/// will travel to and that the tower will consider when seeking
			/// targets. (Units are game coordinate squares.)</summary>
			double firing_range;
			/// <summary>The number of shots the tower can fire before it has
			/// to reload.</summary>
			int volley_shots;
			/// <summary>The amount of time (in milliseconds) that it takes
			/// for the tower to reload.</summary>
			int reload_delay;
			/// <summary>The amount by which the tower's cost is adjusted.</summary>
			int cost_adjustment;
			// Upgrade-related stuff:
			/// <summary>The maximum number of times this tower can be upgraded.</summary>
			int max_level;
			/// <summary>Stores the tower's upgrade progression data.</summary>
			std::vector<TowerUpgradeInfo> upgrades {};
		};

		// The inheritance scheme is more for simplicity/convenience
		// versus really being true logically.
		/// <summary>The class used for walls in the game.</summary>
		class WallType : public TowerType {
		public:
			WallType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st, int cost) :
				TowerType {n, d, c, st, nullptr, nullptr, {}, 0.0, 0.0, 0, 0, cost, 1} {
			}
			// Overrides TowerType::isWall()
			bool isWall() const noexcept final {
				return true;
			}
		};
	}
}