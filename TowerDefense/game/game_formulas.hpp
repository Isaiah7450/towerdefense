#pragma once
// Author: Isaiah Hoffman
// Created: March 17, 2019
#include <cmath>
#include <utility>
#include <vector>
#include "./../ih_math.hpp"
namespace hoffman::isaiah::game {
	class FiringMethod;
	class TargetingStrategy;
	class ShotBaseType;

	namespace towers {
		/// <param name="fr">The firing range of the tower.</param>
		/// <param name="fm">The firing method of the tower.</param>
		/// <param name="is_wall">Is this tower considered a wall?</param>
		/// <returns>The approximate area covered by the tower.</returns>
		double getFiringArea(double fr, const FiringMethod& fm, bool is_wall) noexcept;
		/// <param name="stypes">The shot types and related frequencies that the tower fires.</param>
		/// <param name="dm">The tower's damage multiplier.</param>
		/// <returns>The expected amount of raw damage output by the tower per shot on average.</returns>
		double getAverageDamagePerShot(const std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>>& stypes, double dm) noexcept;
		/// <param name="stypes">The shot types and related frequencies that the tower fires.</param>
		/// <returns>The weighted average of the tower's shot types.</returns>
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
		/// <param name="firing_area">The total firing area covered by the tower.</param>
		/// <param name="fm">The firing method of the tower.</param>
		/// <param name="ts">The targeting strategy of the tower.</param>
		/// <param name="avg_dmg">The expected damage per shot of the tower.</param>
		/// <param name="avg_shot_rating">The average shot rating of the tower.</param>
		/// <param name="is_wall">Is this tower considered a wall?</param>
		/// <returns>The tower's overall rating.</returns>
		double getRating(double rate_of_fire, double firing_area, const FiringMethod& fm, const TargetingStrategy& ts,
			double avg_dmg, double avg_shot_rating, bool is_wall) noexcept;
		/// <param name="rating">The tower's rating.</param>
		/// <param name="cost_adjust">The tower's cost adjustment.</param>
		/// <param name="is_wall">Is this tower considered a wall?</param>
		/// <returns>The cost of the tower.</returns>
		inline double getCost(double rating, int cost_adjust, bool is_wall) noexcept {
			return is_wall ? static_cast<double>(cost_adjust) : cost_adjust + rating / 8.4 + 1.0;
		}
	}
}
