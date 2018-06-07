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
			return (this->getAverageDamagePerShot() * 0.4 + this->getAverageShotRating() * 0.6)
				* this->getRateOfFire() * this->getFiringArea() / 2.5
				- (this->getFiringMethod().getMethod() == FiringMethodTypes::Default ? 0.0 : 2.5)
				+ (this->getTargetingStrategy().getStrategy() == TargetingStrategyTypes::Distances ? 0.0 : 3.5);
		}
	}
}