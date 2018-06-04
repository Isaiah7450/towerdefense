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
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./enemy_type.hpp"
#include "./enemy.hpp"
#include "./my_game.hpp"
#include "./shot_types.hpp"
#include "./shot.hpp"
#include "./status_effects.hpp"
#include "./tower_types.hpp"
#include "./tower.hpp"

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
			// @TODO: Define
			this->player = Player {};
			this->level = 1;
			this->difficulty = 1.0;
			this->enemies.clear();
			this->towers.clear();
			this->shots.clear();
			this->is_paused = false;
			this->in_level = false;
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
			// Update enemies
			std::vector<int> enemies_to_remove {};
			for (unsigned int i = 0; i < this->enemies.size(); ++i) {
				if (this->enemies[i]->update()) {
					if (this->enemies[i]->isAlive()) {
						this->player.changeHealth(this->enemies[i]->getBaseType().getDamage());
						if (!this->player.isAlive()) {
							this->is_paused = true;
							// TODO: Finish implementation here
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

		void MyGame::buyTower(int gx, int gy) {
			UNREFERENCED_PARAMETER(gx);
			UNREFERENCED_PARAMETER(gy);
		}

		void MyGame::sellTower(int gx, int gy) {
			for (unsigned int i = 0; i < this->towers.size(); ++i) {
				const auto& t = this->towers[i];
				if (static_cast<int>(std::floor(t->getGameX())) == gx
					&& static_cast<int>(std::floor(t->getGameY())) == gy) {
					this->player.changeMoney(t->getBaseType()->getCost() / 2.0);
					this->towers.erase(towers.begin() + i);
					break;
				}
			}
		}
	}
}