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
#include "./status_effects.hpp"

namespace hoffman::isaiah {
	namespace game {
		std::shared_ptr<MyGame> g_my_game {nullptr};

		MyGame::MyGame(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
			std::wistream& ground_terrain_file, std::wistream& air_terrain_file) :
			device_resources {dev_res},
			map {std::make_shared<GameMap>(ground_terrain_file, air_terrain_file)},
			enemy_types {},
			enemies {},
			shot_types {} {
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

		void MyGame::update() {
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
						// Reached goal....
						// TODO: Implement this section
					}
					enemies_to_remove.emplace_back(i);
				}
			}
			for (unsigned int i = 0; i < enemies_to_remove.size(); ++i) {
				// Remove dead/goal enemies (yes, the parenthesis are required...)
				this->enemies.erase(this->enemies.begin() + (enemies_to_remove[i] - i));
			}
			// Remove lock
			SetEvent(draw_event);
			CloseHandle(draw_event);
		}

		void MyGame::addEnemy(std::unique_ptr<Enemy>&& e) {
			this->enemies.emplace_back(std::move(e));
		}
	}
}