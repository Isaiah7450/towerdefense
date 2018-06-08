// File Author: Isaiah Hoffman
// File Created: March 26, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include "./../file_util.hpp"
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./enemy_type.hpp"
#include "./enemy.hpp"
#include "./game_level.hpp"
#include "./my_game.hpp"
#include "./shot_types.hpp"
#include "./shot.hpp"
#include "./status_effects.hpp"
#include "./tower_types.hpp"
#include "./tower.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace game {
		std::shared_ptr<MyGame> g_my_game {nullptr};

		MyGame::MyGame(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res, int clevel,
			std::wistream& ground_terrain_file, std::wistream& air_terrain_file) :
			device_resources {dev_res},
			map {std::make_shared<GameMap>(ground_terrain_file, air_terrain_file)},
			enemy_types {},
			enemies {},
			shot_types {},
			shots {},
			tower_types {},
			towers {},
			challenge_level {clevel} {
			this->ground_test_pf = std::make_shared<pathfinding::Pathfinder>(this->getMap(), false,
				false, pathfinding::HeuristicStrategies::Manhattan);
			this->air_test_pf = std::make_shared<pathfinding::Pathfinder>(this->getMap(), true,
				false, pathfinding::HeuristicStrategies::Manhattan);
		}

		MyGame::~MyGame() noexcept = default;

		void MyGame::debug_update(DebugUpdateStates cause) {
#if defined(DEBUG) || defined(_DEBUG)
			// Set lock
			auto draw_event = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, false, TEXT("can_draw"));
			if (!draw_event) {
				return;
			}
			ResetEvent(draw_event);
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
			// Remove lock
			SetEvent(draw_event);
			CloseHandle(draw_event);
#else
			UNREFERENCED_PARAMETER(cause);
#endif
		}

		void MyGame::resetState() {
			this->player = Player {};
			this->level = 1;
			this->difficulty = 1.0;
			this->my_level = nullptr;
			this->enemies.clear();
			this->towers.clear();
			this->shots.clear();
			this->is_paused = false;
			this->in_level = false;
			this->map->resetOtherGraphs();
			this->my_level_enemy_count = 0;
			this->did_lose_life = false;
			this->win_streak = 0;
			this->lose_streak = 0;
		}

		void MyGame::update() {
			if (this->is_paused || !this->in_level) {
				return;
			}
			// Check time before updating...
			static LARGE_INTEGER last_update_time = LARGE_INTEGER {0};
			auto my_times = winapi::MainWindow::getElapsedTime(last_update_time);
			if (my_times.second.QuadPart < math::getMicrosecondsInSecond() / game::logic_framerate) {
				Sleep(0);
				return;
			}
			last_update_time = my_times.first;
			// Set lock
			auto draw_event = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, false, TEXT("can_draw"));
			if (!draw_event) {
				return;
			}
			ResetEvent(draw_event);
			// Do processing...
			// Update level
			if (this->my_level) {
				this->my_level->update();
			}
			// Update enemies
			std::vector<int> enemies_to_remove {};
			for (unsigned int i = 0; i < this->enemies.size(); ++i) {
				if (this->enemies[i]->update()) {
					if (this->enemies[i]->isAlive()) {
						this->did_lose_life = true;
						this->player.changeHealth(-this->enemies[i]->getBaseType().getDamage());
						if (!this->player.isAlive()) {
							this->is_paused = true;
							// TODO: Finish implementation here
						}
					}
					else {
						// Alter influence score on Normal challenge level and higher
						auto& my_node = this->getMap().getInfluenceGraph(
							this->enemies[i]->getBaseType().isFlying()).getNode(
								static_cast<int>(std::floor(this->enemies[i]->getGameX())),
								static_cast<int>(std::floor(this->enemies[i]->getGameY())));
						my_node.setWeight(my_node.getWeight() + 1);
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
				if (this->shots[i]->update(*this->map, this->enemies)) {
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
				if (ret_value) {
					this->shots.emplace_back(std::move(ret_value));
				}
			}
			// Determine if the level is finished
			if (this->my_level && !this->my_level->hasEnemiesLeft() && this->enemies.empty()) {
				this->updateDifficulty();
				this->did_lose_life = false;
				this->in_level = false;
				for (auto& t : this->towers) {
					t->resetTower();
				}
				this->shots.clear();
				++this->level;
				this->player.changeMoney((this->level < 5 ? this->my_level_enemy_count * 2.5
					: this->level < 10 ? this->my_level_enemy_count * 1.5
					: this->level < 15 ? this->my_level_enemy_count
					: this->level < 25 ? this->my_level_enemy_count * 0.85
					: this->level < 35 ? this->my_level_enemy_count * 0.75
					: this->level < 50 ? this->my_level_enemy_count * 0.50
					: this->level < 75 ? this->my_level_enemy_count * 0.33
					: this->level < 150 ? this->my_level_enemy_count * 0.25
					: this->level < 250 ? this->my_level_enemy_count * 0.15
					: this->level < 500 ? this->my_level_enemy_count * 0.10
					: this->level < 1000 ? this->my_level_enemy_count * 0.05 : this->my_level_enemy_count * 0.01) * 1.33);
			}
			// Remove lock
			SetEvent(draw_event);
			CloseHandle(draw_event);
		}

		void MyGame::addEnemy(std::unique_ptr<Enemy>&& e) {
			this->enemies.emplace_back(std::move(e));
		}

		void MyGame::addTower(std::unique_ptr<Tower>&& t) {
			this->towers.emplace_back(std::move(t));
		}

		void MyGame::startWave() {
			this->is_paused = false;
			if (!this->isInLevel()) {
				this->in_level = true;
				try {
					this->load_level_data();
				}
				catch (const util::file::DataFileException& e) {
					MessageBox(nullptr, e.what(), L"Level Loading Error", MB_OK);
					std::exit(1);
				}
			}
		}

		void MyGame::buyTower(int gx, int gy) {
			if (this->isInLevel()) {
				// Can't build while enemies are attacking
				return;
			}
			if (this->getSelectedTower() < 0
				|| this->getSelectedTower() > static_cast<int>(this->getAllTowerTypes().size())) {
				// Invalid tower selected
				return;
			}
			if (this->getMap().getFiterGraph(false).getNode(gx, gy).isBlocked()
				|| this->getMap().getTerrainGraph(false).getNode(gx, gy).isBlocked()) {
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
			auto my_tower = std::make_unique<Tower>(this->device_resources, this->getTowerType(this->getSelectedTower()),
				graphics::Color {0.f, 0.f, 0.f, 1.0f}, gx + 0.5, gy + 0.5);
			this->addTower(std::move(my_tower));
		}

		void MyGame::sellTower(int gx, int gy) {
			if (this->isInLevel()) {
				// Can't sell while enemies are present
				return;
			}
			for (unsigned int i = 0; i < this->towers.size(); ++i) {
				const auto& t = this->towers[i];
				if (static_cast<int>(std::floor(t->getGameX())) == gx
					&& static_cast<int>(std::floor(t->getGameY())) == gy) {
					// (Refunds => 1/2 original price unrounded)
					this->player.changeMoney(t->getBaseType()->getCost() / 2.0);
					this->getMap().getFiterGraph(false).getNode(gx, gy).setBlockage(false);
					this->getMap().getFiterGraph(true).getNode(gx, gy).setBlockage(false);
					this->towers.erase(towers.begin() + i);
					break;
				}
			}
		}
	}
}