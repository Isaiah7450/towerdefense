// File Author: Isaiah Hoffman
// File Created: June 2, 2018
#include <cmath>
#include <string>
#include <utility>
#include <memory>
#include <vector>
#include <algorithm>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./game_object_type.hpp"
#include "./enemy_type.hpp"
#include "./shot_types.hpp"
#include "./tower_types.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Don't forget to keep this version and Tower's version synced.
		double TowerType::getAverageDamagePerShot() const noexcept {
			// (This is a weighted average.)
			double sum = 0.0;
			for (const auto& st : this->shot_types) {
				sum += st.first->getExpectedRawDamage() * st.second;
			}
			return sum;
		}

		double TowerType::getAverageShotRating() const noexcept {
			// (This is a weighted average.)
			double sum = 0.0;
			for (const auto& st : this->shot_types) {
				sum += st.first->getRating() * st.second;
			}
			return sum;
		}

		double TowerType::getRating() const noexcept {
			if (this->isWall()) return 1.0;
			const auto speed_range_multipliers = this->getRateOfFire()
				* (this->getFiringArea() / 2.5);
			const auto behavior_modifier = (this->getFiringMethod().getMethod() == FiringMethodTypes::Default ? 0.0 : -2.5)
				+ (this->getTargetingStrategy().getStrategy() == TargetingStrategyTypes::Distances ? 0.0 : 3.5);
			const auto my_dps_idea = this->getAverageDamagePerShot() * speed_range_multipliers;
			const auto my_effect_idea = (this->getAverageShotRating() - this->getAverageDamagePerShot())
				* speed_range_multipliers;
			if (my_dps_idea > my_effect_idea) {
				return (my_dps_idea * 0.65 + my_effect_idea * 0.35)
					+ behavior_modifier;
			}
			else {
				return (my_dps_idea * 0.35 + my_effect_idea * 0.65)
					+ behavior_modifier;
			}
		}
	}
}