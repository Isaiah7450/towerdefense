// File Author: Isaiah Hoffman
// File Created: June 4, 2018
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>
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
#include "./my_game.hpp"

namespace hoffman::isaiah {
	namespace game {
		void Tower::draw(const graphics::Renderer2D& renderer) const noexcept {
			GameObject::draw(renderer);
			// Draw radius if appropriate
			if (!this->getBaseType()->isWall() && this->show_coverage) {
				renderer.setOutlineColor(graphics::Color {0.7f, 0.f, 0.f, 0.8f});
				renderer.outlineEllipse(renderer.createEllipse(static_cast<float>(this->getScreenX()),
					static_cast<float>(this->getScreenY()),
					static_cast<float>(this->getFiringRange() * graphics::getGameSquareWidth()),
					static_cast<float>(this->getFiringRange() * graphics::getGameSquareHeight())));
				// Also paint upgrade level.
				constexpr const graphics::Color text_color {1.f, 0.2f, 1.f, 1.f};
				if (this->getLevel() > 1) {
					renderer.drawText(std::to_wstring(this->getLevel()), text_color,
						renderer.createRectangle(static_cast<float>(graphics::convertToScreenX(this->getGameX() - 0.5)),
							static_cast<float>(graphics::convertToScreenY(this->getGameY() - 0.5)),
							static_cast<float>(graphics::getGameSquareWidth()),
							static_cast<float>(graphics::getGameSquareHeight())));
				}
			}
			if (this->getVolleyShots() == 0) {
				return;
			}
			// Draw ammunition bar
			constexpr const graphics::Color bar_outline_color {0.2f, 0.2f, 0.2f, 1.0f};
			constexpr const graphics::Color bar_empty_color {0.8f, 0.8f, 0.8f, 0.6f};
			constexpr const graphics::Color bar_filling_color {1.f, 0.f, 1.f, 0.9f};
			constexpr const graphics::Color bar_reloading_color {0.3f, 0.6f, 0.6f, 0.7f};
			const float shots_fired_percent = static_cast<float>(this->shots_fired_since_reload)
				/ static_cast<float>(this->getVolleyShots());
			const float reload_time_percent = static_cast<float>(this->frames_to_reload)
				/ static_cast<float>(math::convertMillisecondsToFrames(this->getReloadDelay()));
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

		std::vector<std::unique_ptr<Shot>> Tower::update(const std::vector<std::unique_ptr<Enemy>>& enemies) {
			std::vector<std::unique_ptr<Shot>> my_shots {};
			if (this->getBaseType()->isWall()) {
				return my_shots;
			}
			if (this->must_reload) {
				--this->frames_to_reload;
				if (this->frames_to_reload <= 0) {
					this->must_reload = false;
					this->shots_fired_since_reload = 0;
				}
				else return my_shots;
			}
			--this->frames_til_next_shot;
			if (this->frames_til_next_shot <= 0.0) {
				this->frames_til_next_shot += math::convertMillisecondsToFrames(1000.0
					/ this->getFiringSpeed());
				auto target = this->findTarget(enemies);
				if (!target) {
					// Take the time to reload a single shot instead of firing
					if (this->shots_fired_since_reload > 0) {
						--this->shots_fired_since_reload;
					}
				}
				else {
					++this->shots_fired_since_reload;
					if (this->shots_fired_since_reload >= this->getVolleyShots()
						&& this->getVolleyShots() != 0) {
						this->frames_to_reload += math::convertMillisecondsToFrames(this->getReloadDelay());
						// Fast reload effect.
						const auto my_fast_reload_ability = this->upgrade_specials.find(TowerUpgradeSpecials::Fast_Reload);
						if (my_fast_reload_ability != this->upgrade_specials.cend()) {
							const auto my_roll = rng::distro_uniform(rng::gen);
							if (my_roll <= my_fast_reload_ability->second.first) {
								this->frames_to_reload *= math::get_max(1.0 - my_fast_reload_ability->second.second, 0.0);
							}
						}
						this->must_reload = this->frames_to_reload > 0;
					}
					// Multishot effect.
					const auto my_multishot_ability = this->upgrade_specials.find(TowerUpgradeSpecials::Multishot);
					if (my_multishot_ability != this->upgrade_specials.cend()) {
						const auto my_roll = rng::distro_uniform(rng::gen);
						if (my_roll <= my_multishot_ability->second.first) {
							for (int i = 0; i < static_cast<int>(my_multishot_ability->second.second); ++i) {
								my_shots.emplace_back(this->createShot(target));
							}
						}
					}
					my_shots.emplace_back(this->createShot(target));
					return my_shots;
				}
			}
			return my_shots;
		}

		const Enemy* Tower::findTarget(const std::vector<std::unique_ptr<Enemy>>& enemies) const {
			const auto& my_method = this->getBaseType()->getFiringMethod();
			const bool use_highest = this->getBaseType()->getTargetingStrategy().getProtocol()
				== TargetingStrategyProtocols::Highest;
			// The fall-back value is the enemy that is closest/farthest from the tower
			// but within its firing range.
			double fallback_winning_value = use_highest ? 0.0 : this->getFiringRange() + 1.0;
			const Enemy* fallback_winner {nullptr};
			// Used for the statistics and names targeting strategy
			[[maybe_unused]] double target_winning_value = use_highest ? 0.0 : 1e20;
			[[maybe_unused]] const Enemy* target_winner {nullptr};
			const auto my_strat = this->getBaseType()->getTargetingStrategy().getStrategy();
			bool use_fallback = true;
			for (const auto& e : enemies) {
				const double signed_gdx = e->getGameX() - this->getGameX();
				const double signed_gdy = e->getGameY() - this->getGameY();
				const double e_angle_from_tower = std::atan2(-signed_gdy, signed_gdx);
				const double adjusted_angle = e_angle_from_tower;
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
				if (gdist <= this->getFiringRange()) {
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
			// Mega missile special ability.
			const auto my_mega_missile_ability = this->upgrade_specials.find(TowerUpgradeSpecials::Mega_Missile);
			if (my_mega_missile_ability != this->upgrade_specials.cend()) {
				const auto ability_roll = rng::distro_uniform(rng::gen);
				if (ability_roll <= my_mega_missile_ability->second.first) {
					// Probably should try to refactor this to not use global state in the future.
					stype = game::g_my_game->getShotType(L"Mega Missile");
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
			// Note that the y-axis is inverted, so we need to correct the angle used here.
			return std::make_unique<Shot>(this->device_resources, stype, graphics::Color {1.f, 0.f, 1.f, 1.f},
				*this, -this->getBaseType()->getFiringMethod().getAngles().at(this->angle_index));
		}
	}
}