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
				frames_until_expires {StatusEffectBase::convert_milliseconds_to_frames(ms_til_expires)},
				strat {new_strat},
				diag_change {diag_move_change} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e);
		private:
			/// <summary>The number of frames remaining until the status
			/// effect expires.</summary>
			double frames_until_expires;
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
	}
}