#pragma once
// File Author: Isaiah Hoffman
// File Created: June 2, 2018
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./game_object_type.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations
		class ShotBaseType;

		/// <summary>Enumeration constants that represents the methods that
		/// a tower uses to fire projectiles.</summary>
		enum class FiringMethodTypes {
			Default, Static, Pulse
		};
		/// <summary>Class that represents the firing method for a tower.</summary>
		class FiringMethod {
		public:
			FiringMethod(FiringMethodTypes m, std::vector<double> a = {}, int d = 0) :
				method {m},
				angles {a},
				duration {d} {
			}
			// Getters
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
			TargetingStrategy(TargetingStrategyTypes s, TargetingStrategyProtocols p,
				TargetingStrategyStatistics ts = TargetingStrategyStatistics::Not_Applicable) :
				strategy {s},
				protocol {p},
				test_stat {ts},
				target_names {} {
			}
			TargetingStrategy(TargetingStrategyTypes s, TargetingStrategyProtocols p,
				std::vector<std::wstring> tnames) :
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
		private:
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

		/// <summary>The class for all towers in the game, including walls.</summary>
		class TowerType : public GameObjectType {
		public:
			TowerType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				std::shared_ptr<FiringMethod> fmethod, std::shared_ptr<TargetingStrategy> tstrategy,
				std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>&& stypes,
				double fs, double fr, int vs, int rd, int cost) :
				GameObjectType {n, d, c, st},
				firing_method {fmethod},
				targeting_strategy {tstrategy},
				shot_types {stypes},
				firing_speed {fs},
				firing_range {fr},
				volley_shots {vs},
				reload_delay {rd},
				cost_adjustment {cost} {
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
				// Using the min-max angles for the tower is probably bad
				// but whatever; this is basically your formula for
				// the area of a circular sector (except in radians rather than degrees)
				return this->getFiringMethod().getMethod() == FiringMethodTypes::Default
					? this->getFiringRange() * this->getFiringRange() * math::pi
					: this->getFiringRange() * this->getFiringRange()
						* (this->getFiringMethod().getMaximumAngle() - this->getFiringMethod().getMinimumAngle()) / 2.0;
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
			/// <returns>The expected amount of raw damage output by the tower per shot on average.</returns>
			double getAverageDamagePerShot() const noexcept;
			/// <returns>The tower's overall rating.</returns>
			double getRating() const noexcept;
			/// <returns>The cost of the tower.</returns>
			double getCost() const noexcept {
				if (this->isWall()) {
					return static_cast<double>(this->cost_adjustment);
				}
				return this->getCostAdjustment() + this->getRating() / 18.0 + 1.0;
			}
		protected:
			// Getters
			int getCostAdjustment() const noexcept {
				return this->cost_adjustment;
			}
			/// <returns>The weighted average of the tower's shot types.</returns>
			double getAverageShotRating() const noexcept;
			/// <returns>The tower's average rate of fire as the average number of shots fired per second.</returns>
			double getRateOfFire() const noexcept {
				if (this->isWall()) {
					return 0;
				}
				if (this->getVolleyShots() == 0 || this->getReloadDelay() == 0) {
					return this->getFiringSpeed();
				}
				const double secs_to_reload = this->getReloadDelay() / 1000.0;
				// The average rate of fire is the total number of shots fired
				// in one cycle divided by the total time in that cycle where
				// one cycle is defined as firing an entire volley and reloading
				// that volley completely.
				const double total_cycle_time = (1.0 / this->getFiringSpeed())
					* this->getVolleyShots() + secs_to_reload;
				return static_cast<double>(this->getVolleyShots()) / total_cycle_time;
			}
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
		};

		// The inheritance scheme is more for simplicity/convenience
		// versus really being true logically.
		/// <summary>The class used for walls in the game.</summary>
		class WallType : public TowerType {
		public:
			WallType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st, int cost) :
				TowerType {n, d, c, st, nullptr, nullptr, {}, 0.0, 0.0, 0, 0, cost} {
			}
			// Overrides TowerType::isWall()
			bool isWall() const noexcept final {
				return true;
			}
		};
	}
}