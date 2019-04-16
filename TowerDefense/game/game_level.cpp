// File Author: Isaiah Hoffman
// File Created: June 6, 2018
#include <algorithm>
#include <deque>
#include <memory>
#include <random>
#include <utility>
#include <vector>
#include <queue>
#include "./../ih_math.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./enemy.hpp"
#include "./game_level.hpp"
#include "./game_util.hpp"
#include "./my_game.hpp"

namespace hoffman::isaiah::game {
	EnemyGroup::EnemyGroup(std::queue<std::unique_ptr<Enemy>>&& group_enemies, int spawn_ms_delay) :
		enemies {std::move(group_enemies)},
		spawn_frame_delay {math::convertMillisecondsToFrames(spawn_ms_delay)},
		frames_until_next_spawn {0} {
	}

	void EnemyGroup::update() noexcept {
		--this->frames_until_next_spawn;
		if (!this->hasEnemiesLeft() || this->frames_until_next_spawn > 0) {
			return;
		}
		this->frames_until_next_spawn += this->spawn_frame_delay;
		game::g_my_game->addEnemy(std::move(this->enemies.front()));
		this->enemies.pop();
	}

	std::queue<std::unique_ptr<Enemy>> EnemyGroup::createEnemies(const EnemyType* etype, int extra_count, const MyGame& my_game) {
		const int enemy_count = etype->isUnique() ? 1 + extra_count : my_game.getChallengeLevel() + extra_count + 2;
		pathfinding::Pathfinder my_pathfinder {my_game.getMap(), etype->isFlying(), etype->canMoveDiagonally(),
			etype->getDefaultStrategy()};
		my_pathfinder.findPath(my_game.getChallengeLevel() / 10.0);
		std::queue<std::unique_ptr<Enemy>> my_enemy_spawns {};
		for (int i = 0; i < enemy_count; ++i) {
			auto my_enemy = std::make_unique<Enemy>(my_game.getDeviceResources(), my_game.getMap(), etype,
				graphics::Color {0.f, 0.f, 0.f, 1.f}, my_pathfinder,
				my_game.getMap().getTerrainGraph(etype->isFlying()).getStartNode()->getGameX() + 0.5,
				my_game.getMap().getTerrainGraph(etype->isFlying()).getStartNode()->getGameY() + 0.5,
				my_game.getLevelNumber(), my_game.getDifficulty(), my_game.getChallengeLevel());
			my_enemy_spawns.emplace(std::move(my_enemy));
		}
		return std::move(my_enemy_spawns);
	}


	EnemyWave::EnemyWave(std::deque<std::unique_ptr<EnemyGroup>>&& wave_groups, int spawn_ms_delay) :
		groups {std::move(wave_groups)},
		active_groups {},
		spawn_frame_delay {math::convertMillisecondsToFrames(spawn_ms_delay)},
		frames_until_next_spawn {0} {
	}

	void EnemyWave::update() noexcept {
		for (auto& g : this->active_groups) {
			g->update();
		}
		if (this->groups.empty()) {
			return;
		}
		--this->frames_until_next_spawn;
		if (this->frames_until_next_spawn > 0) {
			return;
		}
		this->frames_until_next_spawn += this->spawn_frame_delay;
		this->active_groups.emplace_back(std::move(this->groups.back()));
		this->groups.pop_back();
	}

	GameLevel::GameLevel(int level_no, std::deque<std::unique_ptr<EnemyWave>>&& level_waves, int spawn_ms_delay) :
		level {level_no},
		waves {std::move(level_waves)},
		active_waves {},
		spawn_frame_delay {math::convertMillisecondsToFrames(spawn_ms_delay)},
		frames_until_next_spawn {0} {
	}

	void GameLevel::update() noexcept {
		for (auto& w : this->active_waves) {
			w->update();
		}
		if (this->waves.empty()) {
			return;
		}
		--this->frames_until_next_spawn;
		if (this->frames_until_next_spawn > 0) {
			return;
		}
		this->frames_until_next_spawn += this->spawn_frame_delay;
		this->active_waves.emplace_back(std::move(this->waves.back()));
		this->waves.pop_back();
	}

	GlobalLevelEnemyData::GlobalLevelEnemyData(const MyGame& my_game, std::wstring ename, std::wstring cname, double z,
		LevelNormalRandomVariable ec_var, std::array<int, 3> stimes) :
		enemy_type {my_game.getEnemyType(ename)},
		color_name {cname},
		z_difficulty {z},
		extra_count_var {ec_var},
		spawn_times {stimes} {
	}

	GlobalLevelBossData::GlobalLevelBossData(const MyGame& my_game, std::wstring ename, double z) :
		enemy_type {my_game.getEnemyType(ename)},
		z_difficulty {z} {
	}

	LevelGenerator::LevelGenerator(int start_lv, std::vector<GlobalLevelColorData> cdata, std::vector<GlobalLevelEnemyData> edata,
		std::vector<GlobalLevelBossData> bdata,	LevelNormalRandomVariable wd_var, LevelNormalRandomVariable gd_var,
		LevelNormalRandomVariable bd_var, LevelNormalRandomVariable nw_var, LevelNormalRandomVariable ng_var,
		LevelNormalRandomVariable nb_var, int wd, int gd, int bmod) :
		start_level {start_lv},
		color_data {cdata},
		enemy_data {edata},
		boss_data {bdata},
		wave_difficulty_var {wd_var},
		group_difficulty_var {gd_var},
		boss_difficulty_var {bd_var},
		num_waves_var {nw_var},
		num_groups_var {ng_var},
		num_bosses_var {nb_var},
		wave_delay {wd},
		group_delay {gd},
		boss_level_mod {bmod} {
		std::sort(this->color_data.begin(), this->color_data.end(), [](const auto& a, const auto& b) {
			return a.getZScore() < b.getZScore();
		});
		std::sort(this->enemy_data.begin(), this->enemy_data.end(), [](const auto& a, const auto& b) {
			return a.getColorName() == b.getColorName() && a.getZScore() < b.getZScore();
		});
		std::sort(this->boss_data.begin(), this->boss_data.end(), [](const auto& a, const auto& b) {
			return a.getZScore() < b.getZScore();
		});
	}

	std::unique_ptr<GameLevel> LevelGenerator::generateLevel(int level_number, const MyGame& my_game) {
		const int levels_above_start = level_number - this->getStartLevel();
		const int num_waves = this->rollNumWaves(levels_above_start);
		const int num_groups = this->rollNumGroups(levels_above_start);
		const int min_wave_groups = num_groups / num_waves;
		const int wave_groups_overflow = num_groups % num_waves;
		std::deque<std::unique_ptr<EnemyWave>> my_level_waves {};
		for (int w = 0; w < num_waves; ++w) {
			if ((level_number - this->getStartLevel()) % this->boss_level_mod == 0 && level_number > start_level
				&& w == num_waves / 2) {
				// Boss level; add boss enemies.
				const int groups_in_this_wave = this->rollNumBosses(levels_above_start);
				std::deque<std::unique_ptr<EnemyGroup>> my_wave_groups {};
				for (int g = 0; g < groups_in_this_wave; ++g) {
					// Determine boss type.
					const EnemyType* my_etype {nullptr};
					static constexpr const int extra_count = 0;
					static constexpr const int enemy_delay = 1500;
					while (!my_etype) {
						const double my_boss_difficulty = this->rollBossDifficulty(levels_above_start);
						for (const auto& bdata : this->getBossData()) {
							my_etype = bdata.getType();
						}
					}
					std::queue<std::unique_ptr<Enemy>> group_enemies {EnemyGroup::createEnemies(my_etype, extra_count, my_game)};
					auto my_enemy_group = std::make_unique<EnemyGroup>(std::move(group_enemies), enemy_delay);
					my_wave_groups.emplace_back(std::move(my_enemy_group));
				}
				auto my_enemy_wave = std::make_unique<EnemyWave>(std::move(my_wave_groups), this->getGroupDelay());
				my_level_waves.emplace_back(std::move(my_enemy_wave));
			}
			const int groups_in_this_wave = w - wave_groups_overflow < 0 ? min_wave_groups + 1 : min_wave_groups;
			// Determine wave color.
			std::wstring cname = L"";
			while (cname == L"") {
				const double my_wave_difficulty = this->rollWaveDifficulty(levels_above_start);
				for (const auto& cdata : this->getColorData()) {
					if (my_wave_difficulty <= cdata.getZScore()) {
						cname = cdata.getName();
						break;
					}
				}
			}
			std::deque<std::unique_ptr<EnemyGroup>> my_wave_groups {};
			for (int g = 0; g < groups_in_this_wave; ++g) {
				// Determine group type and associated count.
				const EnemyType* my_etype {nullptr};
				int extra_count = 0;
				int enemy_delay = 0;
				while (!my_etype) {
					const double my_group_difficulty = this->rollGroupDifficulty(levels_above_start);
					for (const auto& edata : this->getEnemyData()) {
						if (edata.getColorName() == cname && my_group_difficulty <= edata.getZScore()) {
							my_etype = edata.getType();
							extra_count = edata.rollExtraCount(levels_above_start);
							const double my_roll = rng::distro_uniform(rng::gen);
							enemy_delay = my_roll <= 0.33 ? edata.getSpawnTime(EnemySpawnDensities::Dense)
								: my_roll <= 0.67 ? edata.getSpawnTime(EnemySpawnDensities::Normal)
								: edata.getSpawnTime(EnemySpawnDensities::Sparse);
							break;
						}
					}
				}
				std::queue<std::unique_ptr<Enemy>> group_enemies {EnemyGroup::createEnemies(my_etype, extra_count, my_game)};
				auto my_enemy_group = std::make_unique<EnemyGroup>(std::move(group_enemies), enemy_delay);
				my_wave_groups.emplace_back(std::move(my_enemy_group));
			}
			auto my_enemy_wave = std::make_unique<EnemyWave>(std::move(my_wave_groups), this->getGroupDelay());
			my_level_waves.emplace_back(std::move(my_enemy_wave));
		}
		auto my_level = std::make_unique<GameLevel>(level_number, std::move(my_level_waves), this->getWaveDelay());
		return my_level;
	}
}