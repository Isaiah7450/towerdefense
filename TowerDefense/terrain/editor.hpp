#pragma once
// File Author: Isaiah Hoffman
// File Created: March 29, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include "./../globals.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"

namespace hoffman::isaiah {
	namespace terrain_editor {
		unsigned __stdcall terrain_editor_thread_init(void* data);

		class TerrainEditor {
		public:
			TerrainEditor(HWND my_parent, game::GameMap& game_map) noexcept :\
				parent_hwnd {my_parent},
				map {game_map} {
			}
			/// <summary>Creates the window.</summary>
			void createWindow(HINSTANCE h_inst) noexcept;
			/// <summary>Runs the terrain editor.</summary>
			void run();
			/// <summary>Window's procedure for the terrain editor.</summary>
			static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			// Getters
			HWND getHWND() const noexcept {
				return this->hwnd;
			}
			const game::GameMap& getMap() const noexcept {
				return this->map;
			}
			game::GameMap& getMap() noexcept {
				return this->map;
			}
			const pathfinding::Grid& getTerrainGraph(bool return_air_map) const noexcept {
				return this->getMap().getTerrainGraph(return_air_map);
			}
			pathfinding::Grid& getTerrainGraph(bool return_air_map) noexcept {
				return this->getMap().getTerrainGraph(return_air_map);
			}
		protected:
			/// <summary>Updates the menu bar.</summary>
			void updateMenu() noexcept;
		private:
			/// <summary>Handle to the parent window of the terrain editor.</summary>
			HWND parent_hwnd;
			/// <summary>Handle to the window with the terrain editor.</summary>
			HWND hwnd {nullptr};
			/// <summary>Rectangle that stores the window's dimensions.</summary>
			RECT rc {};
			/// <summary>Handle to the window's menu.</summary>
			HMENU h_menu {nullptr};
			/// <summary>Reference to the terrain editor map.</summary>
			game::GameMap& map;
			/// <summary>The currently selected terrain type.</summary>
			int selected_terrain_type {0};
			/// <summary>The last selected terrain modifier.</summary>
			int selected_terrain_modifier {0};
			/// <summary>Controls whether or not the terrain modifier is applied to a square.</summary>
			bool terrain_modifier_active {false};
			/// <summary>Stores the starting x-position of the mouse.</summary>
			int start_gx {-1};
			/// <summary>Stores the starting y-position of the mouse.</summary>
			int start_gy {-1};
			/// <summary>Stores the ending x-position of the mouse.</summary>
			int end_gx {-1};
			/// <summary>Stores the ending y-position of the mouse.</summary>
			int end_gy {-1};
			// The window's class
			static constexpr auto class_name {L"terrain_editor_window"};
			// The window's name
			static constexpr auto window_name {L"Isaiah's tower defense - terrain editor"};
		};
	}
}