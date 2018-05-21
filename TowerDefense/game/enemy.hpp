#pragma once
// File Author: Isaiah Hoffman
// File Created: May 15, 2018
#include <string>
#include <queue>
#include <array>
#include <vector>
#include <memory>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../pathfinding/graph_node.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./enemy_type.hpp"
#include "./game_object.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declaration
		class StatusEffectBase;

		/// <summary>Class that represents an actual enemy in the game.</summary>
		class Enemy : public GameObject {
		public:
			Enemy(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
				std::shared_ptr<EnemyType> etype, graphics::Color o_color,
				const GameMap& gmap, int level, int difficulty);
			/// <summary>Advances the enemy's game state by one frame.</summary>
			/// <returns>True if the enemy should be removed; otherwise, false.</returns>
			bool update();
			// Setters/Changers
			void changeWalkingSpeed(double amt) noexcept {
				this->current_speeds[0] += amt;
			}
			void changeRunningSpeed(double amt) noexcept {
				this->current_speeds[1] += amt;
			}
			void changeInjuredSpeed(double amt) noexcept {
				this->current_speeds[2] += amt;
			}
			void changeAllSpeeds(double amt) noexcept {
				this->changeWalkingSpeed(amt);
				this->changeRunningSpeed(amt);
				this->changeInjuredSpeed(amt);
			}
			void setSpeedMultiplier(double new_value) noexcept {
				this->speed_multiplier = new_value;
			}
			// Getters
			const EnemyType& getBaseType() const noexcept {
				return *this->base_type;
			}
			const pathfinding::GraphNode* getNextNode() const noexcept {
				return this->my_path.front();
			}
			// This function is mostly for caching paths since those don't change
			// too much during a level.
			/// <returns>A copy of the enemy's current path.</returns>
			std::queue<const pathfinding::GraphNode*> getPathCopy() const noexcept {
				return this->my_path;
			}
			double getHealth() const noexcept {
				return this->current_health;
			}
			double getMaxHealth() const noexcept {
				return this->maximum_health;
			}
			/// <returns>True if the enemy is still considered alive, otherwise false.</returns>
			bool isAlive() const noexcept {
				return this->getHealth() > 0.0;
			}
			/// <returns>The current percentage of the enemy's maximum health that it has remaining.
			/// (Expressed as a decimal, not a %.)</returns>
			double getHealthPercentage() const noexcept {
				return this->getHealth() / this->getMaxHealth();
			}
			double getArmorHealth() const noexcept {
				return this->current_armor_health;
			}
			double getMaxArmorHealth() const noexcept {
				return this->maximum_armor_health;
			}
			/// <returns>True if the enemy's armor is still intact, otherwise false.</returns>
			bool hasArmor() const noexcept {
				return this->getArmorHealth() > 0.0;
			}
			/// <returns>The current percentage of the enemy's armor that is still intact.
			/// (Expressed as a decimal, not a %.)</returns>
			double getArmorPercentage() const noexcept {
				return this->getArmorHealth() / this->getMaxArmorHealth();
			}
			double getSpeedMultiplier() const noexcept {
				return this->speed_multiplier;
			}
			/// <returns>The enemy's walking speed multiplied by their speed multiplier.</returns>
			double getCurrentWalkingSpeed() const noexcept {
				return this->getRawWalkingSpeed() * this->getSpeedMultiplier();
			}
			/// <returns>The enemy's running speed multiplied by their speed multiplier.</returns>
			double getCurrentRunningSpeed() const noexcept {
				return this->getRawRunningSpeed() * this->getSpeedMultiplier();
			}
			/// <returns>The enemy's injured speed multiplied by their speed multiplier.</returns>
			double getCurrentInjuredSpeed() const noexcept {
				return this->getRawInjuredSpeed() * this->getSpeedMultiplier();
			}
			/// <returns>Gets the current movement speed of the enemy, which factors in both
			/// the speed multiplier and the enemy's state.</returns>
			double getCurrentSpeed() const noexcept {
				return this->isInjured() ? this->getCurrentInjuredSpeed() :
					this->isRunning() ? this->getCurrentRunningSpeed() : this->getCurrentWalkingSpeed();
			}
		protected:
			// Setters/Changers
			// Getters
			/// <returns>The value stored in current_speeds[0].</returns>
			double getRawWalkingSpeed() const noexcept {
				return this->current_speeds[0];
			}
			/// <returns>The value stored in current_speeds[1].</returns>
			double getRawRunningSpeed() const noexcept {
				return this->current_speeds[1];
			}
			/// <returns>The value stored in current_speeds[2].</returns>
			double getRawInjuredSpeed() const noexcept {
				return this->current_speeds[2];
			}
			/// <returns>True if the enemy's health has reached a critical level
			/// (as defined by their pain tolerance).</returns>
			bool isInjured() const noexcept {
				return (1.0 - this->getHealthPercentage()) > this->getBaseType().getPainTolerance();
			}
			/// <returns>True if the enemy is currently walking.</returns>
			bool isWalking() const noexcept {
				return !this->isInjured() && this->hasArmor();
			}
			/// <returns>True if the enemy is currently running.</returns>
			bool isRunning() const noexcept {
				return !(this->isInjured() || this->hasArmor());
			}
		private:
			/// <summary>The template type used to create the enemy.</summary>
			std::shared_ptr<EnemyType> base_type;
			// Pathfinding stuff
			/// <summary>The pathfinder used by the enemy.</summary>
			pathfinding::Pathfinder my_pathfinder;
			/// <summary>The current path being taken by the enemy.</summary>
			std::queue<const pathfinding::GraphNode*> my_path;
			/// <summary>The last node in the path that the enemy travelled to.</summary>
			const pathfinding::GraphNode* current_node;
			/// <summary>The direction the enemy is currently moving in (in radians).</summary>
			double current_direction;
			// Information and statistics
			/// <summary>The amount of health that the enemy has remaining.</summary>
			double current_health;
			/// <summary>The maximum amount of damage the enemy can receive before dying.</summary>
			double maximum_health;
			/// <summary>The amount of armor hitpoints that the enemy has remaining.</summary>
			double current_armor_health;
			/// <summary>The maximum amount of damage to the enemy's armor that the enemy can
			/// sustain before the armor perishes.</summary>
			double maximum_armor_health;
			/// <summary>The current pathfinding strategy being employed by the enemy.</summary>
			pathfinding::HeuristicStrategies current_strat;
			/// <summary>The current speeds of the enemy. 0 => Walking, 1 => Running, 2 => Injured</summary>
			std::array<double, 3> current_speeds;
			/// <summary>The current speed multiplier of the enemy.</summary>
			double speed_multiplier;
			// Status ailments
			/// <summary>The list of status effects that are currently affecting this enemy.</summary>
			std::vector<std::unique_ptr<StatusEffectBase>> status_effects;
		};
	}
}