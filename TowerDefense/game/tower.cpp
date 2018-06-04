// File Author: Isaiah Hoffman
// File Created: June 4, 2018
#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../pathfinding/grid.hpp"
#include "./enemy.hpp"
#include "./game_object.hpp"
#include "./game_util.hpp"
#include "./shot.hpp"
#include "./tower_types.hpp"
#include "./tower.hpp"

namespace hoffman::isaiah {
	namespace game {
		std::unique_ptr<Shot> Tower::update(const std::vector<std::unique_ptr<Enemy>>& enemies) {
			if (this->getBaseType()->isWall()) {
				return nullptr;
			}
			if (this->must_reload) {
				--this->frames_to_reload;
				if (this->frames_to_reload <= 0) {
					this->must_reload = false;
					this->shots_fired_since_reload = 0;
				}
				else return nullptr;
			}
			--this->frames_til_next_shot;
			if (this->frames_til_next_shot <= 0.0) {
				this->frames_til_next_shot += game::logic_framerate / this->getBaseType()->getFiringSpeed();
				auto target = this->findTarget(enemies);
				if (!target) {
					// Take the time to reload a single shot instead of firing
					--this->shots_fired_since_reload;
				}
				else {
					++this->shots_fired_since_reload;
					if (this->shots_fired_since_reload >= this->getBaseType()->getVolleyShots()
						&& this->getBaseType()->getVolleyShots() != 0) {
						this->must_reload = true;
						this->frames_to_reload += math::convertMillisecondsToFrames(this->getBaseType()->getReloadDelay());
					}
					return this->createShot(target);
				}
			}
			return nullptr;
		}

		const Enemy* Tower::findTarget(const std::vector<std::unique_ptr<Enemy>>& enemies) const {
			// Firing method is ignored here
			const bool use_highest = this->getBaseType()->getTargetingStrategy().getProtocol()
				== TargetingStrategyProtocols::Highest;
			// The fall-back value is the enemy that is closest/farthest from the tower
			// but within its firing range.
			double fallback_winning_value = use_highest ? 0.0 : this->getBaseType()->getFiringRange() + 1.0;
			const Enemy* fallback_winner {nullptr};
			// Used for the statistics and names targeting strategy
			[[maybe_unused]] double target_winning_value = use_highest ? 0.0 : 1e20;
			[[maybe_unused]] const Enemy* target_winner {nullptr};
			const auto my_strat = this->getBaseType()->getTargetingStrategy().getStrategy();
			bool use_fallback = true;
			for (const auto& e : enemies) {
				const double gdx = math::get_abs(this->getGameX() - e->getGameX());
				const double gdy = std::abs(this->getGameY() - e->getGameY());
				const double gdist = std::sqrt(gdx * gdx + gdy * gdy);
				if (gdist <= this->getBaseType()->getFiringRange()) {
					// Valid target
					if (my_strat == TargetingStrategyTypes::Distances || !target_winner) {
						if ((use_highest && gdist > fallback_winning_value)
							|| (!use_highest && gdist < fallback_winning_value)) {
							fallback_winning_value = gdist;
							fallback_winner = e.get();
						}
					}
					if (my_strat == TargetingStrategyTypes::Names) {
						for (auto& n : this->getBaseType()->getTargetingStrategy().getTargetNames()) {
							if (e->getBaseType().getName() == n) {
								if (!target_winner) {
									target_winner = e.get();
									target_winning_value = gdist;
									use_fallback = false;
								}
								else if ((use_highest && gdist > target_winning_value)
									|| (!use_highest && gdist < target_winning_value)) {
									target_winner = e.get();
									target_winning_value = gdist;
								}
							}
						}
					} // Names Targeting Strategy
					else if (my_strat == TargetingStrategyTypes::Statistics) {
						auto test_stat = this->getBaseType()->getTargetingStrategy().getTestStatistic();
						const double e_stat_value =
							test_stat == TargetingStrategyStatistics::Damage ? e->getBaseType().getDamage() :
							test_stat == TargetingStrategyStatistics::Health ? e->getHealth() :
							test_stat == TargetingStrategyStatistics::Armor_Health ? e->getArmorHealth() :
							test_stat == TargetingStrategyStatistics::Armor_Reduce ? e->getBaseType().getArmorReduce() :
							test_stat == TargetingStrategyStatistics::Speed ? e->getCurrentSpeed() :
							test_stat == TargetingStrategyStatistics::Buffs ? e->getBaseType().getBuffTypesCount() :
							throw std::runtime_error {"Invalid targeting strategy encountered in Tower::findTarget!"};
						if ((use_highest && e_stat_value > target_winning_value)
							|| (!use_highest && e_stat_value < target_winning_value)) {
							use_fallback = false;
							target_winning_value = e_stat_value;
							target_winner = e.get();
							fallback_winning_value = gdist;
						}
						else if (e_stat_value == target_winning_value
							&& ((use_highest && gdist > fallback_winning_value)
								|| (!use_highest && gdist < fallback_winning_value))) {
							use_fallback = true;
							fallback_winning_value = gdist;
							fallback_winner = e.get();
						}
					} // Statistics Targeting Strategy
				}
			}
			return use_fallback ? fallback_winner : target_winner;
		}

		std::unique_ptr<Shot> Tower::createShot(const Enemy* target) const {
			// Determine which shot to fire...
			const auto roll = rng::distro_uniform(rng::gen);
			double running_total = 0.0;
			std::shared_ptr<ShotBaseType> stype = nullptr;
			for (auto& st_pair : this->getBaseType()->getShotTypes()) {
				running_total += st_pair.second;
				if (roll <= running_total) {
					stype = st_pair.first;
					break;
				}
			}
			if (!stype) {
				// Try recursion for now... If this doesn't work well, I'll think of
				// something else...
				return this->createShot(target);
			}
			// Consider firing method
			// For now, I'm implementing pulse the same as static...
			const auto method = this->getBaseType()->getFiringMethod().getMethod();
			if (method == FiringMethodTypes::Default) {
				const double gdx = target->getGameX() - this->getGameX();
				const double gdy = target->getGameY() - this->getGameY();
				const double theta = std::atan2(gdy, gdx);
				return std::make_unique<Shot>(this->device_resources, stype, graphics::Color {1.f, 0.f, 1.f, 1.f},
					*this, theta);
			}
			++this->angle_index;
			this->angle_index %= this->getBaseType()->getFiringMethod().getAngles().size();
			return std::make_unique<Shot>(this->device_resources, stype, graphics::Color {1.f, 0.f, 1.f, 1.f},
				*this, this->getBaseType()->getFiringMethod().getAngles().at(this->angle_index));
		}
	}
}