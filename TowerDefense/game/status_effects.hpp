#pragma once
// File Author: Isaiah Hoffman
// File Created: May 21, 2018
#include "./../ih_math.hpp"
#include <array>
#include <string>

namespace hoffman_isaiah {
	namespace game {
		// Forward declarations:
		class Enemy;

		/// <summary>Enumeration of possible status effects.</summary>
		enum class StatusEffects {
			DoT, Smart, Slow, Stun, Speed_Boost, Forcefield
		};

		/// <summary>Base class for all status effects.</summary>
		class StatusEffectBase {
		public:
			StatusEffectBase(StatusEffects my_effect_type) noexcept :
				effect_type {my_effect_type} {
			}
			virtual ~StatusEffectBase() noexcept = default;
			// Rule of 5:
			StatusEffectBase(const StatusEffectBase&) noexcept = default;
			StatusEffectBase(StatusEffectBase&&) noexcept = default;
			StatusEffectBase& operator=(const StatusEffectBase&) noexcept = default;
			StatusEffectBase& operator=(StatusEffectBase&&) noexcept = default;
			/// <summary>This function clears harmful effects when the status effect finishes.</summary>
			virtual void clearEffects(Enemy& e) {
				UNREFERENCED_PARAMETER(e);
			}
			/// <summary>This function is called once every frame on each
			/// enemy afflicted by this status effect.</summary>
			/// <param name="e">The enemy that this is being called for.</param>
			/// <returns>True if the status effect should be removed from the enemy.</returns>
			virtual bool update(Enemy& e) = 0;
			// Getters
			StatusEffects getStatusEffectType() const noexcept {
				return this->effect_type;
			}
			/// <returns>True if the status effect is considered beneficial to the enemy; otherwise, false.</returns>
			virtual bool isPositiveEffect() const noexcept = 0;
		private:
			/// <summary>The status effect type.</summary>
			StatusEffects effect_type;
		};

		/// <summary>Enumeration of possible DoT damage types.</summary>
		enum class DoTDamageTypes {
			Poison, Fire, Heal, Sentinel_DO_NOT_USE
		};
	}

	inline std::wstring operator*(game::DoTDamageTypes dtype) noexcept {
		constexpr const std::array<const wchar_t*,
			static_cast<int>(game::DoTDamageTypes::Sentinel_DO_NOT_USE)> my_dtype_strs {
			L"Poison", L"Fire", L"Heal"
		};
		return my_dtype_strs.at(static_cast<int>(dtype));
	}

	namespace game {
		// Use DoTDamageTypes::Heal to use this as a healing effect
		/// <summary>Class that represents a status effect that deals
		/// damage over time.</summary>
		class DoTEffect : public StatusEffectBase {
		public:
			/// <param name="dt">The damage type of the DoT.</param>
			/// <param name="dmg_tick">The amount of damage dealt per tick.</param>
			/// <param name="ms_tick">The amount of time (in ms) between ticks.</param>
			/// <param name="t_ticks">The total number of ticks.</param>
			DoTEffect(DoTDamageTypes dt, double dmg_tick, int ms_tick, int t_ticks) :
				StatusEffectBase {StatusEffects::DoT},
				type {dt},
				dmg_per_tick {dmg_tick},
				frames_between_ticks {math::convertMillisecondsToFrames(ms_tick)},
				total_ticks {t_ticks} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e) override;
			// Implements StatusEffectBase::isPositiveEffect()
			bool isPositiveEffect() const noexcept override {
				return this->type == DoTDamageTypes::Heal;
			}
		private:
			/// <summary>The type of damage dealt by this DoT.</summary>
			DoTDamageTypes type;
			/// <summary>The amount of damage dealt per tick.</summary>
			double dmg_per_tick;
			/// <summary>The number of logical frames between ticks.</summary>
			double frames_between_ticks;
			/// <summary>The number of ticks before the DoT effect disappears.</summary>
			int total_ticks;
			/// <summary>The number of frames since the last tick.</summary>
			double frames_since_last_tick {0};
		};

		/// <summary>Class that represents a status effect where the enemy
		/// becomes smarter while the effect lasts.</summary>
		class SmartStrategyEffect : public StatusEffectBase {
		public:
			/// <param name="ms_til_expires">The number of milliseconds until the effect
			/// expires.</param>
			SmartStrategyEffect(int ms_til_expires, pathfinding::HeuristicStrategies new_strat,
				bool diag_move_change) :
				StatusEffectBase {StatusEffects::Smart},
				frames_until_expire {math::convertMillisecondsToFrames(ms_til_expires)},
				strat {new_strat},
				diag_change {diag_move_change} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e) override;
			// Implements StatusEffectBase::isPositiveEffect()
			bool isPositiveEffect() const noexcept override {
				return true;
			}
		private:
			/// <summary>The number of frames remaining until the status
			/// effect expires.</summary>
			double frames_until_expire;
			/// <summary>The new strategy that the enemy will use while the status
			/// effect is present.</summary>
			pathfinding::HeuristicStrategies strat;
			/// <summary>Whether or not the enemy will move diagonally while the
			/// status effect is present.</summary>
			bool diag_change;
			/// <summary>False if this is the first time that update
			/// has been called for this status effect.</summary>
			bool ran_once {false};
			/// <summary>Have we executed the payload yet?</summary>
			bool done_payload {false};
			/// <summary>The number of frames to wait until the buff is applied.</summary>
			int frames_until_apply {0};
		};

		// Note: This effect doesn't have to just be used for slowing enemies.
		// It can also be used to boost their speeds; you just have to provide
		// a negative slow factor.
		/// <summary>Status effect that slows down an enemy's movement speed.</summary>
		class SlowEffect : public StatusEffectBase {
		public:
			/// <param name="ms_til_expires">The number of milliseconds until the
			/// status effect expires.</param>
			/// <param name="sf">The slow-factor applied to the enemy; the enemy
			/// will move this much slower.</param>
			SlowEffect(double ms_til_expires, double sf) :
				StatusEffectBase {StatusEffects::Slow},
				frames_until_expire {math::convertMillisecondsToFrames(ms_til_expires)},
				speed_multiplier {1.0 - sf} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e) override;
			// Implements StatusEffectBase::isPositiveEffect()
			bool isPositiveEffect() const noexcept override {
				return this->speed_multiplier > 1.0;
			}
		private:
			/// <summary>The number of frames until the effect expires.</summary>
			double frames_until_expire;
			/// <summary>The percentage (as a decimal) of the enemy's normal movement speed
			/// that the enemy moves at while slowed. E.g.: 0.2 means the enemy moves at 20%
			/// of their normal speed.</summary>
			double speed_multiplier;
		};

		/// <summary>Class that represents a stun effect.</summary>
		class StunEffect : public StatusEffectBase {
		public:
			/// <param name="ms_until_expires">The number of milliseconds until the effect expires.</param>
			StunEffect(int ms_until_expires) :
				StatusEffectBase {StatusEffects::Stun},
				frames_until_expire {math::convertMillisecondsToFrames(ms_until_expires)} {
			}
			// Implements StatusEffectBase::onClear()
			void clearEffects(Enemy& e) override;
			// Implements StatusEffectBase::update()
			bool update(Enemy& e) override;
			// Implements StatusEffectBase::isPositiveEffect()
			bool isPositiveEffect() const noexcept override {
				return false;
			}
		private:
			/// <summary>The number of frames until the effect expires.</summary>
			double frames_until_expire;
		};

		// I wouldn't recommend trying to use this to negatively impact
		// the enemy. Still, it is possible.
		/// <summary>This class represents a status effect that increases
		/// the speed of an enemy.</summary>
		class SpeedBoostEffect : public StatusEffectBase {
		public:
			/// <param name="ms_until_expires">The number of milliseconds that the effect
			/// lasts for.</param>
			/// <param name="wb">The percentage increase to the enemy's walking speed boost.
			/// (Expressed as a decimal, and note that 1.0 will be added to this value.)</param>
			/// <param name="rb">The percentage increase to the enemy's running speed boost.
			/// (Expressed as a decimal, and note that 1.0 will be added to this value.)</param>
			/// <param name="ib">The percentage increase to the enemy's injured speed boost.
			/// (Expressed as a decimal, and note that 1.0 will be added to this value.)</param>
			SpeedBoostEffect(int ms_until_expires, double wb, double rb, double ib) :
				StatusEffectBase {StatusEffects::Speed_Boost},
				frames_until_expire {math::convertMillisecondsToFrames(ms_until_expires)},
				walking_boost {1.0 + wb},
				running_boost {1.0 + rb},
				injured_boost {1.0 + ib} {
				// Probably should throw an exception if the value is negative;
				// Don't try to use this as a knockback effect; this game is not
				// designed for such an effect!
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e) override;
			// Implements StatusEffectBase::isPositiveEffect()
			bool isPositiveEffect() const noexcept override {
				return math::get_avg(this->walking_boost, this->running_boost, this->injured_boost) > 1.0;
			}
		private:
			/// <summary>The number of logical frames remaining before the effect
			/// expires.</summary>
			double frames_until_expire;
			/// <summary>The percent increase (as a decimal) to the enemy's walking speed.</summary>
			double walking_boost;
			/// <summary>The percent increase (as a decimal) to the enemy's running speed.</summary>
			double running_boost;
			/// <summary>The percent increase (as a decimal) to the enemy's injured speed.</summary>
			double injured_boost;
		};

		/// <summary>Represents the shield status effect.</summary>
		class ShieldEffect : public StatusEffectBase {
		public:
			/// <param name="ms_til_expires">The number of milliseconds until the
			/// status effect expires.</param>
			/// <param name="sh">The amount of health in the shield.</param>
			ShieldEffect(double ms_til_expires, double sh) :
				StatusEffectBase {StatusEffects::Forcefield},
				frames_until_expire {math::convertMillisecondsToFrames(ms_til_expires)},
				shield_dmg_per_tick {0} {
				this->shield_dmg_per_tick = sh / this->frames_until_expire;
			}
		protected:
			// Implements StatusEffectBase::update()
			bool update(Enemy& e) override;
			// Implements StatusEffectBase::isPositiveEffect()
			bool isPositiveEffect() const noexcept override {
				return true;
			}
		private:
			/// <summary>The number of frames until the effect expires.</summary>
			double frames_until_expire;
			/// <summary>The amount by which the enemy's shield degrades each frame.</summary>
			double shield_dmg_per_tick;
		};
	}
}