// File Author: Isaiah Hoffman
// File Created: June 2, 2018
#include <cmath>
#include <string>
#include <utility>
#include <memory>
#include <vector>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./game_object_type.hpp"
#include "./enemy_type.hpp"
#include "./shot_types.hpp"
#include "./tower_types.hpp"

namespace hoffman::isaiah {
	namespace game {
		double TowerType::getAverageDamagePerShot() const noexcept {
			double sum = 0.0;
			int count = 0;
			for (const auto& st : this->shot_types) {
				sum += st.first->getExpectedRawDamage() * st.second;
				++count;
			}
			return sum / static_cast<double>(count);
		}

		double TowerType::getAverageShotRating() const noexcept {
			double sum = 0.0;
			int count = 0;
			for (const auto& st : this->shot_types) {
				sum += st.first->getRating() * st.second;
				++count;
			}
			return sum / static_cast<double>(count);
		}

		double TowerType::getRating() const noexcept {
			return math::get_avg(this->getAverageDamagePerShot() * 1.5, this->getAverageShotRating())
				* this->getRateOfFire() * this->getFiringRange() * this->getFiringRange() * math::pi;
		}
	}
}