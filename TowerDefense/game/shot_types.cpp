// File Author: Isaiah Hoffman
// File Created: May 31, 2018
#include <cmath>
#include <string>
#include "./../ih_math.hpp"
#include "./../graphics/graphics.hpp"
#include "./enemy.hpp"
#include "./game_object_type.hpp"
#include "./game_util.hpp"
#include "./shot_types.hpp"
#include "./status_effects.hpp"

namespace hoffman_isaiah {
	namespace game {
		void ShotBaseType::doHit(Enemy& e, double tower_dmg_multiplier) const {
			e.takeDamage(this->getActualDamage(e.getBaseType().isFlying())
				* tower_dmg_multiplier, this->getPiercing());
			this->apply(e);
		}

		void ShotBaseType::doSplashHit(Enemy& e, double tower_dmg_multiplier) const {
			e.takeDamage(this->getActualSplashDamage(e.getBaseType().isFlying())
				* tower_dmg_multiplier, this->getPiercing());
		}

		void ShotEffectType::doSplashHit(Enemy& e, double tower_dmg_multiplier) const {
			ShotBaseType::doSplashHit(e, tower_dmg_multiplier);
			if (this->isSplashEffectType()) {
				this->apply(e);
			}
		}

		void DoTShotType::apply(Enemy& e) const {
			if (e.hasActiveDoT()) {
				// Don't apply another (because otherwise, DoT gets overpowered)
				return;
			}
			auto my_status = std::make_unique<DoTEffect>(this->getDamageType(),
				this->getDamagePerTick(), this->getMillisecondsBetweenTicks(), this->getTotalTicks());
			e.addStatus(std::move(my_status));
		}

		void SlowShotType::apply(Enemy& e) const {
			if (e.isSlowed()) {
				auto roll = rng::distro_uniform(rng::gen);
				if (roll >= this->getMultipleSlowChance()) {
					return;
				}
			}
			auto my_status = std::make_unique<SlowEffect>(this->getSlowDuration(), this->getSlowFactor());
			e.addStatus(std::move(my_status));
		}

		void StunShotType::apply(Enemy& e) const {
			if (e.isStunned()) {
				auto roll = rng::distro_uniform(rng::gen);
				if (roll >= this->getMultipleStunChance()) {
					return;
				}
			}
			else {
				auto roll = rng::distro_uniform(rng::gen);
				if (roll >= this->getStunChance()) {
					return;
				}
			}
			auto my_status = std::make_unique<StunEffect>(this->getStunDuration());
			e.addStatus(std::move(my_status));
		}
	}
}