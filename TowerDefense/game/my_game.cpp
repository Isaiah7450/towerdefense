// File Author: Isaiah Hoffman
// File Created: March 26, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./my_game.hpp"

namespace hoffman::isaiah {
	namespace game {
		std::shared_ptr<MyGame> g_my_game {nullptr};

		MyGame::MyGame(std::wistream& ground_terrain_file, std::wistream& air_terrain_file) :
			map {std::make_shared<GameMap>(ground_terrain_file, air_terrain_file)} {
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
			// Remove lock
			SetEvent(draw_event);
			CloseHandle(draw_event);
		}
	}
}