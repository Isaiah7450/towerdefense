#pragma once
// File Author: Isaiah Hoffman
// File Created: April 18, 2018
#include <string>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./game_object_type.hpp"

namespace hoffman::isaiah {
	namespace game {
		/// <summary>Template type used to create new enemies.</summary>
		class EnemyType : public GameObjectType {
		public:
			// Constructor
			EnemyType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				int dmg, double base_hp, double base_ahp, double ar, double pt, double base_wspd, double base_rspd,
				double base_ispd, pathfinding::HeuristicStrategies def_strat, bool diag, bool fly) :
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
				flying {fly} {
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
				// TODO: Implement this
				return 0;
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
			// Buff Parameters --> To do later!

		};
	}
}