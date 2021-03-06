// File Author: Isaiah Hoffman
// File Created: May 21, 2018
#include <string>
#include <queue>
#include <array>
#include <vector>
#include <memory>
#include <cmath>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../graphics/graphics.hpp"
#include "./../pathfinding/graph_node.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./enemy_type.hpp"
#include "./enemy.hpp"
#include "./game_object.hpp"
#include "./game_util.hpp"
#include "./my_game.hpp"
#include "./status_effects.hpp"
using namespace std::literals::string_literals;
namespace hoffman_isaiah {
	namespace game {
		// enemy_type.hpp
		void BuffBase::update(const Enemy& caller, std::vector<std::unique_ptr<Enemy>>& targets) {
			++this->frames_since_last_tick;
			if (this->isTimeToApply()) {
				this->frames_since_last_tick -= this->frames_between_buff_ticks;
				for (const auto& e : targets) {
					if (this->isValidTarget(caller, *e)) {
						this->apply(*e);
					}
				}
			}
		}

		bool BuffBase::isValidTarget(const Enemy& caller, const Enemy& target) const {
			const double dx = caller.getGameX() - target.getGameX();
			const double dy = caller.getGameY() - target.getGameY();
			if (std::sqrt(dx * dx + dy * dy) <= this->getRadius()) {
				// Check name
				auto valid_names = this->getTargetNames();
				for (const auto& n : valid_names) {
					if (target.getBaseType().getName() == n) {
						return true;
					}
				}
			}
			return false;
		}

		void SmartBuff::apply(Enemy& target) {
			// Apply buff by adding status effect
			auto my_smart_effect = std::make_unique<SmartStrategyEffect>(this->getBuffDuration(),
				pathfinding::HeuristicStrategies::Diagonal, true);
			target.addStatus(std::move(my_smart_effect));
		}

		void SpeedBuff::apply(Enemy& target) {
			// Apply buff by adding status effect
			auto my_speed_effect = std::make_unique<SpeedBoostEffect>(this->getBuffDuration(),
				this->getWalkingBoost(), this->getRunningBoost(), this->getInjuredBoost());
			target.addStatus(std::move(my_speed_effect));
		}

		void HealerBuff::apply(Enemy& target) {
			target.heal(this->getHealAmount());
		}

		void PurifyBuff::apply(Enemy& target) {
			std::vector<int> statuses_to_remove {};
			auto& active_statuses = target.getActiveStatuses();
			// Determine which statuses to remove
			for (unsigned int i = 0; i < active_statuses.size(); ++i) {
				if (!active_statuses[i]->isPositiveEffect()) {
					active_statuses[i]->clearEffects(target);
					statuses_to_remove.emplace_back(i);
					if (statuses_to_remove.size() >= static_cast<unsigned>(this->getMaxEffectsRemoved())) {
						break;
					}
				}
			}
			// Actually remove the statuses
			for (unsigned int i = 0; i < statuses_to_remove.size(); ++i) {
				active_statuses.erase(active_statuses.begin() + (statuses_to_remove[i] - i));
			}
		}

		void RepairBuff::apply(Enemy& target) {
			target.repair(this->getRepairAmount());
		}

		void ForcefieldBuff::apply(Enemy& target) {
			target.addShield(this->getShieldHealth(), this->getShieldAbsorb());
			auto my_shield_effect = std::make_unique<ShieldEffect>(this->getBuffDuration(), this->getShieldHealth());
			target.addStatus(std::move(my_shield_effect));
		}

		// enemy.hpp
		Enemy::StatusResistance::StatusResistance(StatusEffects effect, double resist, int ms_until_expire) noexcept :
			status_effect {effect},
			status_resist {resist},
			frames_until_expire {math::convertMillisecondsToFrames(ms_until_expire)} {
		}

		void Enemy::StatusResistance::update() {
			if (this->frames_until_expire > 0) {
				--this->frames_until_expire;
				if (this->frames_until_expire <= 0) {
					this->status_resist = 0;
					this->frames_until_expire = 0;
				}
			}
		}

		void Enemy::StatusResistance::increaseResist() {
			++this->num_times;
			if (this->isActive()) {
				this->frames_until_expire *= 1.5;
				this->status_resist *= 1.75;
				this->status_resist += 0.02 * this->num_times;
			}
			else {
				this->frames_until_expire = math::convertMillisecondsToFrames(750);
				this->status_resist = 0.1 + 0.03 * this->num_times;
			}
		}

		Enemy::Enemy(graphics::DX::DeviceResources2D* dev_res, const GameMap& my_map,
			const EnemyType* etype, graphics::Color o_color,
			const GameMap& gmap, int level, double difficulty, int challenge_level) :
			GameObject {dev_res, my_map, etype->getShape(), o_color, etype->getColor(),
			gmap.getTerrainGraph(etype->isFlying()).getStartNode()->getGameX() + 0.5,
			gmap.getTerrainGraph(etype->isFlying()).getStartNode()->getGameY() + 0.5,
			Enemy::gwidth, Enemy::gheight},
			base_type {etype},
			my_pathfinder {gmap, etype->isFlying(), etype->canMoveDiagonally(), etype->getDefaultStrategy()},
			my_path {},
			current_node {nullptr},
			current_direction {0.0},
			current_health {Enemy::getAdjustedHealth(etype->getBaseHealth(), level, difficulty, challenge_level)},
			maximum_health {Enemy::getAdjustedHealth(etype->getBaseHealth(), level, difficulty, challenge_level)},
			current_armor_health {Enemy::getAdjustedArmorHealth(etype->getBaseArmorHP(), level, difficulty, challenge_level)},
			maximum_armor_health {Enemy::getAdjustedArmorHealth(etype->getBaseArmorHP(), level, difficulty, challenge_level)},
			walking_speed {Enemy::getAdjustedSpeed(etype->getBaseWalkingSpeed(), level, difficulty, challenge_level)},
			running_speed {Enemy::getAdjustedSpeed(etype->getBaseRunningSpeed(), level, difficulty, challenge_level)},
			injured_speed {Enemy::getAdjustedSpeed(etype->getBaseInjuredSpeed(), level, difficulty, challenge_level)},
			current_strat {etype->getDefaultStrategy()},
			move_diagonally {etype->canMoveDiagonally()},
			status_effects {},
			buffs {} {
			if (this->getBaseType().isUnique()) {
				this->scale(Enemy::unique_enemy_scale);
			}
			// Get path
			this->my_pathfinder.findPath(challenge_level / 10.0);
			this->my_path = this->my_pathfinder.getPath();
			this->current_node = &this->my_path.front();
			this->my_path.pop();
			this->changeDirection();
			this->addEnemyBuffs();
		}

		Enemy::Enemy(graphics::DX::DeviceResources2D* dev_res, const GameMap& my_map,
			const EnemyType* etype, graphics::Color o_color,
			pathfinding::Pathfinder pf, double start_gx, double start_gy,
			int level, double difficulty, int challenge_level) :
			GameObject {dev_res, my_map, etype->getShape(), o_color, etype->getColor(),
			start_gx, start_gy, Enemy::gwidth, Enemy::gheight},
			base_type {etype},
			my_pathfinder {pf},
			my_path {pf.getPath()},
			current_node {nullptr},
			current_direction {0.0},
			current_health {Enemy::getAdjustedHealth(etype->getBaseHealth(), level, difficulty, challenge_level)},
			maximum_health {Enemy::getAdjustedHealth(etype->getBaseHealth(), level, difficulty, challenge_level)},
			current_armor_health {Enemy::getAdjustedArmorHealth(etype->getBaseArmorHP(), level, difficulty, challenge_level)},
			maximum_armor_health {Enemy::getAdjustedArmorHealth(etype->getBaseArmorHP(), level, difficulty, challenge_level)},
			walking_speed {Enemy::getAdjustedSpeed(etype->getBaseWalkingSpeed(), level, difficulty, challenge_level)},
			running_speed {Enemy::getAdjustedSpeed(etype->getBaseRunningSpeed(), level, difficulty, challenge_level)},
			injured_speed {Enemy::getAdjustedSpeed(etype->getBaseInjuredSpeed(), level, difficulty, challenge_level)},
			current_strat {etype->getDefaultStrategy()},
			move_diagonally {etype->canMoveDiagonally()},
			status_effects {},
			buffs {} {
			if (this->getBaseType().isUnique()) {
				this->scale(Enemy::unique_enemy_scale);
			}
			this->current_node = &this->my_path.front();
			this->my_path.pop();
			this->changeDirection();
			this->addEnemyBuffs();
		}

		bool Enemy::update() {
			// Check if we are still alive
			if (!this->isAlive()) {
				return true;
			}
			// Update status resistances.
			for (auto& my_resist : this->status_resists) {
				my_resist.second.update();
			}
			// Apply buffs
			for (auto& b : this->buffs) {
				b->update(*this, game::g_my_game->getEnemies());
			}
			// Apply status effects
			std::vector<int> statuses_to_remove {};
			for (unsigned int i = 0; i < this->status_effects.size(); ++i) {
				if (this->status_effects[i]->update(*this)) {
					const auto* my_status = this->status_effects[i].get();
					const auto my_effect_type = my_status->getStatusEffectType();
					if (!my_status->isPositiveEffect()) {
						// Check for status resistances.
						auto my_iterator = this->status_resists.find(my_effect_type);
						if (my_iterator == this->status_resists.end()) {
							this->status_resists.emplace(my_effect_type, Enemy::StatusResistance {my_effect_type, 0.10, 1000});
						}
						else {
							my_iterator->second.increaseResist();
						}
					}
					statuses_to_remove.emplace_back(i);
				}
			}
			// Remove old status effects
			for (unsigned int i = 0; i < statuses_to_remove.size(); ++i) {
				this->status_effects.erase(this->status_effects.begin() + (statuses_to_remove[i] - i));
			}
			// Perform movement
			const double my_speed = this->getCurrentSpeed() / game::logic_framerate /
				this->current_node->getWeight();
			const double rx = my_speed;
			const double ry = my_speed;
			if (!this->isStunned()) {
				this->translate(rx * std::cos(this->current_direction), ry * std::sin(this->current_direction));
			}
			const double dx = (this->getNextNode().getGameX() + 0.5) - this->getGameX();
			const double dy = (this->getNextNode().getGameY() + 0.5) - this->getGameY();
			const bool update_next_node = std::sqrt(dx * dx + dy * dy) <= my_speed + 0.05 / game::logic_framerate;
			if (update_next_node) {
				// Change path
				if (this->my_path.size() == 1) {
					return true;
				}
				this->current_node = &this->getNextNode();
				this->my_path.pop();
				this->changeDirection();
			}
			// Reset speed multipliers to normal
			this->speed_multiplier = 1.0;
			this->speed_boosts = {1.0, 1.0, 1.0};
			return !this->isAlive();
		}

		void Enemy::draw(const graphics::Renderer2D& renderer) const noexcept {
			// Call base class member first
			GameObject::draw(renderer);
			// Only draw bars if injured.
			if (this->getHealthPercentage() >= 0.99 && (!this->hasArmor() || this->getArmorPercentage() >= 0.99)) {
				return;
			}
			// Draw health bars
			constexpr const graphics::Color bar_outline_color {0.2f, 0.2f, 0.2f, 1.0f};
			constexpr const graphics::Color bar_empty_color {0.7f, 0.7f, 0.7f, 1.0f};
			constexpr const graphics::Color health_fill_color {0.8f, 0.0f, 0.0f, 0.9f};
			constexpr const graphics::Color armor_fill_color {0.0f, 0.0f, 0.8f, 0.9f};
			constexpr const graphics::Color shield_fill_color {0.0, 0.8f, 0.8f, 0.9f};
			const double hp_percent = this->getHealthPercentage();
			const float bar_max_width = this->getBaseType().isUnique()
				? 0.8f * this->getGameMap().getGameSquareWidth<float>()
				: 0.4f * this->getGameMap().getGameSquareWidth<float>();
			const float bar_height = 0.115f * this->getGameMap().getGameSquareHeight<float>();
			const float hp_bar_offset = 3.23f * bar_height;
			// Roughtly 3.23 * bar_height
			// const float hp_bar_offset = 5.5f * graphics::screen_height / 645.f;
			const float hp_bar_start_x = static_cast<float>(this->getScreenX()) - bar_max_width / 2.f;
			const float hp_bar_end_x = static_cast<float>(hp_bar_start_x + hp_percent * bar_max_width);
			const float hp_bar_start_y = static_cast<float>(this->getScreenY()) - bar_height / 2.f - hp_bar_offset;
			const float hp_bar_end_y = hp_bar_start_y + bar_height;
			const D2D1_RECT_F hp_bar_filled_rc {hp_bar_start_x, hp_bar_start_y, hp_bar_end_x, hp_bar_end_y};
			const D2D1_RECT_F hp_bar_outline_rc {hp_bar_start_x, hp_bar_start_y, hp_bar_start_x + bar_max_width,
				hp_bar_end_y};
			renderer.setBrushColors(bar_outline_color, bar_empty_color);
			renderer.outlineRectangle(hp_bar_outline_rc);
			renderer.setFillColor(health_fill_color);
			renderer.fillRectangle(hp_bar_filled_rc);
			if (this->hasArmor() && this->getArmorPercentage() <= 0.99) {
				const double ahp_percent = this->getArmorPercentage();
				const float ahp_bar_offset = hp_bar_offset + bar_height + 0.5f * bar_height;
				// Roughly hp_bar_offset + 0.5 * bar_height + bar_height
				// const float ahp_bar_offset = hp_bar_offset + bar_height + 0.85f * graphics::screen_height / 645.f;
				const float ahp_bar_start_x = hp_bar_start_x;
				const float ahp_bar_end_x = static_cast<float>(ahp_bar_start_x + ahp_percent * bar_max_width);
				const float ahp_bar_start_y = static_cast<float>(this->getScreenY()) - bar_height / 2.f - ahp_bar_offset;
				const float ahp_bar_end_y = ahp_bar_start_y + bar_height;
				const D2D1_RECT_F ahp_bar_filled_rc {ahp_bar_start_x, ahp_bar_start_y, ahp_bar_end_x, ahp_bar_end_y};
				const D2D1_RECT_F ahp_bar_outline_rc {ahp_bar_start_x, ahp_bar_start_y, ahp_bar_start_x + bar_max_width,
					ahp_bar_end_y};
				renderer.setFillColor(bar_empty_color);
				renderer.outlineRectangle(ahp_bar_outline_rc);
				renderer.setFillColor(armor_fill_color);
				renderer.fillRectangle(ahp_bar_filled_rc);
			}
			if (this->getShieldHealth() > 0) {
				const double shp_percent = this->getShieldHealth() / this->getMaxShieldHealth();
				// const float shp_bar_offset = hp_bar_offset + bar_height + 0.85f * graphics::screen_height / 645.f;
				const float shp_bar_offset = hp_bar_offset + bar_height + 0.5f * bar_height;
				const float shp_bar_start_x = hp_bar_start_x;
				const float shp_bar_end_x = static_cast<float>(shp_bar_start_x + shp_percent * bar_max_width);
				const float shp_bar_start_y = static_cast<float>(this->getScreenY()) - bar_height / 2.f - shp_bar_offset;
				const float shp_bar_end_y = shp_bar_start_y + bar_height;
				const D2D1_RECT_F shp_bar_filled_rc {shp_bar_start_x, shp_bar_start_y, shp_bar_end_x, shp_bar_end_y};
				const D2D1_RECT_F shp_bar_outline_rc {shp_bar_start_x, shp_bar_start_y, shp_bar_start_x + bar_max_width,
					shp_bar_end_y};
				renderer.setFillColor(shield_fill_color);
				renderer.fillRectangle(shp_bar_filled_rc);
			}
		}

		void Enemy::takeDamage(double dmg, double wap, bool bypass_armor_completely) {
			// Forcefield effect.
			if (this->getShieldHealth() > 0 && !bypass_armor_completely) {
				const auto absorbed_dmg = dmg > this->getShieldAbsorb() ? this->getShieldAbsorb() : dmg;
				dmg -= absorbed_dmg;
				this->degradeShield(absorbed_dmg);
			}
			if (this->hasArmor()) {
				// Damage to armor => Base Damage * ((1.0 - Armor Reduce) + Piercing)
				// to a maximum of the base damage amount
				const double ar_protect = math::get_max(this->getBaseType().getArmorReduce() - wap,
					0.0);
				const double ar_dmg = dmg * (1.0 - ar_protect);
				if (wap > 0) {
					// Damage to health => Armor Damage * Piercing
					const double hp_dmg = ar_dmg * wap;
					this->current_health -= hp_dmg;
				}
				if (!bypass_armor_completely) {
					this->current_armor_health -= ar_dmg;
				}
			}
			else {
				// Armor Piercing ignored
				this->current_health -= dmg;
			}
		}

		void Enemy::heal(double amt) {
			this->current_health = math::get_min(this->getMaxHealth(), this->getHealth() + amt);
		}

		void Enemy::repair(double amt) {
			this->current_armor_health = math::get_min(this->getMaxArmorHealth(), this->getArmorHealth() + amt);
		}

		void Enemy::addStatus(std::unique_ptr<StatusEffectBase>&& effect) {
			if (!effect->isPositiveEffect()) {
				const auto my_iterator = this->status_resists.find(effect->getStatusEffectType());
				if (my_iterator != this->status_resists.end()) {
					const auto resist_chance = my_iterator->second.isActive()
						? my_iterator->second.getResistance() : 0;
					const auto my_roll = rng::distro_uniform(rng::gen);
					if (my_roll <= resist_chance) {
						// Enemy resists the status affliction!
						return;
					}
				}
			}
			this->status_effects.emplace_back(std::move(effect));
		}

		void Enemy::changeStrategy(const GameMap& gmap, pathfinding::HeuristicStrategies new_strat, bool diag_move) {
			UNREFERENCED_PARAMETER(gmap);
			if (this->my_path.size() == 1) {
				// It kinda is pointless to do here...
				// It also solves one bug where the enemy tries
				// to change strategies endlessly when they reach
				// the goal.
				return;
			}
			this->current_strat = new_strat;
			this->move_diagonally = diag_move;
			// Obtain new path
			this->my_pathfinder.setStrategy(new_strat, diag_move);
			// How can I optimize this?
			this->my_pathfinder.findPath(game::g_my_game->getChallengeLevel() / 10.0,
				static_cast<int>(std::floor(this->getGameX())),
				static_cast<int>(std::floor(this->getGameY())));
			this->current_node = &this->my_path.front();
			this->my_path.pop();
			this->changeDirection();
		}

		void Enemy::changeDirection() noexcept {
			const double dx = (this->getNextNode().getGameX() + 0.5) - this->getGameX();
			const double dy = (this->getNextNode().getGameY() + 0.5) - this->getGameY();
			this->current_direction = std::atan2(dy, dx);
		}
	}
}