#pragma once
// File Author: Isaiah Hoffman
// File Created: June 6, 2018
#include <vector>
#include <memory>
#include <queue>
#include <deque>
#include "./../ih_math.hpp"
#include "./enemy.hpp"

namespace hoffman::isaiah {
	namespace game {

		/// <summary>Class that represents a group of homogeneous enemies.</summary>
		class EnemyGroup {
		public:
			EnemyGroup(std::queue<std::unique_ptr<Enemy>>&& group_enemies, int spawn_ms_delay);
			/// <summary>Updates the state of the enemy group by one logical frame.</summary>
			void update() noexcept;
			/// <returns>The total number of enemies currently in the group.</returns>
			int getEnemyCount() const noexcept {
				return this->enemies.size();
			}
			/// <returns>True if there are still enemies to spawn.</returns>
			bool hasEnemiesLeft() const noexcept {
				return !this->enemies.empty();
			}
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
	}
}