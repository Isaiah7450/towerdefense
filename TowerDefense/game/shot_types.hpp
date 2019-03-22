#pragma once
// File Author: Isaiah Hoffman
// File Created: May 31, 2018
#include <array>
#include <string>
#include <cmath>
#include "./../ih_math.hpp"
#include "./../graphics/graphics.hpp"
#include "./game_object_type.hpp"
#include "./status_effects.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations
		class Enemy;

		// Polymorphism is the simplest and also the most sensible method
		// of implementing different kinds of effects for shots; applying
		// multiple effects to a single projectile type isn't something that
		// I think is really important or good from a balance standpoint.
		/// <summary>Enumeration constants that represent the various types of shots.</summary>
		enum class ShotTypes {
			Standard, DoT, Slow, Stun, Sentinel_DO_NOT_USE
		};
	}

	inline std::wstring operator*(game::ShotTypes stype) noexcept {
		constexpr const std::array<const wchar_t*,
			static_cast<int>(game::ShotTypes::Sentinel_DO_NOT_USE)> stype_strs {
			L"Standard", L"DoT", L"Slow", L"Stun"
		};
		return stype_strs.at(static_cast<int>(stype));
	}

	namespace game {

		/// <summary>Base class for all shot types.</summary>
		class ShotBaseType : public GameObjectType {
		public:
			/// <returns>The projectile's polymorphic type.</returns>
			virtual ShotTypes getType() const noexcept = 0;
			/// <summary>Applies the projectile's effect to the enemy that was directly
			/// hit by the projectile.</summary>
			/// <param name="e">The enemy directly hit by the projectile.</param>
			/// <param name="tower_dmg_multiplier">The damage multiplier from the tower that
			/// the shot originated from.</param>
			void doHit(Enemy& e, double tower_dmg_multiplier) const;
			/// <summary>Applies the projectile's splash effect to the enemies that were
			/// indirectly hit by the projectile.</summary>
			/// <param name="e">An enemy indirectly hit by the projectile.</param>
			/// <param name="tower_dmg_multiplier">The damage multiplier from the tower that
			/// the shot originated from.</param>
			virtual void doSplashHit(Enemy& e, double tower_dmg_multiplier) const;

			// Getters
			double getDamage() const noexcept {
				return this->damage;
			}
			double getSplashDamage() const noexcept {
				return this->splash_damage;
			}
			/// <param name="is_flying">Is the enemy flying?</param>
			/// <returns>The actual base direct damage dealt to an enemy after flying status is considered.</returns>
			double getActualDamage(bool is_flying) const noexcept {
				return this->getDamage() * (is_flying ? this->getAirMultiplier() : this->getGroundMultiplier());
			}
			/// <param name="is_flying">Is the enemy flying?</param>
			/// <returns>The actual base indirect damage dealt to an enemy after flying status is considered.</returns>
			double getActualSplashDamage(bool is_flying) const noexcept {
				return this->getSplashDamage() * (is_flying ? this->getAirMultiplier() : this->getGroundMultiplier());
			}
			double getGroundMultiplier() const noexcept {
				return this->ground_multiplier;
			}
			double getAirMultiplier() const noexcept {
				return this->air_multiplier;
			}
			double getImpactRadius() const noexcept {
				return this->impact_radius;
			}
			double getPiercing() const noexcept {
				return this->piercing;
			}
			double getSpeed() const noexcept {
				return this->move_speed;
			}
			/// <returns>The expected number of additional targets impacted by
			/// this projectile's splash effect.</returns>
			double getAverageExtraTargets() const noexcept {
				return this->getImpactRadius() * this->getImpactRadius() * math::pi;
			}
			/// <returns>The shot's expected average damage factoring in both multipliers
			/// and splash damage.</returns>
			double getExpectedRawDamage() const noexcept {
				return this->getExpectedBaseDamage() * this->getDamageMultiplierRating();
			}
			/// <returns>The shot's total rating which also considers its special effects.</returns>
			double getRating() const noexcept {
				return this->getBaseRating() + this->getExtraRating();
			}
			/// <returns>The shot's extra rating which considers its special effects primarily.</returns>
			virtual double getExtraRating() const noexcept {
				return 0;
			}

			/// <returns>True if the shot's effect also applies to enemies indirectly hit.</returns>
			virtual bool isSplashEffectType() const noexcept {
				return false;
			}
		protected:
			ShotBaseType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				double dmg, double wap, double ms, double ir, double sdmg, double gm, double am) :
				GameObjectType {n, d, c, st},
				damage {dmg},
				piercing {wap},
				move_speed {ms},
				impact_radius {ir},
				splash_damage {sdmg},
				ground_multiplier {gm},
				air_multiplier {am} {
			}
			/// <summary>Applies the secondary effect to the enemy.</summary>
			/// <param name="e">The enemy to apply the effect to.</param>
			virtual void apply(Enemy& e) const = 0;

			/// <returns>The expected average base damage applied before multipliers and piercing.</returns>
			double getExpectedBaseDamage() const noexcept {
				return this->getAverageExtraTargets() * this->getSplashDamage() + this->getDamage();
			}
			/// <returns>The amount that the damage multipliers contribute the shot's rating as a whole.</returns>
			double getDamageMultiplierRating() const noexcept {
				return this->getAirMultiplier() * 0.33 + this->getGroundMultiplier() * 0.67;
			}
			/// <returns>The shot's basic rating which considers its core stats only.</returns>
			double getBaseRating() const noexcept {
				return this->getExpectedBaseDamage() * this->getDamageMultiplierRating()
					* (1.0 + this->getPiercing() / 2.0) + (this->getSpeed() * 0.01);
			}

		private:
			/// <summary>The amount of damage dealt by the shot when it directly hits an enemy.</summary>
			double damage;
			/// <summary>The weapon's armor piercing rating.</summary>
			double piercing;
			/// <summary>The projectile's movement speed (in game coordinate squares per second).</summary>
			double move_speed;
			/// <summary>The area (in game coordinate squares) in which splash damage and splash effects
			/// occur after the initial impact. (Think of a bomb exploding and affecting the people around
			/// the area where it initially explodes.)</summary>
			double impact_radius;
			/// <summary>The amount of damage dealt to enemies within the explosion radius.</summary>
			double splash_damage;
			/// <summary>The damage multiplier applied to the shot against ground enemies.</summary>
			double ground_multiplier;
			/// <summary>The damage multiplier applied to the shot against air enemies.</summary>
			double air_multiplier;
		};

		/// <summary>Class that represents a standard shot type.</summary>
		class NormalShotType : public ShotBaseType {
		public:
			NormalShotType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				double dmg, double wap, double ms, double ir, double sdmg, double gm, double am) :
				ShotBaseType {n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am} {
			}
			// Implements ShotBaseType::getType()
			ShotTypes getType() const noexcept override {
				return ShotTypes::Standard;
			}
		protected:
			// Implements ShotBaseType::apply()
			void apply(Enemy& e) const override {
				// Do nothing
				UNREFERENCED_PARAMETER(e);
			}
		};

		/// <summary>Another class that adds a few extra details
		/// for shots that have a secondary effect attached to them. (i.e.: everything
		/// except standard shots.</summary>
		class ShotEffectType : public ShotBaseType {
		protected:
			ShotEffectType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				double dmg, double wap, double ms, double ir, double sdmg, double gm, double am, bool splash_too) :
				ShotBaseType {n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am},
				affects_splash {splash_too} {
			}

			// Overrides ShotBaseType::doSplashHit()
			void doSplashHit(Enemy& e, double tower_dmg_multiplier) const final;
			// Overrides ShotBaseType::isSplashEffectType()
			bool isSplashEffectType() const noexcept override {
				return affects_splash;
			}
		protected:
		private:
			/// <summary>Whether or not this effect should also be applied to splash targets.</summary>
			bool affects_splash;
		};

		/// <summary>Class that represents a shot that deals additional DoT damage
		/// after the initial impact.</summary>
		class DoTShotType : public ShotEffectType {
		public:
			/// <param name="dmg_type">The type of damage dealt by the DoT.</param>
			/// <param name="tick_dmg">The amount of damage dealt per tick.</param>
			/// <param name="ms_ticks">The amount of time (in ms) between ticks.</param>
			/// <param name="num_ticks">The total number of ticks.</param>
			DoTShotType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				double dmg, double wap, double ms, double ir, double sdmg, double gm, double am, bool splash_too,
				DoTDamageTypes dmg_type, double tick_dmg, int ms_ticks, int num_ticks) :
				ShotEffectType {n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am, splash_too},
				damage_type {dmg_type},
				dmg_per_tick {tick_dmg},
				time_between_ticks {ms_ticks},
				total_ticks {num_ticks} {
			}
			// Implements ShotBaseType::getType()
			ShotTypes getType() const noexcept override {
				return ShotTypes::DoT;
			}

			// Getters
			DoTDamageTypes getDamageType() const noexcept {
				return this->damage_type;
			}
			double getDamagePerTick() const noexcept {
				return this->dmg_per_tick;
			}
			int getMillisecondsBetweenTicks() const noexcept {
				return this->time_between_ticks;
			}
			int getTotalTicks() const noexcept {
				return this->total_ticks;
			}
			// Derived getters:
			/// <returns>The total amount of damage done by the DoT effect (to a single enemy).</returns>
			double getDoTTotalDamage() const noexcept {
				return this->getDamagePerTick() * this->getTotalTicks();
			}
			/// <returns>The total amount of time the DoT effect lasts for in **seconds**.</returns>
			double getDoTTotalDuration() const noexcept {
				return static_cast<double>(this->getMillisecondsBetweenTicks()) * this->getTotalTicks() / 1000.0;
			}
			/// <returns>The average damage per second dealt to a single enemy.</returns>
			double getDoTDPS() const noexcept {
				return this->getDoTTotalDamage() / this->getDoTTotalDuration();
			}
			/// <returns>The average damage per second dealt to all targets (ignoring failure
			/// to reapply effect; i.e.: idealized.) (Also ignores multipliers.)</returns>
			double getDoTFullDPS() const noexcept {
				return this->getDoTDPS() * (1.0 + (this->isSplashEffectType() ? this->getAverageExtraTargets() : 0.0));
			}
			// Overrides ShotBaseType::getExtraRating()
			double getExtraRating() const noexcept override {
				return (this->getDamageType() == DoTDamageTypes::Fire ? 1.25
					: this->getDamageType() == DoTDamageTypes::Poison ? 1.40
					: this->getDamageType() == DoTDamageTypes::Heal ? -0.05 : 1.0)
					* this->getDamageMultiplierRating() * this->getDoTFullDPS() + 0.8;
			}
		protected:
			// Implements ShotEffectType::apply()
			void apply(Enemy& e) const override;
		private:
			/// <summary>The type of damage dealt by the damage over time effect.</summary>
			DoTDamageTypes damage_type;
			/// <summary>The amount of damage dealt per tick of the DoT.</summary>
			double dmg_per_tick;
			/// <summary>The amount of time (in milliseconds) between ticks.</summary>
			int time_between_ticks;
			/// <summary>The total number of damage ticks before the DoT effect expires.</summary>
			int total_ticks;
		};

		/// <summary>Class that represents a shot that slows the enemy down.</summary>
		class SlowShotType : public ShotEffectType {
		public:
			SlowShotType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				double dmg, double wap, double ms, double ir, double sdmg, double gm, double am, bool splash_too,
				double sf, int sd, double schance) :
				ShotEffectType {n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am, splash_too},
				slow_factor {sf},
				slow_duration {sd},
				multi_slow_chance {schance} {
			}

			// Implements ShotBaseType::getType()
			ShotTypes getType() const noexcept override {
				return ShotTypes::Slow;
			}
			// Getters
			double getSlowFactor() const noexcept {
				return this->slow_factor;
			}
			int getSlowDuration() const noexcept {
				return this->slow_duration;
			}
			double getMultipleSlowChance() const noexcept {
				return this->multi_slow_chance;
			}
			// Overrides ShotBaseType::getExtraRating()
			double getExtraRating() const noexcept override {
				return this->getSlowFactor() * this->getSlowDuration() / 1000.0
					* (1.0 + (this->isSplashEffectType() ? this->getAverageExtraTargets() : 0.0))
					* (1.0 + this->getMultipleSlowChance() / 2.5) + 5.0;
			}
		protected:
			// Implements ShotBaseType::apply()
			void apply(Enemy& e) const override;

		private:
			/// <summary>The slow factor applied to the enemy; the enemy will move this percentage
			/// slower. (Eg: 25% Slow Factor means the enemy moves at 75% of their original speed
			/// which is 25% slower.)</summary>
			double slow_factor;
			/// <summary>The duration of the slow effect in milliseconds.</summary>
			int slow_duration;
			/// <summary>The percentage chance (as a decimal) that the slow effect works if another
			/// slow effect is already present on the enemy.</summary>
			double multi_slow_chance;
		};

		/// <summary>Class that represents a shot that stuns an enemy.</summary>
		class StunShotType : public ShotEffectType {
		public:
			StunShotType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st,
				double dmg, double wap, double ms, double ir, double sdmg, double gm, double am, bool splash_too,
				double schance, int sd, double mschance) :
				ShotEffectType {n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am, splash_too},
				stun_chance {schance},
				stun_duration {sd},
				multi_stun_chance {mschance} {
			}
			// Implements ShotBaseType::getType()
			ShotTypes getType() const noexcept override {
				return ShotTypes::Stun;
			}
			// Getters
			double getStunChance() const noexcept {
				return this->stun_chance;
			}
			int getStunDuration() const noexcept {
				return this->stun_duration;
			}
			double getMultipleStunChance() const noexcept {
				return this->multi_stun_chance;
			}
			// Overrides ShotBaseType::getExtraRating()
			double getExtraRating() const noexcept override {
				return this->getStunChance() * this->getStunDuration() / 1000.0
					* (1.0 + (this->isSplashEffectType() ? this->getAverageExtraTargets() : 0.0))
					* (1.0 + this->getMultipleStunChance() / 1.5) + 7.5;
			}
		protected:
			// Implements ShotBaseType::apply()
			void apply(Enemy& e) const override;
		private:
			/// <summary>The probability that the shot will stun the enemy if
			/// the enemy isn't currently stunned.</summary>
			double stun_chance;
			/// <summary>The time in milliseconds that the stun lasts for.</summary>
			int stun_duration;
			/// <summary>The probability that the enemy will be stunned a second
			/// or subsequent time if a stun is already active.</summary>
			double multi_stun_chance;
		};
	}
}