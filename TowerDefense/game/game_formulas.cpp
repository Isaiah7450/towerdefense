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

namespace hoffman::isaiah::game {
	namespace towers {
		double getFiringArea(double fr, const FiringMethod& fm, bool is_wall) noexcept {
			return !is_wall ? (fm.getMethod() == FiringMethodTypes::Default
				? fr * fr * math::pi
				: fr * fr * (fm.getMaximumAngle() - fm.getMinimumAngle()) / 2.0) : 0;
		}

		double getAverageDamagePerShot(const std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>& stypes, double dm) noexcept {
			double sum = 0.0;
			for (const auto& st : stypes) {
				sum += st.first->getExpectedRawDamage() * st.second;
			}
			return sum * dm;
		}

		double getAverageShotRating(const std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>& stypes) noexcept {
			double sum = 0.0;
			for (const auto& st : stypes) {
				sum += st.first->getRating() * st.second;
			}
			return sum;
		}

		double getRating(double rate_of_fire, double firing_area, const FiringMethod& fm, const TargetingStrategy& ts,
			double avg_dmg, double avg_shot_rating, bool is_wall) noexcept {
			if (is_wall) return 1.0;
			const auto speed_range_multipliers = rate_of_fire * (firing_area / 2.5);
			const auto behavior_modifier = (fm.getMethod() == FiringMethodTypes::Default ? 0.0 : -2.5)
				+ (ts.getStrategy() == TargetingStrategyTypes::Distances ? 0.0 : 3.5);
			const auto my_dps_idea = avg_dmg * speed_range_multipliers;
			const auto my_effect_idea = max(0.0, (avg_shot_rating - avg_dmg)) * speed_range_multipliers;
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

	namespace enemy_buffs {
		namespace buff_base {
			double getAverageInfluenceRating(std::vector<std::wstring> target_names,
				const std::map<std::wstring, std::shared_ptr<EnemyType>>& etypes) {
				double total_rating = 0.0;
				for (const auto& ename : target_names) {
					total_rating += etypes.at(ename)->getBaseRating();
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
