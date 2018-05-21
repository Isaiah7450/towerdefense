#pragma once
// File Author: Isaiah Hoffman
// File Created: May 21, 2018

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations:
		class Enemy;

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

		class DoTEffect : public StatusEffectBase {
		public:
		private:
			/// <summary>The type of damage dealt by this DoT.</summary>
			DoTDamageTypes type;
		};
	}
}