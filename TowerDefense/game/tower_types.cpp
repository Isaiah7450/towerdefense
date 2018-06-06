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
			return math::get_avg(this->getAverageDamagePerShot() * 1.5, this->getAverageShotRating())
				* this->getRateOfFire() * this->getFiringRange() * std::sqrt(math::e)
				* (this->getFiringMethod().getMethod() == FiringMethodTypes::Default ? 1.0
					: (this->getFiringMethod().getMaximumAngle() - this->getFiringMethod().getMinimumAngle())
						/ (2.0 * math::pi));
		}
	}
}