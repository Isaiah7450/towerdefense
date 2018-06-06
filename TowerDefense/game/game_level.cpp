// File Author: Isaiah Hoffman
// File Created: June 6, 2018
#include <vector>
#include <memory>
#include <queue>
#include <deque>
#include "./../ih_math.hpp"
#include "./enemy.hpp"
#include "./game_level.hpp"
#include "./my_game.hpp"

namespace hoffman::isaiah {
	namespace game {
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
			this->active_groups.emplace_back(std::move(this->groups.front()));
			this->groups.pop_front();
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
			this->active_waves.emplace_back(std::move(this->waves.front()));
			this->waves.pop_front();
		}
	}
}