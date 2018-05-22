#pragma once
// File Author: Isaiah Hoffman
// File Created: May 21, 2018

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations:
		class Enemy;

		/// <summary>Enumeration of possible status effects.</summary>
		enum class StatusEffects {
			DoT
		};

		/// <summary>Base class for all status effects.</summary>
		class StatusEffectBase {
		public:
			virtual ~StatusEffectBase();
			/// <summary>This function is called once every frame on each
			/// enemy afflicted by this status effect.</summary>
			/// <param name="e">The enemy that this is being called for.</param>
			/// <returns>True if the status effect should be removed from the enemy.</returns>
			virtual bool update(Enemy& e) = 0;
			// Returns the number of milliseconds in a single logical frame.
			static constexpr double get_milliseconds_per_frame() noexcept {
				return 1000.0 / game::logic_framerate;
			}
			// Converts a time in milliseconds into a number of logical frames
			// that represent the same time span.
			// time_in_ms : The time in milliseconds to convert.
			static constexpr double convert_milliseconds_to_frames(double time_in_ms) noexcept {
				return time_in_ms / StatusEffectBase::get_milliseconds_per_frame();
			}
		private:
		};

		/// <summary>Enumeration of possible DoT damage types.</summary>
		enum class DoTDamageTypes {
			Poison, Fire
		};

		/// <summary>Class that represents a status effect that deals
		/// damage over time.</summary>
		class DoTEffect : public StatusEffectBase {
		public:
			/// <param name="dt">The damage type of the DoT.</param>
			/// <param name="dmg_tick">The amount of damage dealt per tick.</param>
			/// <param name="ms_tick">The amount of time (in ms) between ticks.</param>
			/// <param name="t_ticks">The total number of ticks.</param>
			DoTEffect(DoTDamageTypes dt, double dmg_tick, int ms_tick, int t_ticks) :
				type {dt},
				dmg_per_tick {dmg_tick},
				frames_between_ticks {StatusEffectBase::convert_milliseconds_to_frames(ms_tick)},
				total_ticks {t_ticks} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e);
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
				frames_until_expire {StatusEffectBase::convert_milliseconds_to_frames(ms_til_expires)},
				strat {new_strat},
				diag_change {diag_move_change} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e);
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
				frames_until_expire {StatusEffectBase::convert_milliseconds_to_frames(ms_til_expires)},
				speed_multiplier {1.0 - sf} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e);
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
				frames_until_expire {StatusEffectBase::convert_milliseconds_to_frames(ms_until_expires)} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e);
		private:
			/// <summary>The number of frames until the effect expires.</summary>
			double frames_until_expire;
		};

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
				frames_until_expire {StatusEffectBase::convert_milliseconds_to_frames(ms_until_expires)},
				walking_boost {1.0 + wb},
				running_boost {1.0 + rb},
				injured_boost {1.0 + ib} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e);
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
	}
}