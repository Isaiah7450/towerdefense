// Author: Isaiah Hoffman
// Created: March 17, 2019
#include "./game_formulas.hpp"
#include <cmath>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include "./../ih_math.hpp"
#include "./enemy.hpp"
#include "./enemy_type.hpp"
#include "./shot_types.hpp"
#include "./tower_types.hpp"

namespace hoffman_isaiah::game {
	namespace towers {
		double getFiringArea(double fr, const FiringMethod& fm, bool is_wall) noexcept {
			return !is_wall ? (fm.getMethod() == FiringMethodTypes::Default
				? fr * fr * math::pi
				: fr * fr * (fm.getMaximumAngle() - fm.getMinimumAngle()) / 2.0) : 0;
		}

		double getAverageDamagePerShot(const std::vector<std::pair<const ShotBaseType*, double>>& stypes, double dm) noexcept {
			double sum = 0.0;
			for (const auto& st : stypes) {
				sum += st.first->getBaseRating() * st.second;
			}
			return sum * dm;
		}

		double getAverageShotEffectRating(const std::vector<std::pair<const ShotBaseType*, double>>& stypes) noexcept {
			double sum = 0.0;
			for (const auto& st : stypes) {
				sum += st.first->getExtraRating() * st.second;
			}
			return sum;
		}

		double getAverageShotRating(const std::vector<std::pair<const ShotBaseType*, double>>& stypes) noexcept {
			double sum = 0.0;
			for (const auto& st : stypes) {
				sum += st.first->getRating() * st.second;
			}
			return sum;
		}

		double getRating(double rate_of_fire, double firing_range, double firing_area, const FiringMethod& fm, const TargetingStrategy& ts,
			double avg_dmg, double avg_effect_rating, bool is_wall) noexcept {
			UNREFERENCED_PARAMETER(firing_area);
			if (is_wall) return 1.0;
			const auto my_coverage_angle = fm.getMethod() == FiringMethodTypes::Default ? 2.0 * math::pi
				: (fm.getMaximumAngle() - fm.getMinimumAngle());
//			const auto my_range_modifier = my_coverage_angle * (firing_range * std::log(firing_range - 1.0 + math::e));
			const auto my_range_modifier = my_coverage_angle * (firing_range * std::sqrt(firing_range));
			const auto speed_range_multipliers = rate_of_fire * (my_range_modifier / 2.5);
			const auto behavior_modifier = (fm.getMethod() == FiringMethodTypes::Default ? 0.0 : -2.5)
				+ (ts.getStrategy() == TargetingStrategyTypes::Distances ? 0.0 : 3.5);
			const auto my_damage_value = avg_dmg;
			const auto my_effect_value = avg_effect_rating * 1.33;
			if (my_damage_value > avg_effect_rating) {
				return (my_damage_value * 0.65 + my_effect_value * 0.65) * speed_range_multipliers
					+ behavior_modifier;
			}
			else {
				return (my_damage_value * 0.40 + my_effect_value * 0.60) * speed_range_multipliers
					+ behavior_modifier;
			}
		}
	}

	namespace enemy_buffs {
		namespace buff_base {
			double getAverageInfluenceRating(std::vector<std::wstring> target_names,
				const std::vector<std::unique_ptr<EnemyType>>& etypes) {
				double total_rating = 0.0;
				for (const auto& ename : target_names) {
					for (const auto& my_type : etypes) {
						if (my_type->getName() == ename) {
							total_rating += my_type->getBaseRating();
						}
					}
				}
				return total_rating / static_cast<double>(target_names.size());
			}
		}
	}

	namespace enemies {
		double getExtraRating(const std::vector<std::shared_ptr<BuffBase>>& buffs) noexcept {
			double sum_buff_ratings = 0.0;
			for (const auto& b : buffs) {
				sum_buff_ratings += b->getRating();
			}
			return sum_buff_ratings * std::cbrt(buffs.size());
		}
	}
}
