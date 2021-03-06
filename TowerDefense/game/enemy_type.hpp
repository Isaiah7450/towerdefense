#pragma once
// File Author: Isaiah Hoffman
// File Created: April 18, 2018
#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <cmath>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./game_object_type.hpp"
#include "./game_formulas.hpp"

namespace hoffman_isaiah {
	namespace game {
		// Forward declarations:
		class Enemy;

		/// <summary>Enumeration of valid types of buffs.</summary>
		enum class BuffTypes {
			Intelligence, Speed, Healer, Purify, Repair, Forcefield
		};

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
				frames_between_buff_ticks {math::convertMillisecondsToFrames(ms_ticks)} {
			}
			virtual ~BuffBase() noexcept = default;
			// Rule of 5:
			BuffBase(const BuffBase&) = default;
			BuffBase(BuffBase&&) noexcept = default;
			BuffBase& operator=(const BuffBase&) = default;
			BuffBase& operator=(BuffBase&&) noexcept = default;

			/// <returns>Returns the buff's type. (Useful for switch statements.)</returns>
			virtual BuffTypes getType() const noexcept = 0;
			// Generally speaking, this should be the same as the enumration constant
			// with underscores replaced by spaces.
			/// <returns>Returns the name of the buff as it would appear in user interfaces.</returns>
			virtual std::wstring getName() const noexcept = 0;
			/// <summary>Updates the state of the buff by one logic frame.</summary>
			/// <param name="caller">The enemy that naturally radiates the buff.</param>
			/// <param name="enemies">All of the enemies in the game.</param>
			void update(const Enemy& caller, std::vector<std::unique_ptr<Enemy>>& enemies);

			// Getters
			std::vector<std::wstring> getTargetNames() const noexcept {
				return this->buff_names;
			}
			double getRadius() const noexcept {
				return this->buff_radius;
			}
			/// <returns>The time (in milliseconds) between applications of the buff.</returns>
			double getTimeBetweenApplications() const noexcept {
				return this->frames_between_buff_ticks * math::get_milliseconds_per_frame();
			}
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			virtual double getRating() const noexcept = 0;
		protected:
			/// <summary>This function is called for each enemy that the buff
			/// applies to when it is activated.</summary>
			/// <param name="target">The target of the buff.</param>
			virtual void apply(Enemy& target) = 0;
			/// <returns>True if it is time for the buff to be applied; otherwise, false.</returns>
			bool isTimeToApply() noexcept {
				if (this->frames_since_last_tick >= this->frames_between_buff_ticks) {
					return true;
				}
				return false;
			}
			/// <returns>The average base rating of the enemies this buff affects.</returns>
			double getAverageInfluenceRating() const noexcept;
			/// <returns>The basic rating of the buff as determined by its
			/// radius of influence, time between activations, and the number of enemies it affects.</returns>
			virtual double getBaseRating() const noexcept {
				return enemy_buffs::buff_base::getBaseRating(this->getRadius(), this->getTimeBetweenApplications(),
					this->getTargetNames().size(), this->getAverageInfluenceRating());
			}
			/// <param name="caller">The invoker of the buff.</param>
			/// <param name="target">The potential target of the buff.</param>
			/// <returns>True if the given target is a valid target for the buff.</returns>
			bool isValidTarget(const Enemy& caller, const Enemy& target) const;
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
				return enemy_buffs::temp_buff_base::getBaseRating(BuffBase::getBaseRating(), this->getBuffDuration());
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
			// Implements BuffBase::getType()
			BuffTypes getType() const noexcept override {
				return BuffTypes::Intelligence;
			}
			// Implements BuffBase::getName()
			std::wstring getName() const noexcept override {
				return L"Intelligence";
			}
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return enemy_buffs::smart_buff::getRating(this->getBaseRating());
			}
		protected:
			// Implements BuffBase::apply()
			void apply(Enemy& target) override;
		private:
		};

		/// <summary>Class that represents a buff where an enemy's speed is is increased.</summary>
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

			// Implements BuffBase::getType()
			BuffTypes getType() const noexcept override {
				return BuffTypes::Speed;
			}
			// Implements BuffBase::getName()
			std::wstring getName() const noexcept override {
				return L"Speed";
			}
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return enemy_buffs::speed_buff::getRating(this->getBaseRating(), this->getWalkingBoost(),
					this->getRunningBoost(), this->getInjuredBoost());
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
		protected:
			// Implements BuffBase::apply()
			void apply(Enemy& target) override;
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

		/// <summary>Class that represents a buff where the enemy heals
		/// applicable enemies by a static amount every so often.</summary>
		class HealerBuff : public BuffBase {
		public:
			/// <param name="heal_amt">The amount of HP healed with each tick of the buff.</param>
			HealerBuff(std::vector<std::wstring> target_names, double br, int ms_ticks,
				double heal_amt) :
				BuffBase {target_names, br, ms_ticks},
				heal_amount {heal_amt} {
				// Note that heal amount should be positive
			}
			// Implements BuffBase::getType()
			BuffTypes getType() const noexcept override {
				return BuffTypes::Healer;
			}
			// Implements BuffBase::heal()
			std::wstring getName() const noexcept override {
				return L"Healing";
			}
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return enemy_buffs::healer_buff::getRating(this->getBaseRating(), this->getHealAmount());
			}
			// Getters
			double getHealAmount() const noexcept {
				return this->heal_amount;
			}
		protected:
			// Implements BuffBase::apply()
			void apply(Enemy& target) override;
		private:
			/// <summary>The amount that surrounding enemies are healed by each turn.</summary>
			double heal_amount;
		};

		/// <summary>Class that represents a buff that purifies an enemy of
		/// negative status effects.</summary>
		class PurifyBuff : public BuffBase {
		public:
			/// <param name="cure_max">This buff will cure a maximum of this number of non-beneficial effects.</param>
			PurifyBuff(std::vector<std::wstring> target_names, double br, int ms_ticks, int cure_max) :
				BuffBase {target_names, br, ms_ticks},
				max_cured {cure_max} {
			}
			// Implements BuffBase::getType()
			BuffTypes getType() const noexcept override {
				return BuffTypes::Purify;
			}
			// Implements BuffBase::getName()
			std::wstring getName() const noexcept override {
				return L"Cleansing";
			}
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return enemy_buffs::purify_buff::getRating(this->getBaseRating(), this->getMaxEffectsRemoved());
			}
			// Getters
			int getMaxEffectsRemoved() const noexcept {
				return this->max_cured;
			}
		protected:
			// Implements BuffBase::apply()
			void apply(Enemy& target) override;
		private:
			/// <summary>The maximum number of status effects cured.</summary>
			int max_cured;
		};

		/// <summary>Class that represents a buff where the enemy repairs armor of
		/// applicable enemies by a static amount every so often.</summary>
		class RepairBuff : public BuffBase {
		public:
			/// <param name="repair_amt">The amount of HP healed with each tick of the buff.</param>
			RepairBuff(std::vector<std::wstring> target_names, double br, int ms_ticks,
				double repair_amt) :
				BuffBase {target_names, br, ms_ticks},
				repair_amount {repair_amt} {
				// Note that heal amount should be positive
			}
			// Implements BuffBase::getType()
			BuffTypes getType() const noexcept override {
				return BuffTypes::Repair;
			}
			// Implements BuffBase::heal()
			std::wstring getName() const noexcept override {
				return L"Repairing";
			}
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return enemy_buffs::repair_buff::getRating(this->getBaseRating(), this->getRepairAmount());
			}
			// Getters
			double getRepairAmount() const noexcept {
				return this->repair_amount;
			}
		protected:
			// Implements BuffBase::apply()
			void apply(Enemy& target) override;
		private:
			/// <summary>The amount that surrounding enemies' armor are repaired by each turn.</summary>
			double repair_amount;
		};

		/// <summary>Class that represents a buff where incoming damage is reduced for awhile.</summary>
		class ForcefieldBuff : public TemporaryBuffBase {
		public:
			/// <param name="shp">The amount of health the generated shield possesses.</param>
			/// <param name="sa">The amount of damage absorbed by the shield.</param>
			ForcefieldBuff(std::vector<std::wstring> target_names, double br, int ms_ticks, int bd,
				double shp, double sa) :
				TemporaryBuffBase {target_names, br, ms_ticks, bd},
				shield_health {shp},
				shield_absorb {sa} {
			}
			// Implements BuffBase::getRating()
			double getRating() const noexcept override {
				return enemy_buffs::forcefield_buff::getRating(this->getBaseRating(), this->getShieldHealth(), this->getShieldAbsorb());
			}
			// Implements BuffBase::getType()
			BuffTypes getType() const noexcept override {
				return BuffTypes::Forcefield;
			}
			// Implements BuffBase::getName()
			std::wstring getName() const noexcept override {
				return L"Forcefield";
			}
			// Getters
			double getShieldHealth() const noexcept {
				return this->shield_health;
			}
			double getShieldAbsorb() const noexcept {
				return this->shield_absorb;
			}
		protected:
			// Implements BuffBase::apply()
			void apply(Enemy& target) override;
		private:
			/// <summary>The amount of health the generated shield possesses.</summary>
			double shield_health;
			/// <summary>The amount of damage the shield absorbs directly.</summary>
			double shield_absorb;
		};

		/// <summary>Template type used to create new enemies.</summary>
		class EnemyType : public GameObjectType {
		public:
			// Constructor
			EnemyType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				int dmg, double base_hp, double base_ahp, double ar, double pt, double base_wspd, double base_rspd,
				double base_ispd, pathfinding::HeuristicStrategies def_strat, bool diag, bool fly, bool unique,
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
				unique_class {unique},
				buff_types {btypes} {
			}
			// Getters
			int getDamage() const noexcept {
				return this->damage;
			}
			double getBaseHealth() const noexcept {
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
			bool isUnique() const noexcept {
				return this->unique_class;
			}
			const std::vector<std::shared_ptr<BuffBase>>& getBuffTypes() const noexcept {
				return this->buff_types;
			}
			int getBuffTypesCount() const noexcept {
				return static_cast<int>(this->getBuffTypes().size());
			}
			// A bunch of statistical information
			/// <returns>The effective armor health of the enemy, which is an estimate of how much
			/// damage the enemy can withstand before their armor is rendered ineffective.</returns>
			double getEffectiveArmorHealth() const noexcept {
				return enemies::getEffectiveArmorHealth(this->getBaseArmorHP(), this->getArmorReduce());
			}
			/// <returns>The effective health of the enemy, which is an estimate of how much
			/// damage the enemy can withstand before dying (ignoring piercing).</returns>
			double getEffectiveHealth() const noexcept {
				return enemies::getEffectiveHealth(this->getBaseHealth(), this->getEffectiveArmorHealth());
			}
			/// <returns>The proportion of the enemy's lifespan that the enemy spends walking
			/// (expressed as a decimal, not a %).</returns>
			double getWalkingPercent() const noexcept {
				return enemies::getWalkingPercent(this->getRunningPercent(), this->getInjuredPercent());
			}
			/// <returns>The proportion of the enemy's lifespan that the enemy spends running
			/// (expressed as a decimal, not a %).</returns>
			double getRunningPercent() const noexcept {
				return enemies::getRunningPercent(this->getInjuredPercent(), this->getBaseHealth(), this->getEffectiveHealth());
			}
			/// <returns>The proportion of the enemy's lifespan that the enemy spends injured
			/// (expressed as a decimal, not a %).</returns>
			double getInjuredPercent() const noexcept {
				return enemies::getInjuredPercent(this->getBaseHealth(), this->getEffectiveHealth(), this->getPainTolerance());
			}
			/// <returns>A rating value that factors in several of the enemy's core statistics.
			/// This does not factor in special abilities like buffs.</returns>
			double getBaseRating() const noexcept {
				return enemies::getBaseRating(this->getBaseWalkingSpeed(), this->getBaseRunningSpeed(), this->getBaseInjuredSpeed(),
					this->getWalkingPercent(), this->getRunningPercent(), this->getInjuredPercent(), this->getEffectiveHealth(),
					this->getDiagonalMultiplier(), this->getFlyingMultiplier(), this->getDamageMultiplier());
			}
			/// <returns>Another rating that focuses primarily on the enemy's special abilities.</returns>
			double getExtraRating() const noexcept {
				return enemies::getExtraRating(this->getBuffTypes());
			}
			/// <returns>Returns the enemy's full rating.</returns>
			double getRating() const noexcept {
				return enemies::getRating(this->getBaseRating(), this->getExtraRating());
			}
		protected:
			/// <returns>The impact that the amount of damage an enemy can potentially deal has on
			/// an enemy's rating.</returns>
			double getDamageMultiplier() const noexcept {
				return enemies::getDamageMultiplier(this->getDamage());
			}
			/// <returns>The impact that being able to move diagonally has on an enemy's rating.</returns>
			double getDiagonalMultiplier() const noexcept {
				return enemies::getDiagonalMultiplier(this->canMoveDiagonally());
			}
			/// <returns>The impact that flying has on an enemy's rating.</returns>
			double getFlyingMultiplier() const noexcept {
				return enemies::getFlyingMultiplier(this->isFlying());
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
			/// <summary>Is this enemy a unique enemy?</summary>
			bool unique_class;
			/// <summary>List of buffs that enemies of this type possess.</summary>
			std::vector<std::shared_ptr<BuffBase>> buff_types;
		};
	}
}