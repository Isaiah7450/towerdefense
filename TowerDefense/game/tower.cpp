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
		void Tower::draw(const graphics::Renderer2D& renderer) const noexcept {
			GameObject::draw(renderer);
			if (this->getBaseType()->getVolleyShots() == 0) {
				return;
			}
			// Draw ammunition bar
			constexpr const graphics::Color bar_outline_color {0.2f, 0.2f, 0.2f, 1.0f};
			constexpr const graphics::Color bar_empty_color {0.8f, 0.8f, 0.8f, 0.6f};
			constexpr const graphics::Color bar_filling_color {1.f, 0.f, 1.f, 0.9f};
			constexpr const graphics::Color bar_reloading_color {0.3f, 0.6f, 0.6f, 0.7f};
			const float shots_fired_percent = static_cast<float>(this->shots_fired_since_reload)
				/ static_cast<float>(this->getBaseType()->getVolleyShots());
			const float reload_time_percent = static_cast<float>(this->frames_to_reload)
				/ static_cast<float>(math::convertMillisecondsToFrames(this->getBaseType()->getReloadDelay()));
			const float bar_max_width = 0.7f * graphics::getGameSquareWidth<float>();
			const float bar_height = 0.15f * graphics::getGameSquareHeight<float>();
			const float ammo_bar_offset = 8.f * graphics::screen_height / 645.f;
			const float ammo_bar_start_x = static_cast<float>(this->getScreenX()) - bar_max_width / 2.f;
			const float ammo_bar_start_y = static_cast<float>(this->getScreenY()) - bar_height / 2.f - ammo_bar_offset;
			const float ammo_bar_end_x = ammo_bar_start_x + bar_max_width * shots_fired_percent;
			const float ammo_bar_end_y = ammo_bar_start_y + bar_height;
			const D2D_RECT_F ammo_bar_filled_rc {ammo_bar_start_x, ammo_bar_start_y, ammo_bar_end_x, ammo_bar_end_y};
			const D2D_RECT_F ammo_bar_outline_rc {ammo_bar_start_x, ammo_bar_start_y,
				ammo_bar_start_x + bar_max_width, ammo_bar_end_y};
			renderer.setBrushColors(bar_outline_color, bar_empty_color);
			renderer.outlineRectangle(ammo_bar_outline_rc);
			renderer.fillRectangle(ammo_bar_outline_rc);
			renderer.setFillColor(bar_filling_color);
			renderer.fillRectangle(ammo_bar_filled_rc);
			if (reload_time_percent > 0) {
				const float reload_bar_start_x = static_cast<float>(this->getScreenX()) + bar_max_width / 2.f
					- (bar_max_width * (1.f - reload_time_percent));
				const float reload_bar_end_x = static_cast<float>(this->getScreenX()) + bar_max_width / 2.f;
				const D2D_RECT_F reload_bar_filled_rc {reload_bar_start_x, ammo_bar_start_y,
					reload_bar_end_x, ammo_bar_end_y};
				renderer.setFillColor(bar_reloading_color);
				renderer.fillRectangle(reload_bar_filled_rc);
			}
		}

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
				this->frames_til_next_shot += math::convertMillisecondsToFrames(1000.0 / this->getBaseType()->getFiringSpeed());
				auto target = this->findTarget(enemies);
				if (!target) {
					// Take the time to reload a single shot instead of firing
					if (this->shots_fired_since_reload > 0) {
						--this->shots_fired_since_reload;
					}
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
			const auto& my_method = this->getBaseType()->getFiringMethod();
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
				const double signed_gdx = e->getGameX() - this->getGameX();
				const double signed_gdy = e->getGameY() - this->getGameY();
				// (Values during input are specified from [0...2 pi], so this fixes the range.
				const double e_angle_from_tower = std::atan2(signed_gdy, signed_gdx);
				const double adjusted_angle = e_angle_from_tower >= 0 ? e_angle_from_tower
					: math::pi - e_angle_from_tower;
				if (my_method.getMethod() != FiringMethodTypes::Default) {
					if (adjusted_angle < my_method.getMinimumAngle()
						|| adjusted_angle > my_method.getMaximumAngle()) {
						// Not within firing angle of the tower
						continue;
					}
				}
				const double gdx = math::get_abs(signed_gdx);
				const double gdy = std::abs(signed_gdy);
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