// File Author: Isaiah Hoffman
// File Created: March 26, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <future>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <utility>
#include <vector>
#include "./../resource.h"
#include "./../file_util.hpp"
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../audio/audio.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../graphics/other_dialogs.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./enemy_type.hpp"
#include "./enemy.hpp"
#include "./game_level.hpp"
#include "./game_util.hpp"
#include "./my_game.hpp"
#include "./shot_types.hpp"
#include "./shot.hpp"
#include "./status_effects.hpp"
#include "./tower_types.hpp"
#include "./tower.hpp"
using namespace std::literals::string_literals;

namespace hoffman_isaiah {
	namespace game {
		std::shared_ptr<MyGame> g_my_game {nullptr};

		MyGame::MyGame(graphics::DX::DeviceResources2D* dev_res) :
			device_resources {dev_res},
			highest_levels {{ID_CHALLENGE_LEVEL_EASY, 0}, {ID_CHALLENGE_LEVEL_NORMAL, 0},
				{ID_CHALLENGE_LEVEL_HARD, 0}, {ID_CHALLENGE_LEVEL_EXPERT, 0}} {
		}

		MyGame::~MyGame() noexcept = default;
		MyGame::MyGame(MyGame&&) = default;
		MyGame& MyGame::operator=(MyGame&&) = default;

		void MyGame::debugUpdate(DebugUpdateStates cause) {
#if defined(DEBUG) || defined(_DEBUG)
			// Do processing...
			switch (cause) {
			case DebugUpdateStates::Terrain_Changed:
			{
				this->ground_test_pf = std::make_shared<pathfinding::Pathfinder>(this->getMap(), false,
					false, pathfinding::HeuristicStrategies::Manhattan);
				this->air_test_pf = std::make_shared<pathfinding::Pathfinder>(this->getMap(), true,
					false, pathfinding::HeuristicStrategies::Manhattan);
				break;
			}
			default:
				break;
			}
#else
			UNREFERENCED_PARAMETER(cause);
#endif
		}

		void MyGame::resetState(int new_clevel, std::wstring map_name, bool is_custom) {
			this->player = Player {};
			this->challenge_level = new_clevel;
			this->level = 1;
			this->difficulty = 1.0;
			this->my_level = nullptr;
			this->my_level_enemy_count = 0;
			this->my_level_enemy_killed = 0;
			this->enemies.clear();
			this->towers.clear();
			this->shots.clear();
			for (auto& e_seen : this->enemies_seen) {
				e_seen.second = false;
			}
			this->is_paused = false;
			this->in_level = false;
			const auto air_terrain_filename_base = this->resources_folder_path + L"graphs/air_graph_";
			const auto ground_terrain_filename_base = this->resources_folder_path + L"graphs/ground_graph_";
			std::wifstream ground_terrain_file {ground_terrain_filename_base + map_name + L".txt"};
			std::wifstream air_terrain_file {air_terrain_filename_base + map_name + L".txt"};
			if (ground_terrain_file.good() && air_terrain_file.good()) {
				this->map = std::make_shared<GameMap>(ground_terrain_file, air_terrain_file);
				this->debugUpdate(DebugUpdateStates::Terrain_Changed);
				this->setGameType(is_custom);
				this->map_base_name = map_name;
			}
			else {
				this->map_base_name = MyGame::getDefaultMapName(new_clevel + ID_CHALLENGE_LEVEL_EASY);
				ground_terrain_file.open(ground_terrain_filename_base + this->getMapBaseName() + L".txt");
				air_terrain_file.open(air_terrain_filename_base + this->getMapBaseName() + L".txt");
				if (ground_terrain_file.good() && air_terrain_file.good()) {
					MessageBox(nullptr, L"Warning: Specified map could not be found. Falling back to a default map.", L"Map Load Failed",
						MB_OK | MB_ICONWARNING);
					this->map = std::make_shared<GameMap>(ground_terrain_file, air_terrain_file);
					this->debugUpdate(DebugUpdateStates::Terrain_Changed);
					this->setGameType(false);
				}
				else {
					MessageBox(nullptr, L"Map loading failed.", L"Map Load Failed", MB_OK | MB_ICONERROR);
				}
			}
			this->my_level_enemy_count = 0;
			this->did_lose_life = false;
			this->win_streak = 0;
			this->lose_streak = 0;
			this->is_hiscore = false;
		}

		void MyGame::update() {
			if (this->is_paused || !this->in_level) {
				return;
			}
			if (!this->player.isAlive()) {
				this->is_paused = true;
				this->in_level = false;
				// To prevent players from closing out and thus being able to replay the level.
				const std::wstring save_name {this->getUserDataPath() + game::default_save_file_name};
				std::wofstream my_save {save_name};
				this->saveGame(my_save);
				my_save.close();
				std::wofstream my_game_stats {save_name + L".stats"};
				for (const auto& estats : this->enemy_kill_count) {
					if (this->enemies_seen.at(estats.first)) {
						my_game_stats << estats.first << L": " << estats.second << L"\n";
					}
				}
				my_game_stats.close();
				// For integrity reasons, the stats of custom games are not tracked.
				if (!this->in_custom_game) {
					if (this->getLevelNumber() > 99) {
						this->start_custom_games = true;
					}
					if (this->calculateScore() > this->highest_score) {
						this->highest_score = this->calculateScore();
						this->is_hiscore = true;
					}
				}
				this->saveGlobalData();
				audio::g_my_audio->playSong(audio::gameover_index);
			}
			// Do processing...
			for (int k = 0; k < this->update_speed && this->in_level; ++k) {
				// Update level
				if (this->my_level) {
					this->my_level->update();
				}
				// Update enemies
				std::vector<std::future<bool>> ret_values {};
				std::vector<int> enemies_to_remove {};
				for (unsigned int i = 0; i < this->enemies.size(); ++i) {
					// Here, we want a pointer to a member since the function we want to invoke
					// is a class method.
					//ret_values.push_back(std::async(&Enemy::update, this->enemies[i].get()));
				}
				for (unsigned int i = 0; i < this->enemies.size(); ++i) {
					if (this->enemies[i]->update()) {
					//if (ret_values.at(i).get()) {
						if (this->enemies[i]->isAlive()) {
							this->did_lose_life = true;
							this->player.changeHealth(-this->enemies[i]->getBaseType().getDamage());
						}
						else {
							++this->my_level_enemy_killed;
							this->enemy_kill_count.at(this->enemies[i]->getBaseType().getName()) += 1;
							// Alter influence score on Experienced challenge level and higher
							if (this->getChallengeLevel() >= ID_CHALLENGE_LEVEL_HARD - ID_CHALLENGE_LEVEL_EASY) {
								auto& my_node = this->getMap().getInfluenceGraph(
									this->enemies[i]->getBaseType().isFlying()).getNode(
										static_cast<int>(std::floor(this->enemies[i]->getGameX())),
										static_cast<int>(std::floor(this->enemies[i]->getGameY())));
								my_node.setWeight(my_node.getWeight() + 1);
							}
						}
						enemies_to_remove.emplace_back(i);
					}
				}
				for (unsigned int i = 0; i < enemies_to_remove.size(); ++i) {
					// Remove dead/goal enemies (yes, the parenthesis are required...)
					this->enemies.erase(this->enemies.begin() + (enemies_to_remove[i] - i));
				}
				// Update shots
				std::vector<int> shots_to_remove {};
				for (unsigned int i = 0; i < this->shots.size(); ++i) {
					if (this->shots[i]->update(this->enemies)) {
						shots_to_remove.emplace_back(i);
					}
				}
				for (unsigned int i = 0; i < shots_to_remove.size(); ++i) {
					// Remove shots that collided or that should otherwise be erased
					this->shots.erase(this->shots.begin() + (shots_to_remove[i] - i));
				}
				// Update towers
				for (auto& t : this->towers) {
					auto ret_value = t->update(this->enemies);
					for (auto& s : ret_value) {
						this->shots.emplace_back(std::move(s));
					}
				}
				// Determine if the level is finished
				if (this->my_level && !this->my_level->hasEnemiesLeft() && this->enemies.empty()
					&& this->player.isAlive()) {
					// Award reward money
					const double kill_percent = static_cast<double>(this->my_level_enemy_killed)
						/ this->my_level_enemy_count;
					const int max_reward_money = static_cast<int>(((this->level < 5 ?
						100 : this->level < 10 ?
						85 : this->level < 15 ?
						70 : this->level < 20 ?
						60 : this->level < 25 ?
						50 : this->level < 30 ?
						45 : this->level < 35 ?
						40 : this->level < 40 ?
						35 : this->level < 45 ?
						30 : this->level < 50 ?
						25 : this->level < 75 ?
						20 : this->level < 100 ?
						15 : 10)) * (1.00 + this->getChallengeLevel() / 4.0)
						+ std::sqrt(this->difficulty))
						+ (this->level < 99
							// For the last 10 or so levels, the difficulty of the
							// game is much higher compared to the rest of the game.
							// While I think planning ahead is very important for
							// any tower defense game, I also do not want to lock
							// the player in with their choices from level 1 to 50
							// with no real opportunity to make changes.
							? (this->level == 98 ? 250
								: this->level == 97 ? 150
								: this->level == 96 ? 125
								: this->level == 95 ? 100
								: this->level == 89 ? 75
								: this->level >= 90 ? 45
								: this->level % 25 == 24 ? 60
								: this->level % 10 == 9 ? 30
								: this->level % 5 == 4 ? 10 : 0)
							: 0);
					this->player.changeMoney(max_reward_money * kill_percent);
					this->my_level_enemy_count = 0;
					this->my_level_enemy_killed = 0;
					this->my_level = nullptr;
					// Reset game state
					for (auto& t : this->towers) {
						// Also take the time to reward extra cash if appropriate.
						const auto my_extra_cash_ability = t->getUpgradeSpecials().find(TowerUpgradeSpecials::Extra_Cash);
						if (my_extra_cash_ability != t->getUpgradeSpecials().cend()) {
							const auto my_roll = rng::distro_uniform(rng::gen);
							if (my_roll <= my_extra_cash_ability->second.first) {
								this->player.changeMoney(my_extra_cash_ability->second.second);
							}
						}
						t->resetTower();
					}
					this->shots.clear();
					// Update difficulty
					this->updateDifficulty();
					++this->level;
					// Unlock the ability to start custom games, and make sure the game remembers this fact.
					if (this->getLevelNumber() > 99) {
						this->start_custom_games = true;
						this->saveGlobalData();
					}
					this->did_lose_life = false;
					this->in_level = false;
					if (level < 100) {
						audio::g_my_audio->playSong(audio::town_index);
					}
					else {
						audio::g_my_audio->playSong(audio::victory_index);
					}
				}
			}
		}

		void MyGame::addEnemy(std::unique_ptr<Enemy>&& e) {
			const std::wstring ename = e->getBaseType().getName();
			if (!this->enemies_seen.at(ename)) {
				this->enemies_seen.at(ename) = true;
			}
			this->enemies.emplace_back(std::move(e));
		}

		void MyGame::addTower(std::unique_ptr<Tower>&& t) {
			this->towers.emplace_back(std::move(t));
		}

		void MyGame::startWave() {
			this->is_paused = false;
			if (!this->isInLevel() && this->player.isAlive()) {
				// Automatically save the player's progress...
				std::wofstream save_file {this->getUserDataPath() + game::default_save_file_name};
				if (!save_file.fail() && !save_file.bad()) {
					this->saveGame(save_file);
					if (!this->in_custom_game) {
						if (this->getLevelNumber() > this->highest_levels.at(challenge_level + ID_CHALLENGE_LEVEL_EASY)) {
							this->highest_levels.at(challenge_level + ID_CHALLENGE_LEVEL_EASY) = this->getLevelNumber();
						}
						this->saveGlobalData();
					}
				}
				// Load the level...
				this->in_level = true;
				try {
					this->load_level_data();
				}
				catch ([[maybe_unused]] const util::file::DataFileException& e) {
					MessageBox(nullptr, e.what(), L"Level Loading Error", MB_OK);
					// Though it is not really meant to be used for levels under the threshold,
					// it should still work despite such.
					this->my_level = this->my_level_generator->generateLevel(this->getLevelNumber(), *this);
				}
				this->my_level_enemy_count = this->my_level->getEnemyCount();
				if (level != 99 && level % 5 != 0 || level == 100) {
					audio::g_my_audio->playSong(audio::level_index);
				}
				else {
					audio::g_my_audio->playSong(audio::boss_index);
				}
			}
		}

		void MyGame::previewWave() {
			if (this->isInLevel() || !this->player.isAlive()) {
				// Can only preview while not in a level.
				return;
			}
			this->load_level_data();
			const winapi::PreviewLevelDialog preview_dialog {GetActiveWindow(),
				GetModuleHandle(nullptr), *this->my_level};
			this->my_level = nullptr;
		}

		void MyGame::buyHealth() {
			if (this->isInLevel() || !this->player.isAlive()) {
				// Can't buy health while enemies are attacking or if dead.
				return;
			}
			if (this->player.getMoney() < this->getHealthBuyCost()) {
				// Not enough money.
				return;
			}
			this->player.changeMoney(-this->hp_buy_cost);
			this->hp_buy_cost *= this->hp_buy_multiplier;
			this->player.changeHealth(this->hp_gained_per_buy);
		}

		void MyGame::changeUpdateSpeed() {
			this->update_speed = this->getNextUpdateSpeed();
		}

		void MyGame::buyTower(int gx, int gy) {
			if (this->getMap().getFiterGraph(false).getNode(gx, gy).isBlocked()
				&& this->getMap().getFiterGraph(true).getNode(gx, gy).isBlocked()) {
				// Toggle radius
				for (auto& t : this->towers) {
					if (t->getGameX() == gx + 0.5 && t->getGameY() == gy + 0.5) {
						t->toggleShowCoverage();
					}
				}
				return;
			}
			if (this->isInLevel() || !this->player.isAlive()) {
				// Can't build while enemies are attacking or if dead
				return;
			}
			if (this->getSelectedTower() < 0
				|| this->getSelectedTower() > static_cast<int>(this->getAllTowerTypes().size())) {
				// Invalid tower selected
				return;
			}
			if (this->getMap().getFiterGraph(false).getNode(gx, gy).isBlocked()
				|| this->getMap().getTerrainGraph(false).getNode(gx, gy).isBlocked()
				|| (this->getMap().getTerrainGraph(false).getStartNode()->getGameX() == gx
				&& this->getMap().getTerrainGraph(false).getStartNode()->getGameY() == gy)
				|| (this->getMap().getTerrainGraph(true).getStartNode()->getGameX() == gx
				&& this->getMap().getTerrainGraph(true).getStartNode()->getGameY() == gy)
				|| (this->getMap().getTerrainGraph(false).getGoalNode()->getGameX() == gx
				&& this->getMap().getTerrainGraph(false).getGoalNode()->getGameY() == gy)
				|| (this->getMap().getTerrainGraph(true).getGoalNode()->getGameX() == gx
				&& this->getMap().getTerrainGraph(true).getGoalNode()->getGameY() == gy)) {
				// Cannot build on this space...
				return;
			}
			// (We round for simplicity sake and also consistency.)
			if (this->player.getMoney() < std::ceil(this->getTowerType(this->getSelectedTower())->getCost())) {
				// Insufficient funds...
				return;
			}
			this->getMap().getFiterGraph(false).getNode(gx, gy).setBlockage(true);
			this->getMap().getFiterGraph(true).getNode(gx, gy).setBlockage(true);
			const auto my_pathfinder_ground = std::make_unique<pathfinding::Pathfinder>(this->getMap(), false,
				false, pathfinding::HeuristicStrategies::Manhattan);
			const auto my_pathfinder_air = std::make_unique<pathfinding::Pathfinder>(this->getMap(), true,
				false, pathfinding::HeuristicStrategies::Manhattan);
			if (!my_pathfinder_ground->checkPathExists() || !my_pathfinder_air->checkPathExists()) {
				// No path for enemies...
				this->getMap().getFiterGraph(false).getNode(gx, gy).setBlockage(false);
				this->getMap().getFiterGraph(true).getNode(gx, gy).setBlockage(false);
				return;
			}
			this->player.changeMoney(-this->getTowerType(this->getSelectedTower())->getCost());
			auto my_tower = std::make_unique<Tower>(this->device_resources, this->getMap(),
				this->getTowerType(this->getSelectedTower()),
				graphics::Color {0.f, 0.f, 0.f, 1.0f}, gx + 0.5, gy + 0.5);
			this->addTower(std::move(my_tower));
			this->debugUpdate(DebugUpdateStates::Terrain_Changed);
		}

		void MyGame::sellTower(int gx, int gy) {
			if (this->isInLevel() || !this->player.isAlive()) {
				// Can't sell while enemies are present or if dead
				return;
			}
			for (unsigned int i = 0; i < this->towers.size(); ++i) {
				const auto& t = this->towers[i];
				if (static_cast<int>(std::floor(t->getGameX())) == gx
					&& static_cast<int>(std::floor(t->getGameY())) == gy) {
					// (Refunds => 1/2 value unrounded)
					this->player.changeMoney(t->getCost() / 2.0);
					this->getMap().getFiterGraph(false).getNode(gx, gy).setBlockage(false);
					this->getMap().getFiterGraph(true).getNode(gx, gy).setBlockage(false);
					this->towers.erase(towers.begin() + i);
					this->debugUpdate(DebugUpdateStates::Terrain_Changed);
					break;
				}
			}
		}

		void MyGame::toggleAllRadii() noexcept {
			for (auto& t : this->towers) {
				t->toggleShowCoverage();
			}
		}


		std::wstring MyGame::getDefaultMapName(int new_challenge) const {
			switch (new_challenge) {
			case ID_CHALLENGE_LEVEL_EASY:
				return L"beginner";
			case ID_CHALLENGE_LEVEL_HARD:
				return L"experienced";
			case ID_CHALLENGE_LEVEL_EXPERT:
				return L"expert";
			case ID_CHALLENGE_LEVEL_NORMAL:
			default:
				return L"intermediate";
			}
		}

		const EnemyType* MyGame::getEnemyType(std::wstring name) const {
			for (auto& etype : this->enemy_types) {
				if (etype->getName() == name) return etype.get();
			}
			throw std::out_of_range {"Enemy does not exist."};
		}
	}
}