#pragma once
// File Author: Isaiah Hoffman
// File Created: March 29, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <string>
#include "./../globals.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"

namespace hoffman::isaiah {
	namespace terrain_editor {
		unsigned __stdcall terrain_editor_thread_init(void* data);

		class TerrainEditor {
		public:
			TerrainEditor(HWND my_parent, std::wstring map_base_name);
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
				return *this->map;
			}
			const pathfinding::Grid& getTerrainGraph(bool return_air_map) const noexcept {
				return this->getMap().getTerrainGraph(return_air_map);
			}
			pathfinding::Grid& getTerrainGraph(bool return_air_map) noexcept {
				return this->map->getTerrainGraph(return_air_map);
			}
			bool areGroundWeightsActive() const noexcept {
				return this->show_ground_weights;
			}
			bool areAirWeightsActive() const noexcept {
				return this->show_air_weights;
			}
		protected:
			/// <summary>Updates the menu bar.</summary>
			void updateMenu() noexcept;
			// Note: Caller is responsible for syncing this method.
			/// <summary>Reloads the map based on the stored map name.</summary>
			void reloadMap();
			/// <summary>Saves the map based using the stored map name.</summary>
			void saveMap();
		private:
			/// <summary>Handle to the parent window of the terrain editor.</summary>
			HWND parent_hwnd;
			/// <summary>Handle to the window with the terrain editor.</summary>
			HWND hwnd {nullptr};
			/// <summary>Rectangle that stores the window's dimensions.</summary>
			RECT rc {};
			/// <summary>Handle to the window's menu.</summary>
			HMENU h_menu {nullptr};
			/// <summary>The terrain editor's map which may be different from the map in use by the game.</summary>
			std::shared_ptr<game::GameMap> map {nullptr};
			/// <summary>The currently selected terrain type.</summary>
			int selected_terrain_type {0};
			/// <summary>The last selected terrain modifier.</summary>
			int selected_terrain_modifier {0};
			/// <summary>Controls whether or not the terrain modifier is applied to a square.</summary>
			bool terrain_modifier_active {false};
			/// <summary>The amount to add to the default terrain weights for each terrain.</summary>
			int terrain_weights_adjust {0};
			/// <summary>Show the ground weights of every square?</summary>
			bool show_ground_weights {false};
			/// <summary>Show the air weights of every square?</summary>
			bool show_air_weights {false};
			/// <summary>Stores the starting x-position of the mouse.</summary>
			int start_gx {-1};
			/// <summary>Stores the starting y-position of the mouse.</summary>
			int start_gy {-1};
			/// <summary>Stores the ending x-position of the mouse.</summary>
			int end_gx {-1};
			/// <summary>Stores the ending y-position of the mouse.</summary>
			int end_gy {-1};
			/// <summary>Stores the current name used when saving the map.</summary>
			std::wstring map_name {L"default"};
			// The window's class
			static constexpr auto class_name {L"terrain_editor_window"};
			// The window's name
			static constexpr auto window_name {L"A Shaping War: Isaiah's tower defense game (Terrain Editor)"};
		};

		// Global state is not the best, but it tends to be the most trivial solution.
		extern std::shared_ptr<TerrainEditor> g_my_editor;
	}
}