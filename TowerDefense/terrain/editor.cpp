// File Author: Isaiah Hoffman
// File Created: March 29, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <Windowsx.h>
#include <Shobjidl.h>
#include "./../resource.h"
#include <process.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../graphics/file_dialogs.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/other_dialogs.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./../game/my_game.hpp"
#include "./editor.hpp"

using namespace std::literals::string_literals;

namespace hoffman_isaiah {
	namespace terrain_editor {
		LRESULT CALLBACK TerrainEditor::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			switch (msg) {
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			case WM_QUIT:
				break;
			default:
				break;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		
		unsigned __stdcall terrain_editor_thread_init(void* data) {
			const auto* my_game = game::g_my_game.get();
			// Global state stuff...
			const auto my_editor_ptr = std::make_unique<TerrainEditor>(static_cast<HWND>(data),
				my_game->getMapBaseName());
			auto& my_editor = *my_editor_ptr;
			HINSTANCE my_h_inst = nullptr;
			if (!GetModuleHandleEx(0, nullptr, &my_h_inst)) {
				winapi::handleWindowsError(L"TE Thread: Terrain editor creation");
			}
			my_editor.createWindow(my_h_inst);
			try {
				my_editor.run();
			}
			catch (const std::runtime_error& e) {
				MessageBoxA(my_editor.getHWND(), e.what(), "TE Thread Error", MB_OK);
			}
			return 0;
		}

		TerrainEditor::TerrainEditor(HWND my_parent, std::wstring map_base_name) :
			parent_hwnd {my_parent},
			map_name {map_base_name} {
			this->reloadMap();
		}

		void TerrainEditor::createWindow(HINSTANCE h_inst) noexcept {
			static bool ran_once = false;
			if (!ran_once) {
				[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
				WNDCLASSEX wcex {sizeof(WNDCLASSEX), CS_DBLCLKS, TerrainEditor::windowProc,
					0, 0, h_inst, nullptr, LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_BACKGROUND+1),
					nullptr, TerrainEditor::class_name, nullptr};
				if (!RegisterClassEx(&wcex)) {
					winapi::handleWindowsError(L"TE Thread: Class registration");
				}
				}
				ran_once = true;
			}
			// Create and setup menu
			this->h_menu = LoadMenu(h_inst, MAKEINTRESOURCE(IDR_TERRAIN_EDITOR_MENU));
			this->updateMenu();
			// Determine window size and viewport
			SetRect(&this->rc, 0, 0, graphics::screen_width, graphics::screen_height);
			constexpr const auto dwStyles = WS_OVERLAPPEDWINDOW & (0xFFFFFFFFL ^ WS_MAXIMIZEBOX ^ WS_SIZEBOX);
			constexpr const auto dwExStyles = 0;
			AdjustWindowRectEx(&this->rc, dwStyles, (this->h_menu ? true : false), dwExStyles);
			const std::wstring my_window_name = TerrainEditor::window_name + L" ["s + this->map_name + L"]";
			// Create window
			this->hwnd = CreateWindowEx(dwExStyles, TerrainEditor::class_name, my_window_name.c_str(),
				dwStyles, CW_USEDEFAULT, CW_USEDEFAULT, this->rc.right - this->rc.left,
				this->rc.bottom - this->rc.top, this->parent_hwnd, this->h_menu, h_inst, nullptr);
			if (!this->hwnd) {
				winapi::handleWindowsError(L"TE Thread: Window creation");
			}
		}

		void TerrainEditor::updateMenu() noexcept {
			// TODO
			DrawMenuBar(this->hwnd);
		}

		void TerrainEditor::reloadMap() {
			// Note: Caller is responsible for syncing code.
			std::wifstream ground_terrain_file {game::g_my_game->getResourcesPath() + L"graphs/ground_graph_"
				+ this->map_name + L".txt"};
			std::wifstream air_terrain_file {game::g_my_game->getResourcesPath() + L"graphs/air_graph_"
				+ this->map_name + L".txt"};
			if (ground_terrain_file.bad() || ground_terrain_file.fail()
				|| air_terrain_file.bad() || air_terrain_file.fail()) {
				throw std::runtime_error {"File not found!"};
			}
			this->map = std::make_shared<game::GameMap>(ground_terrain_file, air_terrain_file);
			// Update the window's title.
			const std::wstring my_window_name = TerrainEditor::window_name + L" ["s
				+ this->map_name + L"]";
			SetWindowText(this->getHWND(), my_window_name.c_str());
		}

		void TerrainEditor::saveMap() {
			// Open save files
			std::wofstream ground_save_file {game::g_my_game->getResourcesPath() + L"graphs/ground_graph_"s
				+ this->map_name + L".txt"};
			std::wofstream air_save_file {game::g_my_game->getResourcesPath() + L"graphs/air_graph_"s
				+ this->map_name + L".txt"};
			if (!ground_save_file.good() || !air_save_file.good()) {
				MessageBox(this->getHWND(), L"TE Thread: Could not save map!", this->window_name, MB_OK);
				return;
			}
			// Output maps to save files
			ground_save_file << this->getTerrainGraph(false);
			air_save_file << this->getTerrainGraph(true);
			// Reenable revert to save
			winapi::enableMenuItem(hwnd, 1, ID_TE_ACTIONS_REVERT_TO_SAVE);
		}

		void TerrainEditor::run() {
			// Create resource manager
			auto my_resources = std::make_unique<graphics::DX::DeviceResources2D>();
			// Create resources
			my_resources->createDeviceIndependentResources();
			if (FAILED(my_resources->createDeviceResources(this->hwnd))) {
				winapi::handleWindowsError(L"Creation of Direct2D resources");
			}
			// Create renderer
			auto my_renderer = std::make_unique<graphics::Renderer2D>(my_resources.get());
			// Show window
			ShowWindow(this->hwnd, SW_SHOWNORMAL);
			UpdateWindow(this->hwnd);
			// Message Loop
#pragma warning(push)
#pragma warning(disable: 26494) // Code Analysis: type.5 --> Always initialize.
			MSG msg;
#pragma warning(pop)
			bool keep_looping = true;
			bool need_to_update = false;
			while (keep_looping) {
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					// Handle user interaction and WM_QUIT message
					switch (msg.message) {
					case WM_COMMAND:
					{
						switch (msg.wParam) {
						case ID_TE_FILE_NEW_MAP:
						{
							std::wstring new_map_name = this->map_name;
							const wchar_t last_num = new_map_name.at(new_map_name.size() - 1);
							if (last_num >= L'0' && last_num <= L'9') {
								new_map_name.pop_back();
								new_map_name.push_back((last_num + 1));
							}
							else {
								new_map_name.push_back(L'0');
							}
							winapi::TerrainEditorNewMapDialog my_map_dialog {this->getHWND(), GetModuleHandle(nullptr), new_map_name};
							if (my_map_dialog.isGood()) {
								this->map_name = my_map_dialog.getName();
								// Update the window's title to include the map's name.
								const std::wstring my_window_name = TerrainEditor::window_name + L" ["s
									+ this->map_name + L"]";
								SetWindowText(hwnd, my_window_name.c_str());
								// Reset map to all mountainous terrain
								this->map->getTerrainGraph(false).clearGrid(my_map_dialog.getRows(), my_map_dialog.getColumns(),
									pathfinding::GraphNode::blocked_space_weight);
								this->map->getTerrainGraph(true).clearGrid(my_map_dialog.getRows(), my_map_dialog.getColumns(),
									pathfinding::GraphNode::blocked_space_weight);
								this->map->getTerrainGraph(false).setStartNode(0, 0);
								this->map->getTerrainGraph(true).setStartNode(0, 0);
								this->map->getTerrainGraph(false).setGoalNode(0, 0);
								this->map->getTerrainGraph(true).setGoalNode(0, 0);
								this->map->resetOtherGraphs();
								// Disable revert to save --> No save exists!
								winapi::disableMenuItem(hwnd, 1, ID_TE_ACTIONS_REVERT_TO_SAVE);
								need_to_update = true;
							}
							break;
						}
						case ID_TE_FILE_OPEN_MAP:
						{
							const winapi::TerrainEditorOpenMapDialog my_dialog {this->getHWND(), GetModuleHandle(nullptr)};
							if (my_dialog.isGood()) {
								const std::wstring old_name = this->map_name;
								this->map_name = my_dialog.getName();
								try {
									this->reloadMap();
								}
								catch (...) {
									MessageBox(hwnd, L"Error: Could not load the requested map.", L"TE: Open Map Failed!", MB_OK | MB_ICONERROR);
									this->map_name = old_name;
									this->reloadMap();
								}
								need_to_update = true;
							}
							break;
						}
						case ID_TE_FILE_SAVE_MAP:
						{
							this->saveMap();
							break;
						}
						case ID_TE_FILE_SAVE_MAP_AS:
						{
							const winapi::TerrainEditorSaveMapAsDialog my_dialog {this->getHWND(), GetModuleHandle(nullptr), this->map_name};
							if (my_dialog.isGood()) {
								const std::wstring ground_filename = game::g_my_game->getResourcesPath() + L"graphs/ground_graph_"s
									+ my_dialog.getName() + L".txt";
								bool go_ahead = true;
								if (my_dialog.showOvewriteConfirmation() && std::filesystem::exists(ground_filename)) {
									const int my_result = MessageBox(this->hwnd, (ground_filename + L" already exists. Overwrite anyway?").c_str(),
										L"TE: Save As - Confirm Overwrite", MB_YESNO | MB_ICONWARNING);
									if (my_result == IDNO) {
										go_ahead = false;
									}
								}
								if (go_ahead) {
									this->map_name = my_dialog.getName();
									this->saveMap();
									// Update the window's title.
									const std::wstring my_window_name = TerrainEditor::window_name + L" ["s
										+ this->map_name + L"]";
									SetWindowText(hwnd, my_window_name.c_str());
								}
							}
							break;
						}
						case ID_TE_FILE_QUIT:
							PostQuitMessage(0);
							break;
						case ID_TE_ACTIONS_REVERT_TO_SAVE:
						{
							this->reloadMap();
							need_to_update = true;
							break;
						}
						case ID_TE_ACTIONS_SET_GROUND_START:
						case ID_TE_ACTIONS_SET_AIR_START:
						case ID_TE_ACTIONS_SET_GROUND_END:
						case ID_TE_ACTIONS_SET_AIR_END:
							this->selected_terrain_modifier = static_cast<int>(msg.wParam);
							this->terrain_modifier_active = true;
							break;
						case ID_TE_ACTIONS_INCREASE_WEIGHTS:
							++this->terrain_weights_adjust;
							if (this->terrain_weights_adjust > 9) {
								this->terrain_weights_adjust = 9;
							}
							break;
						case ID_TE_ACTIONS_DECREASE_WEIGHTS:
							--this->terrain_weights_adjust;
							if (this->terrain_weights_adjust < 0) {
								this->terrain_weights_adjust = 0;
							}
							break;
						case ID_TE_ACTIONS_TOGGLE_GROUND_WEIGHTS:
							this->show_ground_weights = !this->show_ground_weights;
							break;
						case ID_TE_ACTIONS_TOGGLE_AIR_WEIGHTS:
							this->show_air_weights = !this->show_air_weights;
							break;
						case ID_TE_TERRAIN_TYPES_NONE:
						case ID_TE_TERRAIN_TYPES_GRASS:
						case ID_TE_TERRAIN_TYPES_SWAMP:
						case ID_TE_TERRAIN_TYPES_FOREST:
						case ID_TE_TERRAIN_TYPES_CAVE:
						case ID_TE_TERRAIN_TYPES_OCEAN:
						case ID_TE_TERRAIN_TYPES_MOUNTAIN:
							this->selected_terrain_type = static_cast<int>(msg.wParam);
							break;
						default:
							break;
						}
						this->updateMenu();
						break;
					}
					case WM_LBUTTONDOWN:
					case WM_RBUTTONDOWN:
					{
						// Obtain start coordinates
						const auto gx = static_cast<int>(this->getMap().convertToGameX(GET_X_LPARAM(msg.lParam)));
						const auto gy = static_cast<int>(this->getMap().convertToGameY(GET_Y_LPARAM(msg.lParam)));
						if (this->getMap().getTerrainGraph(false).verifyCoordinates(gx, gy)) {
							this->start_gx = gx;
							this->start_gy = gy;
						}
						break;
					}
					case WM_MOUSEMOVE:
					{
						// Update end coordinates
						const auto gx = static_cast<int>(this->getMap().convertToGameX(GET_X_LPARAM(msg.lParam)));
						const auto gy = static_cast<int>(this->getMap().convertToGameY(GET_Y_LPARAM(msg.lParam)));
						this->end_gx = gx;
						this->end_gy = gy;
						break;
					}
					case WM_LBUTTONUP:
					case WM_RBUTTONUP:
					{
						// Update end coordinates
						const auto new_gx = static_cast<int>(this->getMap().convertToGameX(GET_X_LPARAM(msg.lParam)));
						const auto new_gy = static_cast<int>(this->getMap().convertToGameY(GET_Y_LPARAM(msg.lParam)));
						this->end_gx = math::get_max(new_gx, this->start_gx);
						this->end_gy = math::get_max(new_gy, this->start_gy);
						this->start_gx = math::get_min(this->start_gx, new_gx);
						this->start_gy = math::get_min(this->start_gy, new_gy);
						if (!(this->getMap().getTerrainGraph(false).verifyCoordinates(this->start_gx, this->start_gy)
							&& this->getMap().getTerrainGraph(false).verifyCoordinates(this->end_gx, this->end_gy))) {
							this->start_gx = -1;
							this->start_gy = -1;
							this->end_gx = -1;
							this->end_gy = -1;
							break;
						}
						// Update nodes with new terrain
						for (int gx = this->start_gx; gx <= this->end_gx; ++gx) {
							for (int gy = this->start_gy; gy <= this->end_gy; ++gy) {
								auto& selected_gnode = this->map->getTerrainGraph(false).getNode(gx, gy);
								auto& selected_anode = this->map->getTerrainGraph(true).getNode(gx, gy);
								switch (this->selected_terrain_type) {
								case ID_TE_TERRAIN_TYPES_GRASS:
									selected_gnode.setWeight(1);
									selected_anode.setWeight(1);
									break;
								case ID_TE_TERRAIN_TYPES_SWAMP:
									selected_gnode.setWeight(2);
									selected_anode.setWeight(1);
									break;
								case ID_TE_TERRAIN_TYPES_FOREST:
									selected_gnode.setWeight(2);
									selected_anode.setWeight(3);
									break;
								case ID_TE_TERRAIN_TYPES_CAVE:
									selected_gnode.setWeight(1);
									selected_anode.setBlockage(true);
									break;
								case ID_TE_TERRAIN_TYPES_OCEAN:
									selected_gnode.setBlockage(true);
									selected_anode.setWeight(1);
									break;
								case ID_TE_TERRAIN_TYPES_MOUNTAIN:
									selected_gnode.setBlockage(true);
									selected_anode.setBlockage(true);
									break;
								case ID_TE_TERRAIN_TYPES_NONE:
								default:
									break;
								}
								// Apply terrain weight adjustment
								selected_gnode.setWeight(selected_gnode.getWeight() + this->terrain_weights_adjust);
								selected_anode.setWeight(selected_anode.getWeight() + this->terrain_weights_adjust);
								// Right-click to paint over with default terrain (grass).
								if (msg.message == WM_RBUTTONUP) {
									selected_gnode.setWeight(1);
									selected_anode.setWeight(1);
								}
							}
						} // End outer for
						if (this->terrain_modifier_active && msg.message != WM_RBUTTONUP) {
							// Apply terrain modifier
							// Use top-left hand corner of rectangle to apply modifier
							switch (this->selected_terrain_modifier) {
							case ID_TE_ACTIONS_SET_GROUND_START:
								this->getTerrainGraph(false).setStartNode(this->start_gx, this->start_gy);
								this->getTerrainGraph(false).getNode(this->start_gx, this->start_gy).setBlockage(false);
								break;
							case ID_TE_ACTIONS_SET_AIR_START:
								this->getTerrainGraph(true).setStartNode(this->start_gx, this->start_gy);
								this->getTerrainGraph(true).getNode(this->start_gx, this->start_gy).setBlockage(false);
								break;
							case ID_TE_ACTIONS_SET_GROUND_END:
								this->getTerrainGraph(false).setGoalNode(this->start_gx, this->start_gy);
								this->getTerrainGraph(false).getNode(this->start_gx, this->start_gy).setBlockage(false);
								break;
							case ID_TE_ACTIONS_SET_AIR_END:
								this->getTerrainGraph(true).setGoalNode(this->start_gx, this->start_gy);
								this->getTerrainGraph(true).getNode(this->start_gx, this->start_gy).setBlockage(false);
								break;
							default:
								break;
							}
							this->terrain_modifier_active = false;
						}
						need_to_update = true;
						// Reset start and end coordinates
						this->start_gx = -1;
						this->start_gy = -1;
						this->end_gx = -1;
						this->end_gy = -1;
						break;
					}
					case WM_QUIT:
						keep_looping = false;
						break;
					default:
						break;
					}
				}
				else {
					// Render scene
					if (need_to_update) {
						// Note that we still have the mutex...
						// Update first if necessary
						// game::g_my_game->debugUpdate(game::DebugUpdateStates::Terrain_Changed);
					}
					const HRESULT hr = my_renderer->render(*this, this->start_gx, this->start_gy,
						this->end_gx, this->end_gy);
					if (hr == D2DERR_RECREATE_TARGET) {
						my_resources->discardDeviceResources();
						my_resources->createDeviceResources(this->hwnd);
					}
				}
			}
		}
	}
}