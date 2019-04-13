#pragma once
// File Author: Isaiah Hoffman
// File Created: June 6, 2018
#include <deque>
#include <memory>
#include <queue>
#include <vector>
#include "./../ih_math.hpp"
#include "./enemy.hpp"
#include "./game_util.hpp"

namespace hoffman::isaiah::game {
	// Forward declarations.
	class MyGame;
	class GameMap;

	/// <summary>Class that represents a group of homogeneous enemies.</summary>
	class EnemyGroup {
	public:
		/// <param name="group_enemies">The queue of enemies to spawn.</param>
		/// <param name="spawn_ms_delay">The delay in milliseconds between enemies.</param>
		EnemyGroup(std::queue<std::unique_ptr<Enemy>>&& group_enemies, int spawn_ms_delay);
		/// <summary>Updates the state of the enemy group by one logical frame.</summary>
		void update() noexcept;
		/// <returns>The total number of enemies currently in the group.</returns>
		int getEnemyCount() const noexcept {
			return static_cast<int>(this->enemies.size());
		}
		/// <returns>True if there are still enemies to spawn.</returns>
		bool hasEnemiesLeft() const noexcept {
			return !this->enemies.empty();
		}

		/// <param name="etype">The type of enemies to create.</param>
		/// <param name="extra_count">The number of extra enemies to create.</param>
		/// <param name="my_game">Reference to the game state.</param>
		/// <returns>A queue of enemies that matches the provided information.</returns>
		static std::queue<std::unique_ptr<Enemy>> createEnemies(const EnemyType* etype, int extra_count, const MyGame& my_game);
	private:
		/// <summary>The enemies in the group.</summary>
		std::queue<std::unique_ptr<Enemy>> enemies;
		/// <summary>The time in logical frames between the spawning of new enemies.</summary>
		double spawn_frame_delay;
		/// <summary>The number of frames remaining before a new enemy is spawned.</summary>
		double frames_until_next_spawn;
	};

	/// <summary>Class that represents a group of smaller groups of enemies.</summary>
	class EnemyWave {
	public:
		/// <param name="wave_groups">The queue of groups to spawn.</param>
		/// <param name="spawn_ms_delay">The delay in milliseconds between groups.</param>
		EnemyWave(std::deque<std::unique_ptr<EnemyGroup>>&& wave_groups, int spawn_ms_delay);
		/// <summary>Updates the state of the enemy wave by one logical frame.</summary>
		void update() noexcept;
		/// <returns>The number of enemies still left to spawn in the wave.</returns>
		int getEnemyCount() const noexcept {
			int subtotal = 0;
			for (const auto& g : this->groups) {
				subtotal += g->getEnemyCount();
			}
			for (const auto& g : this->active_groups) {
				subtotal += g->getEnemyCount();
			}
			return subtotal;
		}
		/// <returns>True if there are any enemies left to be spawned in the wave.</returns>
		bool hasEnemiesLeft() const noexcept {
			if (this->groups.empty()) {
				for (const auto& g : this->active_groups) {
					if (g->hasEnemiesLeft()) {
						return true;
					}
				}
				return false;
			}
			return true;
		}
	private:
		/// <summary>The groups in this wave of enemies, in the order that they will begin spawning.</summary>
		std::deque<std::unique_ptr<EnemyGroup>> groups;
		/// <summary>The list of groups that are currently spawning enemies (or have spawned all their enemies).</summary>
		std::vector<std::unique_ptr<EnemyGroup>> active_groups;
		/// <summary>The number of logical frames between when groups start to spawn enemies.</summary>
		double spawn_frame_delay;
		/// <summary>The number of logical frames remaining before a new group starts spawning enemies.</summary>
		double frames_until_next_spawn;
	};

	/// <summary>Class that represents a level in the game.</summary>
	class GameLevel {
	public:
		/// <param name="level_no">The current level number.</param>
		/// <param name="level_waves">The queue of waves to spawn.</param>
		/// <param name="spawn_ms_delay">The delay in milliseconds between waves.</param>
		GameLevel(int level_no, std::deque<std::unique_ptr<EnemyWave>>&& level_waves, int spawn_ms_delay);
		/// <summary>Updates the state of the game level by one logical frame.</summary>
		void update() noexcept;
		/// <returns>The number of enemies left to spawn in the level.</returns>
		int getEnemyCount() const noexcept {
			int subtotal = 0;
			for (const auto& w : this->waves) {
				subtotal += w->getEnemyCount();
			}
			for (const auto& w : this->active_waves) {
				subtotal += w->getEnemyCount();
			}
			return subtotal;
		}
		/// <returns>True if there are still enemies to spawn; otherwise, false.</returns>
		bool hasEnemiesLeft() const noexcept {
			if (this->waves.empty()) {
				for (const auto& w : this->active_waves) {
					if (w->hasEnemiesLeft()) {
						return true;
					}
				}
				return false;
			}
			return true;
		}
	private:
		/// <summary>The level number of this level.</summary>
		int level;
		/// <summary>The waves of enemies in this level left to spawn.</summary>
		std::deque<std::unique_ptr<EnemyWave>> waves;
		/// <summary>The list of waves for this level that are currently (or have finished) spawning enemies.</summary>
		std::vector<std::unique_ptr<EnemyWave>> active_waves;
		/// <summary>The number of logical frames between when waves start to spawn enemies.</summary>
		double spawn_frame_delay;
		/// <summary>The number of logical frames remaining before a new wave starts spawning enemies.</summary>
		double frames_until_next_spawn;
	};

	/// <summary>Stores a normally distributed random variable whose mean changes each level.</summary>
	class LevelNormalRandomVariable {
	public:
		/// <param name="my_params">The parameters to the normal distribution (mu and sigma).</param>
		/// <param name="my_change">The change in the center of the distribution for each level.</param>
		LevelNormalRandomVariable(NormalRandomVariable my_params, double my_change) noexcept :
			normal_params {my_params},
			level_change {my_change} {
		}

		/// <param name="levels_above_start">The number of levels since the first generated level.</param>
		/// <returns>A normally distributed value with the appropriate parameters.</returns>
		double operator()(int levels_above_start) const noexcept {
			return this->normal_params() + this->level_change * levels_above_start;
		}
	private:
		/// <summary>The parameters to the normal distribution.</summary>
		NormalRandomVariable normal_params;
		/// <summary>The amount by which the center (mu) of the distribution is shifted each level.</summary>
		double level_change;
	};

	/// <summary>Stores metadata about colors (i.e.: primary grouping of enemies) for
	/// use by the level generator.</summary>
	class GlobalLevelColorData {
	public:
		/// <param name="n">The reference name for the enemy color type.</param>
		/// <param name="z">The z-score associated with the color.</param>
		GlobalLevelColorData(std::wstring n, double z) noexcept :
			name {n},
			z_difficulty {z} {
		}
		// Getters
		std::wstring getName() const noexcept {
			return this->name;
		}
		double getZScore() const noexcept {
			return this->z_difficulty;
		}
	private:
		/// <summary>The reference name for the color.</summary>
		std::wstring name;
		/// <summary>The z-score (representing difficulty) associated with the color.</summary>
		double z_difficulty;
	};

	/// <summary>Represents the possible enemy spawn densities.</summary>
	enum class EnemySpawnDensities {
		Dense, Normal, Sparse
	};

	/// <summary>Stores metadata about enemies for use by the level generator.</summary>
	class GlobalLevelEnemyData {
	public:
		/// <param name="my_game">Constant reference to the game state.</param>
		/// <param name="ename">The name of the enemy.</param>
		/// <param name="cname">The name of the color group that the enemy references.</param>
		/// <param name="z">The z-score associated with the enemy.</param>
		/// <param name="ec_params">The normal distribution parameters for the extra count of enemies.</param>
		/// <param name="ec_increase">The amount that the mean number of extra enemies generated increases each level.</param>
		/// <param name="stimes">The dense (0), normal (1), and sparse (2) times of the enemy in milliseconds.</param>
		GlobalLevelEnemyData(const MyGame& my_game, std::wstring ename, std::wstring cname, double z,
			LevelNormalRandomVariable ec_var, std::array<int, 3> stimes);

		/// <param name="levels_above_start">The number of levels since the first generated level.</param>
		/// <returns>The extra number of enemies to generate for this particular call.</returns>
		int rollExtraCount(int levels_above_start) const noexcept {
			return math::get_max(static_cast<int>(this->extra_count_var(levels_above_start)), 0);
		}
		// Getters
		const EnemyType* getType() const noexcept {
			return this->enemy_type;
		}
		std::wstring getColorName() const noexcept {
			return this->color_name;
		}
		double getZScore() const noexcept {
			return this->z_difficulty;
		}
		/// <param name="spawn_density">The spawn density time to return.</param>
		int getSpawnTime(EnemySpawnDensities spawn_density) const noexcept {
			return spawn_density == EnemySpawnDensities::Dense ? this->spawn_times.at(0)
				: spawn_density == EnemySpawnDensities::Normal ? this->spawn_times.at(1)
				: this->spawn_times.at(2);
		}
	private:
		/// <summary>The enemy type associated with this data.</summary>
		const EnemyType* enemy_type;
		/// <summary>The color group the enemy belongs to.</summary>
		std::wstring color_name;
		/// <summary>The z-score associated with this enemy which is used to determine
		/// the frequency this enemy is spawned.</summary>
		double z_difficulty;
		/// <summary>The random variable associated with determining the extra count.</summary>
		LevelNormalRandomVariable extra_count_var;
		/// <summary>The delay in milliseconds for spawning this enemy type at varying densities.
		/// 0 => Dense, 1 => Normal, 2 => Sparse.</summary>
		std::array<int, 3> spawn_times;
	};

	/// <summary>Used to generate new levels randomly.</summary>
	class LevelGenerator {
	public:
		/// <param name="start_lv">The number of the first level to automatically generate.</param>
		/// <param name="cdata">The color metadata for the level generator.</param>
		/// <param name="edata">The enemy metadata for the level generator.</param>
		/// <param name="wd_var">The wave difficulty random variable.</param>
		/// <param name="gd_var">The group difficulty random variable.</param>
		/// <param name="nw_var">The number of waves random variable.</param>
		/// <param name="ng_var">The number of groups random variable.</param>
		/// <param name="wd">The wave delay (in milliseconds).</param>
		/// <param name="gd">The group delay (in milliseconds).</param>
		LevelGenerator(int start_lv, std::vector<GlobalLevelColorData> cdata, std::vector<GlobalLevelEnemyData> edata,
			LevelNormalRandomVariable wd_var, LevelNormalRandomVariable gd_var, LevelNormalRandomVariable nw_var,
			LevelNormalRandomVariable ng_var, int wd, int gd);

		/// <summary>Randomly generates a level.</summary>
		/// <param name="level_number">The level number of the level to generate.</param>
		/// <param name="my_game">Reference to the game state.</param>
		/// <returns>A pointer to the generated level. The caller is responsible for taking ownership.</returns>
		std::unique_ptr<GameLevel> generateLevel(int level_number, const MyGame& my_game);
	protected:
		/// <param name="levels_above_start">The number of levels since the first generated level.</param>
		/// <returns>The wave difficulty value for this particular call.</returns>
		double rollWaveDifficulty(int levels_above_start) const noexcept {
			return this->wave_difficulty_var(levels_above_start);
		}
		/// <param name="levels_above_start">The number of levels since the first generated level.</param>
		/// <returns>The group difficulty value for this particular call.</returns>
		double rollGroupDifficulty(int levels_above_start) const noexcept {
			return this->group_difficulty_var(levels_above_start);
		}
		/// <param name="levels_above_start">The number of levels since the first generated level.</param>
		/// <returns>The number of waves to generate for this particular call.</returns>
		int rollNumWaves(int levels_above_start) const noexcept {
			return math::get_max(static_cast<int>(this->num_waves_var(levels_above_start)), 1);
		}
		/// <param name="levels_above_start">The number of levels since the first generated level.</param>
		/// <returns>The number of groups to generate for this particular call.</returns>
		int rollNumGroups(int levels_above_start) const noexcept {
			return math::get_max(static_cast<int>(this->num_groups_var(levels_above_start)), 1);
		}

		// Getters
		int getStartLevel() const noexcept {
			return this->start_level;
		}
		const std::vector<GlobalLevelColorData>& getColorData() const noexcept {
			return this->color_data;
		}
		const std::vector<GlobalLevelEnemyData>& getEnemyData() const noexcept {
			return this->enemy_data;
		}
		int getWaveDelay() const noexcept {
			return this->wave_delay;
		}
		int getGroupDelay() const noexcept {
			return this->group_delay;
		}
	private:
		/// <summary>The first level that the level generator should be used for.</summary>
		int start_level;
		/// <summary>The global color metadata.</summary>
		std::vector<GlobalLevelColorData> color_data;
		/// <summary>The global enemy metadata.</summary>
		std::vector<GlobalLevelEnemyData> enemy_data;
		/// <summary>The normally distributed random variable associated with
		/// the difficulty of generated waves each level.</summary>
		LevelNormalRandomVariable wave_difficulty_var;
		/// <summary>The normally distributed random variable associated with
		/// the difficulty of generated groups each level.</summary>
		LevelNormalRandomVariable group_difficulty_var;
		/// <summary>The normally distributed random variable associated with
		/// the number of waves generated each level.</summary>
		LevelNormalRandomVariable num_waves_var;
		/// <summary>The normally distributed random variable associated with
		/// the number of groups generated each level.</summary>
		LevelNormalRandomVariable num_groups_var;
		/// <summary>The standard wave delay (in milliseconds) used by the level generator.</summary>
		int wave_delay;
		/// <summary>The standard group delay (in milliseconds) used by the level generator.</summary>
		int group_delay;
	};
}