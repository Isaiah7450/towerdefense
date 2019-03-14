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
				const GameMap& gmap, int level, double difficulty, int challenge_level);
			Enemy(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
				std::shared_ptr<EnemyType> etype, graphics::Color o_color,
				pathfinding::Pathfinder pf, double start_gx, double start_gy,
				int level, double difficulty, int challenge_level);
			/// <summary>Advances the enemy's game state by one frame.</summary>
			/// <returns>True if the enemy should be removed; otherwise, false.</returns>
			bool update();
			// Overrides GameObject::draw
			void draw(const graphics::Renderer2D& renderer) const noexcept;

			/// <summary>Deals damage to the enemy.</summary>
			/// <param name="dmg">The base amount of damage for the enemy to receive.</param>
			/// <param name="wap">The armor piercing value associated with the damage.</param>
			/// <param name="bypass_armor_completely">If true, no damage will be dealt to the enemy's
			/// armor.</param>
			void takeDamage(double dmg, double wap, bool bypass_armor_completely = false);
			/// <summary>Heals the enemy's hitpoints.</summary>
			/// <param name="amt">The amount of hitpoints to restore.</param>
			void heal(double amt);
			/// <summary>Restores the enemy's armor hitpoints.</summary>
			/// <param name="amt">The amount of armor hitpoints to restore.</param>
			void repair(double amt);
			/// <summary>Adds a status to the enemy.</summary>
			/// <param name="effect">The effect to add; uses move semantics.</param>
			void addStatus(std::unique_ptr<StatusEffectBase>&& effect);

			// Setters/Changers
			void multiplyWalkingBoost(double amt) noexcept {
				this->speed_boosts[0] = math::get_max(0.10, this->getWalkingSpeedBoost() * amt);
			}
			void multiplyRunningBoost(double amt) noexcept {
				this->speed_boosts[1] = math::get_max(0.10, this->getRunningSpeedBoost() * amt);
			}
			void multiplyInjuredBoost(double amt) noexcept {
				this->speed_boosts[2] = math::get_max(0.10, this->getInjuredSpeedBoost() * amt);
			}
			/// <param name="amt">Amount to multiply all of the
			/// enemies' speed boosts by (as a decimal percent).</param>
			void multiplyAllSpeedBoosts(double amt) noexcept {
				this->multiplyWalkingBoost(amt);
				this->multiplyRunningBoost(amt);
				this->multiplyInjuredBoost(amt);
			}
			void setSpeedMultiplier(double new_value) noexcept {
				// Minimum speed place for balance purposes
				this->speed_multiplier = math::get_max(new_value, 0.05);
			}
			void setStun(bool new_status) noexcept {
				this->stun_active = new_status;
			}
			void setDoTActive(bool new_status) noexcept {
				this->dot_active = new_status;
			}
			/// <summary>Changes the enemy's pathfinding strategy.</summary>
			/// <param name="gmap">Reference to the game's maps.</param>
			/// <param name="new_strat">The new heuristic estimation strategy to use.</param>
			/// <param name="diag_move">Whether the enemy is now allowed to move diagonally or not.</param>
			void changeStrategy(const GameMap& gmap,
				pathfinding::HeuristicStrategies new_strat, bool diag_move);
			/// <summary>Creates or extends a shield effect for the enemy.</summary>
			/// <param name="shp">The starting health of the shield.</param>
			/// <param name="sa">The damage absorption of the shield.</param>
			void addShield(double shp, double sa) noexcept {
				this->shield_health += shp;
				this->shield_max_health += shp;
				this->shield_absorb = math::get_min(sa, this->getShieldAbsorb());
			}
			/// <summary>Deals damage directly to the enemy's shield.</summary>
			/// <param name="dmg">The amount of damage to inflict.</param>
			void degradeShield(double dmg) noexcept {
				this->shield_health -= dmg;
				if (this->shield_health <= 0) {
					this->shield_absorb = 0;
					this->shield_health = 0;
					this->shield_max_health = 0;
				}
			}
			// Getters
			const EnemyType& getBaseType() const noexcept {
				return *this->base_type;
			}
			const pathfinding::GraphNode& getNextNode() const noexcept {
				return this->my_path.front();
			}
			// This function is mostly for caching paths since those don't change
			// too much during a level.
			/// <returns>A copy of the enemy's current path.</returns>
			std::queue<pathfinding::GraphNode> getPathCopy() const noexcept {
				return std::queue<pathfinding::GraphNode>(this->my_path);
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
			/// <returns>The multiplier applied solely to the enemy's walking speed.</returns>
			double getWalkingSpeedBoost() const noexcept {
				return this->speed_boosts[0];
			}
			/// <returns>The multiplier applied solely to the enemy's running speed.</returns>
			double getRunningSpeedBoost() const noexcept {
				return this->speed_boosts[1];
			}
			/// <returns>The multiplier applied solely to the enemy's injured speed.</returns>
			double getInjuredSpeedBoost() const noexcept {
				return this->speed_boosts[2];
			}
			/// <returns>The enemy's overall speed multiplier.</returns>
			double getSpeedMultiplier() const noexcept {
				return this->speed_multiplier;
			}
			/// <returns>True if the enemy is currently stunned.</returns>
			bool isStunned() const noexcept {
				return this->stun_active;
			}
			/// <returns>True if the enemy is currently slowed.</returns>
			bool isSlowed() const noexcept {
				return this->getSpeedMultiplier() < 1.0;
			}
			/// <returns>True if the enemy has a DoT effect that is active.</returns>
			bool hasActiveDoT() const noexcept {
				return this->dot_active;
			}
			/// <returns>The enemy's raw walking speed multiplied by their speed multiplier.</returns>
			double getCurrentWalkingSpeed() const noexcept {
				return this->getRawWalkingSpeed() * this->getSpeedMultiplier();
			}
			/// <returns>The enemy's raw running speed multiplied by their speed multiplier.</returns>
			double getCurrentRunningSpeed() const noexcept {
				return this->getRawRunningSpeed() * this->getSpeedMultiplier();
			}
			/// <returns>The enemy's raw injured speed multiplied by their speed multiplier.</returns>
			double getCurrentInjuredSpeed() const noexcept {
				return this->getRawInjuredSpeed() * this->getSpeedMultiplier();
			}
			/// <returns>Gets the current movement speed of the enemy, which factors in both
			/// the speed multiplier and the enemy's state.</returns>
			double getCurrentSpeed() const noexcept {
				return this->isInjured() ? this->getCurrentInjuredSpeed() :
					this->isRunning() ? this->getCurrentRunningSpeed() : this->getCurrentWalkingSpeed();
			}
			double getShieldHealth() const noexcept {
				return this->shield_health;
			}
			double getMaxShieldHealth() const noexcept {
				return this->shield_max_health;
			}
			double getShieldAbsorb() const noexcept {
				return this->shield_absorb;
			}
			pathfinding::HeuristicStrategies getCurrentStrategy() const noexcept {
				return this->current_strat;
			}
			bool canMoveDiagonally() const noexcept {
				return this->move_diagonally;
			}
			std::vector<std::unique_ptr<StatusEffectBase>>& getActiveStatuses() noexcept {
				return this->status_effects;
			}
			const std::vector<std::unique_ptr<StatusEffectBase>>& getActiveStatuses() const noexcept {
				return this->status_effects;
			}
		protected:
			// Functions that help construction
			/// <returns>The enemy's starting health adjusted for the level, difficulty, and challenge level.</returns>
			static double getAdjustedHealth(double base_hp, int level, double difficulty, int challenge_level) noexcept {
				return base_hp + math::get_max(0.0, 0.05 * (level - 1.0) * challenge_level + 0.001 * (difficulty - 1.0));
			}
			/// <returns>The enemy's starting armor health adjusted for the level, difficulty, and challenge level.</returns>
			static double getAdjustedArmorHealth(double base_ahp, int level, double difficulty, int challenge_level) noexcept {
				return base_ahp > 0 ? math::get_min(base_ahp * (challenge_level + 2.0) + level,
					base_ahp + ((level - 1.0) * 0.05) * ((difficulty + challenge_level) / 12.5 + 0.84)) : 0;
			}
			/// <returns>The enemy's starting speed adjusted for the level, difficulty, and challenge level.</returns>
			static double getAdjustedSpeed(double base_speed, int level, double difficulty, int challenge_level) noexcept {
				return base_speed + math::get_min(base_speed / 2.0, (level - 1) * challenge_level * 0.001)
					+ math::get_max(0.0, math::get_min(base_speed / 2.0, (difficulty - 1.0) * 0.0025));
			}
			/// <summary>Creates the enemy's actual buffs from the templates stored in its type.</summary>
			void addEnemyBuffs() noexcept {
				for (auto& b : this->getBaseType().getBuffTypes()) {
					// This is obnoxious, but I think it works better
					// than getting into some weird cloning stuff
					switch (b->getType()) {
					case BuffTypes::Intelligence:
						this->buffs.emplace_back(std::make_unique<SmartBuff>(dynamic_cast<SmartBuff&>(*b)));
						break;
					case BuffTypes::Speed:
						this->buffs.emplace_back(std::make_unique<SpeedBuff>(dynamic_cast<SpeedBuff&>(*b)));
						break;
					case BuffTypes::Healer:
						this->buffs.emplace_back(std::make_unique<HealerBuff>(dynamic_cast<HealerBuff&>(*b)));
						break;
					case BuffTypes::Purify:
						this->buffs.emplace_back(std::make_unique<PurifyBuff>(dynamic_cast<PurifyBuff&>(*b)));
						break;
					case BuffTypes::Repair:
						this->buffs.emplace_back(std::make_unique<RepairBuff>(dynamic_cast<RepairBuff&>(*b)));
						break;
					case BuffTypes::Forcefield:
						this->buffs.emplace_back(std::make_unique<ForcefieldBuff>(dynamic_cast<ForcefieldBuff&>(*b)));
						break;
					}
				}
			}
			// Setters/Changers
			// Getters
			/// <returns>The base walking speed of the enemy. (Base Speed).</returns>
			double getBaseWalkingSpeed() const noexcept {
				return this->walking_speed;
			}
			/// <returns>The base running speed of the enemy. (Base Speed).</returns>
			double getBaseRunningSpeed() const noexcept {
				return this->running_speed;
			}
			/// <returns>The base injured speed of the enemy. (Base Speed).</returns>
			double getBaseInjuredSpeed() const noexcept {
				return this->injured_speed;
			}
			/// <returns>The raw walking speed of the enemy. (Base Speed * Boosts)</returns>
			double getRawWalkingSpeed() const noexcept {
				return this->getBaseWalkingSpeed() * this->getWalkingSpeedBoost();
			}
			/// <returns>The raw running speed of the enemy. (Base Speed * Boosts)</returns>
			double getRawRunningSpeed() const noexcept {
				return this->getBaseRunningSpeed() * this->getRunningSpeedBoost();
			}
			/// <returns>The raw injured speed of the enemy. (Base Speed * Boosts)</returns>
			double getRawInjuredSpeed() const noexcept {
				return this->getBaseInjuredSpeed() * this->getInjuredSpeedBoost();
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
			/// <summary>Updates the direction that the enemy is taking.</summary>
			void changeDirection() noexcept;
		private:
			/// <summary>The template type used to create the enemy.</summary>
			std::shared_ptr<EnemyType> base_type;
			// Pathfinding stuff
			/// <summary>The pathfinder used by the enemy.</summary>
			pathfinding::Pathfinder my_pathfinder;
			/// <summary>The current path being taken by the enemy.</summary>
			std::queue<pathfinding::GraphNode> my_path;
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
			/// <summary>The enemy's base walking speed after considering difficulty modifiers.</summary>
			double walking_speed;
			/// <summary>The enemy's base running speed after considering difficulty modifiers.</summary>
			double running_speed;
			/// <summary>The enemy's base injured speed after considering difficulty modifiers.</summary>
			double injured_speed;
			/// <summary>The current pathfinding strategy being employed by the enemy.</summary>
			pathfinding::HeuristicStrategies current_strat;
			/// <summary>Can the enemy currently move diagonally?</summary>
			bool move_diagonally;
			/// <summary>The current speed boosts of the enemy. 0 => Walking, 1 => Running, 2 => Injured</summary>
			std::array<double, 3> speed_boosts {1.0, 1.0, 1.0};
			/// <summary>The current speed multiplier of the enemy.</summary>
			double speed_multiplier {1.0};
			/// <summary>Is a DoT effect active?</summary>
			bool dot_active {false};
			/// <summary>Is a stun effect active?</summary>
			bool stun_active {false};
			/// <summary>The amount of health in the shield until it expires.</summary>
			double shield_health {0.0};
			/// <summary>The maximum amount of damage the shield can withstand.</summary>
			double shield_max_health {0.0};
			/// <summary>The amount of damage absorbed directly by the shield.</summary>
			double shield_absorb {0.0};
			// Buffs
			/// <summary>The list of passive buffs that this enemy possesses.</summary>
			std::vector<std::unique_ptr<BuffBase>> buffs;
			// Status ailments
			/// <summary>The list of status effects that are currently affecting this enemy.</summary>
			std::vector<std::unique_ptr<StatusEffectBase>> status_effects;
		};
	}
}