// File Author: Isaiah Hoffman
// File Created: May 21, 2018
#include "./enemy.hpp"
#include "./status_effects.hpp"

namespace hoffman::isaiah {
	namespace game {
		StatusEffectBase::~StatusEffectBase() = default;

		bool DoTEffect::update(Enemy& e) {
			// Milliseconds per frame
			constexpr const double ms_per_frame = 1000.0 / game::logic_framerate;
			const double frames_per_tick = this->time_between_ticks / ms_per_frame;
			++this->frames_since_last_tick;
			if (this->frames_since_last_tick >= frames_per_tick) {
				this->frames_since_last_tick -= frames_per_tick;
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
	}
}