// File Author: Isaiah Hoffman
// File Created: May 21, 2018
#include "./enemy.hpp"
#include "./my_game.hpp"
#include "./status_effects.hpp"
#include "./game_util.hpp"

namespace hoffman_isaiah {
	namespace game {

		bool DoTEffect::update(Enemy& e) {
			++this->frames_since_last_tick;
			e.setDoTActive(true);
			if (this->frames_since_last_tick >= this->frames_between_ticks) {
				this->frames_since_last_tick -= this->frames_between_ticks;
				--this->total_ticks;
				switch (this->type) {
				case DoTDamageTypes::Heal:
					e.heal(this->dmg_per_tick);
					break;
				case DoTDamageTypes::Fire:
					e.takeDamage(this->dmg_per_tick, 0.5);
					break;
				case DoTDamageTypes::Poison:
				default:
					e.takeDamage(this->dmg_per_tick, 0.8, true);
					break;
				}
			}
			if (this->total_ticks <= 0) {
				e.setDoTActive(false);
			}
			return this->total_ticks <= 0;
		}

		bool SmartStrategyEffect::update(Enemy& e) {
			// (Weird ordering, but I don't want the case
			// where a programming/design bug causes
			// the enemy's strategy to be permanently
			// replaced.)
			if (this->frames_until_expire <= 0) {
				// Revert to default
				e.changeStrategy(game::g_my_game->getMap(), e.getBaseType().getDefaultStrategy(),
					e.getBaseType().canMoveDiagonally());
				return true;
			}
			else if (!this->ran_once) {
				this->ran_once = true;
				const auto my_roll = rng::distro_uniform(rng::gen);
				this->frames_until_apply = my_roll < 0.05 ? 0
					: my_roll < 0.10 ? 1 : my_roll < 0.15 ? 2
					: my_roll < 0.20 ? 3 : my_roll < 0.25 ? 4
					: my_roll < 0.30 ? 5 : my_roll < 0.35 ? 6
					: my_roll < 0.40 ? 7 : my_roll < 0.45 ? 8
					: my_roll < 0.50 ? 9 : my_roll < 0.55 ? 10
					: my_roll < 0.60 ? 11 : my_roll < 0.65 ? 12
					: my_roll < 0.70 ? 13 : my_roll < 0.75 ? 14
					: my_roll < 0.80 ? 15 : my_roll < 0.85 ? 16
					: my_roll < 0.90 ? 17 : my_roll < 0.95 ? 18 : 19;
			}
			if (this->frames_until_apply == 0) {
				// Recalculating the path, esp. if it isn't
				// necessary, is expensive, so check first
				// if it is necessary.
				if (e.canMoveDiagonally() == this->diag_change && e.getCurrentStrategy() == this->strat) {
					// This also prevents a bunch of this effect stacking up
					// on enemy by eliminating unnecessary ones
					return true;
				}
				e.changeStrategy(game::g_my_game->getMap(), this->strat, this->diag_change);
				this->done_payload = true;
			}
			if (this->done_payload) {
				--this->frames_until_expire;
			}
			else {
				--this->frames_until_apply;
			}
			return false;
		}

		bool SlowEffect::update(Enemy& e) {
			// It is worth noting that slow effects are multiplicative.
			// For instance, if an enemy receives both a 10% slow effect
			// and a 25% slow effect, the total slowdown for the enemy
			// becomes 0.90 * 0.75 => 0.675 or a 32.5% decrease in speed!
			// Note also that there is a minimum speed multiplier for enemies
			// for balance sake.
			e.setSpeedMultiplier(e.getSpeedMultiplier() * this->speed_multiplier);
			--this->frames_until_expire;
			return this->frames_until_expire <= 0;
		}

		void StunEffect::clearEffects(Enemy& e) {
			e.setStun(false);
		}

		bool StunEffect::update(Enemy& e) {
			e.setStun(true);
			--this->frames_until_expire;
			if (this->frames_until_expire < 0) {
				this->clearEffects(e);
			}
			return this->frames_until_expire < 0;
		}

		bool SpeedBoostEffect::update(Enemy& e) {
			// Like with the SlowEffect, these values are multiplicative,
			// not additive.
			// One has to be careful here as there is no upper cap, so
			// carelessness could lead to enemies that radiate speed boosts
			// causing tremendous speed boosts!
			e.multiplyWalkingBoost(this->walking_boost);
			e.multiplyRunningBoost(this->running_boost);
			e.multiplyInjuredBoost(this->injured_boost);
			--this->frames_until_expire;
			return this->frames_until_expire <= 0;
		}

		bool ShieldEffect::update(Enemy& e) {
			e.degradeShield(this->shield_dmg_per_tick);
			--this->frames_until_expire;
			return this->frames_until_expire <= 0 || e.getShieldHealth() <= 0.0;
		}
	}
}