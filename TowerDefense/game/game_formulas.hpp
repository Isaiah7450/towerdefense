#pragma once
// Author: Isaiah Hoffman
// Created: March 17, 2019
#include <cmath>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include "./../ih_math.hpp"
namespace hoffman::isaiah::game {
	class FiringMethod;
	class TargetingStrategy;
	class ShotBaseType;
	class Enemy;
	class EnemyType;
	class BuffBase;

	namespace towers {
		/// <param name="fr">The firing range of the tower.</param>
		/// <param name="fm">The firing method of the tower.</param>
		/// <param name="is_wall">Is this tower considered a wall?</param>
		/// <returns>The approximate area covered by the tower.</returns>
		double getFiringArea(double fr, const FiringMethod& fm, bool is_wall) noexcept;
		/// <param name="stypes">The shot types and related frequencies that the tower fires.</param>
		/// <param name="dm">The tower's damage multiplier.</param>
		/// <returns>The expected amount of damage output by the twoer.</returns>
		double getAverageDamagePerShot(const std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>& stypes, double dm) noexcept;
		/// <param name="stypes">The shot types and related frequencies that the tower fires.</param>
		/// <returns>The weighted average of the tower's shot types' effect ratings.</returns>
		double getAverageShotEffectRating(const std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>& stypes) noexcept;
		/// <param name="stypes">The shot types and related frequencies that the tower fires.</param>
		/// <returns>The weighted average of the tower's shot types' ratings.</returns>
		double getAverageShotRating(const std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>& stypes) noexcept;
		/// <param name="fs">The tower's firing speed in shots / second.</param>
		/// <param name="vs">The tower's volley shots.</param>
		/// <param name="rd">The tower's reload delay in milliseconds.</param>
		/// <param name="is_wall">Is this tower considered a wall?</param>
		/// <returns>The tower's average rate of fire as the average number of shots fired per second.</returns>
		inline double getRateOfFire(double fs, int vs, int rd, bool is_wall) noexcept {
			if (is_wall) return 0.0;
			if (vs == 0 || rd == 0) return fs;
			// The average rate of fire is the total number of shots fired
			// in one cycle divided by the total time in that cycle where
			// one cycle is defined as firing an entire volley and reloading
			// that volley completely.
			const double secs_to_reload = rd / 1000.0;
			const double total_cycle_time = (1.0 / fs) * vs + secs_to_reload;
			return static_cast<double>(vs) / total_cycle_time;
		}
		/// <param name="avg_dmg">The expected damage per shot of the tower.</param>
		/// <param name="rate_of_fire">The rate of fire of the tower.</param>
		/// <returns>The tower's expected DPS.</returns>
		inline double getExpectedDPS(double avg_dmg, double rate_of_fire) noexcept {
			return avg_dmg * rate_of_fire;
		}
		/// <param name="rate_of_fire">The rate of fire of the tower.</param>
		/// <param name="firing_range'>The firing range of the tower.</param>
		/// <param name="firing_area">The total firing area covered by the tower.</param>
		/// <param name="fm">The firing method of the tower.</param>
		/// <param name="ts">The targeting strategy of the tower.</param>
		/// <param name="avg_dmg">The expected damage per shot of the tower.</param>
		/// <param name="avg_effect_rating">The average shot effect rating of the tower.</param>
		/// <param name="is_wall">Is this tower considered a wall?</param>
		/// <returns>The tower's overall rating.</returns>
		double getRating(double rate_of_fire, double firing_range, double firing_area, const FiringMethod& fm, const TargetingStrategy& ts,
			double avg_dmg, double avg_effect_rating, bool is_wall) noexcept;
		/// <param name="rating">The tower's rating.</param>
		/// <param name="cost_adjust">The tower's cost adjustment.</param>
		/// <param name="is_wall">Is this tower considered a wall?</param>
		/// <returns>The cost of the tower.</returns>
		inline double getCost(double rating, int cost_adjust, bool is_wall) noexcept {
			return is_wall ? static_cast<double>(cost_adjust) : cost_adjust + rating / 8.2 + 1.0;
		}
	}

	namespace enemy_buffs {
		// Is this namespace approach the best way? Probably not, but convenient for naming purposes.
		namespace buff_base {
			/// <param name="target_names">The list of targets that the buff affects.</param>
			/// <param name="etypes">The mapping of enemy types in the game.</param>
			/// <returns>The average base rating of the enemies the buff affects.</returns>
			double getAverageInfluenceRating(std::vector<std::wstring> target_names,
				const std::map<std::wstring, std::shared_ptr<EnemyType>>& etypes);
			/// <param name="br">The radius of the buff.</param>
			/// <param name="delay_ms">The delay between applications of the buff (in milliseconds).</param>
			/// <param name="target_names_size">The number of different target names the buff is valid for.</param>
			/// <param name="avg_influence_rating">The average rating of the enemies the buff affects.</param>
			/// <returns>The basic rating of the buff as determined by its
			/// radius of influence, time between activations, and the number of enemies it affects.</returns>
			inline double getBaseRating(double br, double delay_ms, size_t target_names_size, double avg_influence_rating) noexcept {
				return br * br * std::sqrt(target_names_size) * avg_influence_rating * (1000.0 / delay_ms);
			}
		}

		namespace temp_buff_base {
			/// <param name="parent_base_rating">The base rating of the buff as determined by the parent class.</param>
			/// <param name="bd">The duration of the buff in milliseconds.</param>
			/// <returns>The basic rating of the buff as determined by its
			/// radius of influence, time between activations, and the number of enemies it affects.</returns>
			inline double getBaseRating(double parent_base_rating, int bd) noexcept {
				return parent_base_rating * (bd / 1000.0);
			}
		}

		namespace smart_buff {
			/// <param name="base_rating">The base rating of the buff.</param>
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			inline double getRating(double base_rating) noexcept {
				return base_rating * std::sqrt(2);
			}
		}

		namespace speed_buff {
			/// <param name="base_rating">The base rating of the buff.</param>
			/// <param name="wb">The % walking speed boost.</param>
			/// <param name="rb">The % running speed boost.</param>
			/// <param name="ib">The % injured speed boost.</param>
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			inline double getRating(double base_rating, double wb, double rb, double ib) noexcept {
				return base_rating * (1.0 + wb * 0.5 + rb * 0.3 + ib * 0.2);
			}
		}

		namespace healer_buff {
			/// <param name="base_rating">The base rating of the buff.</param>
			/// <param name="heal_amt">The amount of health healed every so often.</param>
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			inline double getRating(double base_rating, double heal_amt) noexcept {
				// 1.3 is a "fudge" factor.
				return base_rating * heal_amt * 1.3;
			}
		}

		namespace purify_buff {
			/// <param name="base_rating">The base rating of the buff.</param>
			/// <param name="max_effects_cured">The maximum number of effects that are cured.</param>
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			inline double getRating(double base_rating, int max_effects_cured) noexcept {
				// 0.55 is a "fudge" factor based on the usefulness of this effect.
				return base_rating * std::sqrt(max_effects_cured) * 0.55;
			}
		}

		namespace repair_buff {
			/// <param name="base_rating">The base rating of the buff.</param>
			/// <param name="repair_amt">The amount of armor repaired every so often.</param>
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			inline double getRating(double base_rating, double repair_amt) noexcept {
				// 1.4 is a "fudge" factor based on the usefulness of this buff.
				return base_rating * repair_amt * 1.4;
			}
		}

		namespace forcefield_buff {
			/// <param name="base_rating">The base rating of the buff.</param>
			/// <param name="sh">The shield's health.</param>
			/// <param name="sa">The shield's damage absorption.</param>
			/// <returns>The rating associated with this buff. This is a value that is an
			/// estimate of how much more threatening the enemy is due to the buff.</returns>
			inline double getRating(double base_rating, [[maybe_unused]] double sh, double sa) noexcept {
				// 1.7 is a "fudge" factor based on the usefulness of this buff.
				return base_rating * sa * 1.7 + (sh / sa);
			}
		}
	}

	namespace enemies {
		/// <param name="ahp">The enemy's armor health.</param>
		/// <param name="ar">The enemy's armor reduction.</param>
		/// <returns>The effective armor health of the enemy, which is an estimate of how much
		/// damage the enemy can withstand before their armor is rendered ineffective.</returns>
		inline double getEffectiveArmorHealth(double ahp, double ar) noexcept {
			return ahp * (1.0 / (1.0 - ar));
		}
		/// <param name="hp">The enemy's health.</param>
		/// <param name="eahp">The enemy's effective armor health.</param>
		/// <returns>The effective health of the enemy, which is an estimate of how much
		/// damage the enemy can withstand before dying (ignoring piercing).</returns>
		inline double getEffectiveHealth(double hp, double eahp) noexcept {
			return hp + eahp;
		}
		/// <param name="run_percent">The % life spent running.</param>
		/// <param name="injured_percent">The % life spent injured.</param>
		/// <returns>The proportion of the enemy's lifespan that the enemy spends walking
		/// (expressed as a decimal, not a %).</returns>
		inline double getWalkingPercent(double run_percent, double injured_percent) noexcept {
			return 1.0 - run_percent - injured_percent;
		}
		/// <param name="injured_percent">The % life spent injured.</param>
		/// <param name="hp">The enemy's health.</param>
		/// <param name="ehp">The enemy's effective health.</param>
		/// <returns>The proportion of the enemy's lifespan that the enemy spends running
		/// (expressed as a decimal, not a %).</returns>
		inline double getRunningPercent(double injured_percent, double hp, double ehp) noexcept {
			return hp / ehp - injured_percent;
		}
		/// <param name="hp">The enemy's health.</param>
		/// <param name="ehp">The enemy's effective health.</param>
		/// <param name="pt">The enemy's pain tolerance.</param>
		/// <returns>The proportion of the enemy's lifespan that the enemy spends injured
		/// (expressed as a decimal, not a %).</returns>
		inline double getInjuredPercent(double hp, double ehp, double pt) noexcept {
			return (hp * (1.0 - pt)) / ehp;
		}
		/// <param name="dmg">The enemy's damage.</param>
		/// <returns>The impact that the amount of damage an enemy can potentially deal has on
		/// an enemy's rating.</returns>
		inline double getDamageMultiplier(double dmg) noexcept {
			return std::sqrt(dmg);
		}
		/// <param name="move_diag">Can the enemy move diagonally?</param>
		/// <returns>The impact that being able to move diagonally has on an enemy's rating.</returns>
		inline double getDiagonalMultiplier(bool move_diag) noexcept {
			return move_diag ? 1.15 : 1.00;
		}
		/// <param name="is_flying">Is the enemy considered flying?</param>
		/// <returns>The impact that flying has on an enemy's rating.</returns>
		inline double getFlyingMultiplier(bool is_flying) noexcept {
			return is_flying ? 1.08 : 1.00;
		}
		/// <param name="wspd">The enemy's walking speed.</param>
		/// <param name="rspd">The enemy's running speed.</param>
		/// <param name="ispd">The enemy's injured speed.</param>
		/// <param name="walk_percent">The % life spent walking.</param>
		/// <param name="run_percent">The % life spent running.</param>
		/// <param name="injured_percent">The % life spent injured.</param>
		/// <param name="ehp">The enemy's effective health.</param>
		/// <param name="diag_multi">The multiplier from diagonal movement.</param>
		/// <param name="fly_multi">The multiplier from flying status.</param>
		/// <param name="dmg_multi">The multiplier from damage.</param>
		/// <returns>A rating value that factors in several of the enemy's core statistics.
		/// This does not factor in special abilities like buffs.</returns>
		inline double getBaseRating(double wspd, double rspd, double ispd, double walk_percent, double run_percent,
			double injured_percent, double ehp, double diag_multi, double fly_multi, double dmg_multi) noexcept {
			// Basically, these are the number of tiles an enemy will cross if they take 1 damage
			// for each tile they cross.
			const double walk_tiles = wspd * walk_percent * ehp;
			const double run_tiles = rspd * run_percent * ehp;
			const double injured_tiles = ispd * injured_percent * ehp;
			return (walk_tiles + run_tiles + injured_tiles) * diag_multi * fly_multi * dmg_multi;
		}
		/// <param name="buffs">The enemy's passive buffs.</param>
		/// <returns>Another rating that focuses primarily on the enemy's special abilities.</returns>
		double getExtraRating(const std::vector<std::shared_ptr<BuffBase>>& buffs) noexcept;
		/// <param name="base_rating">The enemy's "raw stats" rating.</param>
		/// <param name="extra_rating">The enemy's abilities rating.</param>
		/// <returns>Returns the enemy's full rating.</returns>
		inline double getRating(double base_rating, double extra_rating) noexcept {
			return base_rating + extra_rating;
		}
	}
}
