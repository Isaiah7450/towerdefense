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
				time_between_ticks {ms_tick},
				total_ticks {t_ticks} {
			}
			// Implements StatusEffectBase::update()
			bool update(Enemy& e);
		private:
			/// <summary>The type of damage dealt by this DoT.</summary>
			DoTDamageTypes type;
			/// <summary>The amount of damage dealt per tick.</summary>
			double dmg_per_tick;
			// (Note that this value is subject to the logic framerate speed;
			// resolutions faster than 10 ms aren't really supported.)
			/// <summary>The amount of time in milliseconds between ticks.</summary>
			int time_between_ticks;
			/// <summary>The number of ticks before the DoT effect disappears.</summary>
			int total_ticks;
			/// <summary>The number of frames since the last tick.</summary>
			double frames_since_last_tick {0};
		};
	}
}