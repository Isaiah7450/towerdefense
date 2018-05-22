// File Author: Isaiah Hoffman
// File Created: May 21, 2018
#include "./enemy.hpp"
#include "./my_game.hpp"
#include "./status_effects.hpp"

namespace hoffman::isaiah {
	namespace game {
		StatusEffectBase::~StatusEffectBase() = default;

		bool DoTEffect::update(Enemy& e) {
			++this->frames_since_last_tick;
			if (this->frames_since_last_tick >= this->frames_between_ticks) {
				this->frames_since_last_tick -= this->frames_between_ticks;
				--this->total_ticks;
				switch (this->type) {
				case DoTDamageTypes::Fire:
					e.takeDamage(this->dmg_per_tick, 0.5);
					break;
				case DoTDamageTypes::Poison:
				default:
					e.takeDamage(this->dmg_per_tick, 0.8, true);
					break;
				}
			}
			return this->total_ticks <= 0;
		}

		bool SmartStrategyEffect::update(Enemy& e) {
			// (Weird ordering, but I don't want the case
			// where a programming/design bug causes
			// the enemy's strategy to be permanently
			// replaced.)
			if (this->frames_until_expires <= 0) {
				// Revert to default
				e.changeStrategy(game::g_my_game->getMap(), e.getBaseType().getDefaultStrategy(),
					e.getBaseType().canMoveDiagonally());
				return true;
			}
			else if (!this->ran_once) {
				this->ran_once = true;
				// Recalculating the path, esp. if it isn't
				// necessary, is expensive, so check first
				// if it is necessary.
				if (e.canMoveDiagonally() == this->diag_change && e.getCurrentStrategy() == this->strat) {
					// This also prevents a bunch of this effect stacking up
					// on enemy by eliminating unnecessary ones
					return true;
				}
				e.changeStrategy(game::g_my_game->getMap(), this->strat, this->diag_change);
			}
			--this->frames_until_expires;
			return false;
		}
	}
}