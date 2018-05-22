#pragma once
// File Author: Isaiah Hoffman
// File Created: April 18, 2018
#include <string>
#include <vector>
#include <memory>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./game_object_type.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations:
		class Enemy;

		// Note that any functions declared here that
		// are not implemented here will be implemented
		// in enemy.cpp
		// Also, note that all buffs are passive
		/// <summary>Base class for all buffs.</summary>
		class BuffBase {
		public:
			/// <param name="target_names">List of enemy names that are valid targets for the buff.</param>
			/// <param name="br">The radius in which the buff applies.</param>
			/// <param name="ms_ticks">The number of milliseconds between ticks.</param>
			BuffBase(std::vector<std::wstring> target_names, double br, int ms_ticks) :
				buff_names {target_names},
				buff_radius {br},
				frames_between_buff_ticks {math::convert_milliseconds_to_frames(ms_ticks)} {
			}
			virtual ~BuffBase() noexcept;
			/// <returns>Returns the name of the buff as
			/// it would appear in user interfaces.</returns>
			virtual std::wstring getName() const noexcept = 0;
			/// <summary>Called once each turn. It applies
			/// the buff to all applicable enemies.</summary>
			/// <param name="caller">The enemy that activated the buff.</param>
			/// <param name="enemies">All of the enemies in the game.</param>
			virtual void apply(Enemy& caller, std::vector<std::unique_ptr<Enemy>>& enemies) = 0;

			// Getters
			std::vector<std::wstring> getTargetNames() const noexcept {
				return this->buff_names;
			}
			double getRadius() const noexcept {
				return this->buff_radius;
			}
			double getMillisecondsBetweenApplications() const noexcept {
				return this->frames_between_buff_ticks * math::get_milliseconds_per_frame();
			}
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			virtual double getRating() const noexcept = 0;
		protected:
			/// <param name="caller">The invoker of the buff.</param>
			/// <param name="target">The potential target of the buff.</param>
			/// <returns>True if the given target is a valid target for the buff.</returns>
			bool isValidTarget(const Enemy& caller, const Enemy& target) const;
			/// <summary>This updates the buff's status and checks if it can be applied again.</summary>
			/// <returns>True if the buff can be applied now.</returns>
			bool updateAndCheckApply() noexcept {
				++this->frames_since_last_tick;
				if (this->frames_since_last_tick >= this->frames_between_buff_ticks) {
					this->frames_since_last_tick -= this->frames_between_buff_ticks;
					return true;
				}
				return false;
			}
			/// <returns>The basic rating of the buff as determined by its
			/// radius of influence, time between activations, and the number of enemies it affects.</returns>
			virtual double getBaseRating() const noexcept {
				return this->getTargetNames().size() * this->getRadius() * this->getRadius() * math::pi
					/ (this->getMillisecondsBetweenApplications() / 1000.0);
			}
		private:
			/// <summary>The names of enemies that are affected by the buff.</summary>
			std::vector<std::wstring> buff_names;
			/// <summary>The maximum distance (in game coordinate squares) that the buff covers.</summary>
			double buff_radius;
			/// <summary>The number of logical frames between each application of the buff.</summary>
			double frames_between_buff_ticks;
			/// <summary>The number of frames that have passed since the last application of the buff.</summary>
			double frames_since_last_tick {0.0};
		};

		/// <summary>Base class for all buffs of a temporary nature.</summary>
		class TemporaryBuffBase : public BuffBase {
		public:
			/// <param name="bd">Duration of the buff in milliseconds.</param>
			TemporaryBuffBase(std::vector<std::wstring> target_names, double br, int ms_ticks, int bd) :
				BuffBase {target_names, br, ms_ticks},
				buff_duration {bd} {
			}
			int getBuffDuration() const noexcept {
				return this->buff_duration;
			}
		protected:
			// Overrides BuffBase::getBaseRating()
			double getBaseRating() const noexcept override {
				return BuffBase::getBaseRating() * (this->getBuffDuration() / 1000.0);
			}
		private:
			/// <summary>The duration of the buff in milliseconds.</summary>
			int buff_duration;
		};

		/// <summary>Class that represents a buff that increases the intelligence of
		/// surrounding enemies.</summary>
		class SmartBuff : public TemporaryBuffBase {
		public:
			SmartBuff(std::vector<std::wstring> target_names, double br, int ms_ticks, int bd) :
				TemporaryBuffBase {target_names, br, ms_ticks, bd} {
			}
			// Implements BuffBase::getName()
			std::wstring getName() const noexcept override {
				return L"Intelligence";
			}
			// Implements BuffBase::apply()
			void apply(Enemy& e, std::vector<std::unique_ptr<Enemy>>& enemies) override;
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return this->getBaseRating() * math::get_sqrt(2.0);
			}
		private:
		};

		class SpeedBuff : public TemporaryBuffBase {
		public:
			/// <param name="wb">The percentage increase to the enemy's walking speed boost.</param>
			/// <param name="rb">The percentage increase to the enemy's running speed boost.</param>
			/// <param name="ib">The percentage increase to the enemy's injured speed boost.</param>
			SpeedBuff(std::vector<std::wstring> target_names, double br, int ms_ticks, int bd,
				double wb, double rb, double ib) :
				TemporaryBuffBase {target_names, br, ms_ticks, bd},
				walking_boost {wb},
				running_boost {rb},
				injured_boost {ib} {
			}

			// Implements BuffBase::getName()
			std::wstring getName() const noexcept override {
				return L"Speed";
			}
			// Implements BuffBase::apply()
			void apply(Enemy& caller, std::vector<std::unique_ptr<Enemy>>& enemies) override;
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return this->getBaseRating() * (1.0 + this->getWalkingBoost() * 0.5
					+ this->getRunningBoost() * 0.3 + this->getInjuredBoost() * 0.2);
			}
			// Getters
			double getWalkingBoost() const noexcept {
				return this->walking_boost;
			}
			double getRunningBoost() const noexcept {
				return this->running_boost;
			}
			double getInjuredBoost() const noexcept {
				return this->injured_boost;
			}
		private:
			/// <summary>The percentage increase to the enemy's walking speed boost
			/// (expressed as a decimal).</summary>
			double walking_boost;
			/// <summary>The percentage increase to the enemy's running speed boost
			/// (expressed as a decimal).</summary>
			double running_boost;
			/// <summary>The percentage increase to the enemy's injured speed boost
			/// (expressed as a decimal).</summary>
			double injured_boost;
		};

		/// <summary>Template type used to create new enemies.</summary>
		class EnemyType : public GameObjectType {
		public:
			// Constructor
			EnemyType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				int dmg, double base_hp, double base_ahp, double ar, double pt, double base_wspd, double base_rspd,
				double base_ispd, pathfinding::HeuristicStrategies def_strat, bool diag, bool fly,
				std::vector<std::shared_ptr<BuffBase>> btypes = {}) :
				GameObjectType {n, d, c, st},
				damage {dmg},
				base_health {base_hp},
				base_armor_hp {base_ahp},
				armor_reduce {ar},
				pain_tolerance {pt},
				walking_speed {base_wspd},
				running_speed {base_rspd},
				injured_speed {base_ispd},
				default_strategy {def_strat},
				move_diag {diag},
				flying {fly},
				buff_types {btypes} {
			}
			// Getters
			int getDamage() const noexcept {
				return this->damage;
			}
			double getBaseHP() const noexcept {
				return this->base_health;
			}
			double getBaseArmorHP() const noexcept {
				return this->base_armor_hp;
			}
			double getArmorReduce() const noexcept {
				return this->armor_reduce;
			}
			/// <returns>The percentage of damage (to the enemy's health) that the
			/// enemy can receive before being considered 'injured'.</summary>
			double getPainTolerance() const noexcept {
				return this->pain_tolerance;
			}
			double getBaseWalkingSpeed() const noexcept {
				return this->walking_speed;
			}
			double getBaseRunningSpeed() const noexcept {
				return this->running_speed;
			}
			double getBaseInjuredSpeed() const noexcept {
				return this->injured_speed;
			}
			pathfinding::HeuristicStrategies getDefaultStrategy() const noexcept {
				return this->default_strategy;
			}
			bool canMoveDiagonally() const noexcept {
				return this->move_diag;
			}
			bool isFlying() const noexcept {
				return this->flying;
			}
			const std::vector<std::shared_ptr<BuffBase>>& getBuffTypes() const noexcept {
				return this->buff_types;
			}
			// A bunch of statistical information
			/// <returns>The effective armor health of the enemy, which is an estimate of how much
			/// damage the enemy can withstand before their armor is rendered ineffective.</returns>
			double getEffectiveArmorHealth() const noexcept {
				return this->getBaseArmorHP() * (1.0 / (1.0 - this->getArmorReduce()));
			}
			/// <returns>The effective health of the enemy, which is an estimate of how much
			/// damage the enemy can withstand before dying.</returns>
			double getEffectiveHealth() const noexcept {
				return this->getBaseArmorHP() + this->getBaseHP();
			}
			/// <returns>The proportion of the enemy's lifespan that the enemy spends walking
			/// (expressed as a decimal, not a %).</returns>
			double getWalkingPercent() const noexcept {
				return this->getBaseHP() * this->getPainTolerance() / this->getEffectiveHealth();
			}
			/// <returns>The proportion of the enemy's lifespan that the enemy spends running
			/// (expressed as a decimal, not a %).</returns>
			double getRunningPercent() const noexcept {
				return this->getEffectiveArmorHealth() / this->getEffectiveHealth();
			}
			/// <returns>The proportion of the enemy's lifespan that the enemy spends injured
			/// (expressed as a decimal, not a %).</returns>
			double getInjuredPercent() const noexcept {
				return this->getBaseHP() * (1.0 - this->getPainTolerance()) / this->getEffectiveHealth();
			}
			/// <returns>A rating value that factors in several of the enemy's core statistics.
			/// This does not factor in special abilities like buffs.</returns>
			double getBaseRating() const noexcept {
				// Basically, these are the number of tiles an enemy will cross if they take 1 damage
				// for each tile they cross.
				const double tiles_while_walking = this->getWalkingPercent() * this->getBaseWalkingSpeed()
					* this->getEffectiveHealth();
				const double tiles_while_running = this->getRunningPercent() * this->getBaseRunningSpeed()
					* this->getEffectiveHealth();
				const double tiles_while_injured = this->getInjuredPercent() * this->getBaseInjuredSpeed()
					* this->getEffectiveHealth();
				// The final rating is equivalent to the total number of tiles travelled across the
				// enemy's entire lifespan (under the assumption of receiving 1 damage per tile)
				// multiplied by the damage, flying, and diagonal movement multipliers.
				return (tiles_while_walking + tiles_while_running + tiles_while_injured)
					* this->getDiagonalMultiplier() * this->getFlyingMultiplier();
			}
			/// <returns>Another rating that focuses primarily on the enemy's special abilities.</returns>
			double getExtraRating() const noexcept {
				double multiplier = 0.0;
				for (const auto& b : this->buff_types) {
					multiplier += b->getRating();
				}
				return multiplier * this->getBaseRating() * 0.05;
			}
			/// <returns>Returns the enemy's full rating.</returns>
			double getRating() const noexcept {
				return this->getBaseRating() + this->getExtraRating();
			}
		protected:
			/// <returns>The impact that the amount of damage an enemy can potentially deal has on
			/// an enemy's rating.</returns>
			double getDamageMultiplier() const noexcept {
				return math::get_sqrt(this->getDamage());
			}
			/// <returns>The impact that being able to move diagonally has on an enemy's rating.</returns>
			double getDiagonalMultiplier() const noexcept {
				return this->canMoveDiagonally() ? 1.15 : 1.00;
			}
			/// <returns>The impact that flying has on an enemy's rating.</returns>
			double getFlyingMultiplier() const noexcept {
				return this->isFlying() ? 1.08 : 1.00;
			}
		private:
			/// <summary>The amount of damage the enemy deals to the player if it reaches the goal.</summary>
			int damage;
			/// <summary>The amount of damage that the enemy can withstand before perishing.</summary>
			double base_health;
			/// <summary>The amount of damage that the enemy's armor can withstand before breaking.</summary>
			double base_armor_hp;
			/// <summary>The percentage of damage that the enemy's armor blocks (expressed as a decimal, not a %).</summary>
			double armor_reduce;
			/// <summary>The percentage of damage (to the enemy's health) can receive before being
			/// considered 'injured' (expressed as a decimal, not a %).</summary>
			double pain_tolerance;
			/// <summary>The speed in game coordinate squares per second that the enemy moves while wearing armor.</summary>
			double walking_speed;
			/// <summary>The speed in game coordinate squares per second that the enemy moves while not wearing armor.</summary>
			double running_speed;
			/// <summary>The speed in game coordinate squares per second that the enemy moves while injured.</summary>
			double injured_speed;
			/// <summary>The pathfinding heuristic the enemy uses.</summary>
			pathfinding::HeuristicStrategies default_strategy;
			/// <summary>Can the enemy move diagonally?</summary>
			bool move_diag;
			/// <summary>Is the enemy a flying enemy?</summary>
			bool flying;
			/// <summary>List of buffs that enemies of this type possess.</summary>
			std::vector<std::shared_ptr<BuffBase>> buff_types;
		};
	}
}