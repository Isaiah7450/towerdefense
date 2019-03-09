// File Author: Isaiah Hoffman
// File Created: March 29, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <Windowsx.h>
#include <Shobjidl.h>
#include "./../resource.h"
#include <process.h>
#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../graphics/graphics.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./../game/my_game.hpp"
#include "./editor.hpp"

using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace terrain_editor {
		std::shared_ptr<TerrainEditor> g_my_editor {nullptr};

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
			auto* my_game = game::g_my_game.get();
			// Global state stuff...
			terrain_editor::g_my_editor = std::make_shared<TerrainEditor>(static_cast<HWND>(data),
				my_game->getMap());
			auto& my_editor = *terrain_editor::g_my_editor;
			HINSTANCE my_h_inst = nullptr;
			if (!GetModuleHandleEx(0, nullptr, &my_h_inst)) {
				winapi::handleWindowsError(L"TE Thread: Terrain editor creation");
			}
			my_editor.createWindow(my_h_inst);
			try {
				my_editor.run();
			}
			catch (std::runtime_error& e) {
				MessageBoxA(my_editor.getHWND(), e.what(), "TE Thread Error", MB_OK);
			}
			return 0;
		}

		void TerrainEditor::createWindow(HINSTANCE h_inst) noexcept {
			static bool ran_once = false;
			if (!ran_once) {
				WNDCLASSEX wcex {sizeof(WNDCLASSEX), CS_DBLCLKS, TerrainEditor::windowProc,
					0, 0, h_inst, nullptr, LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_BACKGROUND+1),
					nullptr, TerrainEditor::class_name, nullptr};
				if (!RegisterClassEx(&wcex)) {
					winapi::handleWindowsError(L"TE Thread: Class registration");
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
			// Create window
			this->hwnd = CreateWindowEx(dwExStyles, TerrainEditor::class_name, TerrainEditor::window_name,
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

		void TerrainEditor::run() {
			// Create resource manager
			auto my_resources = std::make_shared<graphics::DX::DeviceResources2D>();
			// Create resources
			my_resources->createDeviceIndependentResources();
			if (FAILED(my_resources->createDeviceResources(this->hwnd))) {
				winapi::handleWindowsError(L"Creation of Direct2D resources");
			}
			// Create renderer
			auto my_renderer = std::make_unique<graphics::Renderer2D>(my_resources);
			// Show window
			ShowWindow(this->hwnd, SW_SHOWNORMAL);
			UpdateWindow(this->hwnd);
			// Keep track of mutex
			auto sync_mutex = OpenMutex(SYNCHRONIZE | MUTEX_MODIFY_STATE, false, TEXT("can_execute"));
			if (!sync_mutex) {
				winapi::handleWindowsError(L"TE Thread: Can execute mutex creation");
				return;
			}
			// Keep track of draw event
			auto draw_event = OpenEvent(SYNCHRONIZE, false, TEXT("can_draw"));
			if (!draw_event) {
				CloseHandle(sync_mutex);
				winapi::handleWindowsError(L"TE Thread: Can draw event creation");
				return;
			}
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
							// Remove .txt.
							this->map_name.erase(this->map_name.end() - 4, this->map_name.end());
							const wchar_t last_num = this->map_name.at(this->map_name.size() - 1);
							if (last_num >= L'0' && last_num <= L'9') {
								this->map_name.pop_back();
								this->map_name.push_back((last_num + 1));
							}
							else {
								this->map_name.push_back(L'0');
							}
							this->map_name += L".txt";
							WaitForSingleObject(sync_mutex, INFINITE);
							// Reset map to all mountainous terrain
							this->getMap().getTerrainGraph(false).clearGrid(pathfinding::GraphNode::blocked_space_weight);
							this->getMap().getTerrainGraph(true).clearGrid(pathfinding::GraphNode::blocked_space_weight);
							this->getMap().getTerrainGraph(false).setStartNode(0, 0);
							this->getMap().getTerrainGraph(true).setStartNode(0, 0);
							this->getMap().getTerrainGraph(false).setGoalNode(0, 0);
							this->getMap().getTerrainGraph(true).setGoalNode(0, 0);
							// Disable revert to save --> No save exists!
							constexpr MENUITEMINFO m_item {
								sizeof(MENUITEMINFO), MIIM_STATE, 0, MFS_DISABLED,
								0, nullptr, nullptr, nullptr, 0, nullptr, 0, nullptr
							};
							SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 1), ID_TE_ACTIONS_REVERT_TO_SAVE, false, &m_item);
							need_to_update = true;
							break;
						}
						case ID_TE_FILE_OPEN_MAP:
						{
							/*
							// CoCreate (whatever that is) the File Open Dialog object
							IFileDialog* open_dialog = nullptr;
							HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
								IID_PPV_ARGS(&open_dialog));
							if (FAILED(hr)) {
								MessageBox(this->hwnd, L"TE Thread: Open dialog box creation failed!", TerrainEditor::window_name,
									MB_OK);
								break;
							}
							// Create an event handling object and hook it to the dialog
							IFileDialogEvents* file_dialog_events = nullptr;
							hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&file_dialog_events));
							*/
							need_to_update = true;
							break;
						}
						case ID_TE_FILE_SAVE_MAP:
						{
							// Open save files
							std::wofstream ground_save_file {L"./resources/graphs/ground_"s + this->map_name};
							std::wofstream air_save_file {L"./resources/graphs/air_"s + this->map_name};
							if (!ground_save_file.good() || !air_save_file.good()) {
								MessageBox(this->hwnd, L"TE Thread: Could not save map!", this->window_name, MB_OK);
								break;
							}
							// Reenable revert to save
							constexpr MENUITEMINFO m_item {
								sizeof(MENUITEMINFO), MIIM_STATE, 0, MFS_ENABLED,
								0, nullptr, nullptr, nullptr, 0, nullptr, 0, nullptr
							};
							SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), 1), ID_TE_ACTIONS_REVERT_TO_SAVE, false, &m_item);
							// Output maps to save files
							ground_save_file << this->getTerrainGraph(false);
							air_save_file << this->getTerrainGraph(true);
							break;
						}
						case ID_TE_FILE_SAVE_MAP_AS:
							break;
						case ID_TE_FILE_QUIT:
							PostQuitMessage(0);
							break;
						case ID_TE_ACTIONS_REVERT_TO_SAVE:
						{
							// Open save files
							std::wifstream ground_save_file {L"./resources/graphs/ground_"s + this->map_name};
							std::wifstream air_save_file {L"./resources/graphs/air_"s + this->map_name};
							auto ground_grid {std::make_unique<pathfinding::Grid>(ground_save_file)};
							auto air_grid {std::make_unique<pathfinding::Grid>(air_save_file)};
							WaitForSingleObject(sync_mutex, INFINITE);
							this->getMap().resetOtherGraphs();
							this->getMap().setTerrainGraphs(std::move(ground_grid), std::move(air_grid));
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
					}
					case WM_LBUTTONDOWN:
					{
						// Obtain start coordinates
						auto gx = static_cast<int>(graphics::convertToGameX(GET_X_LPARAM(msg.lParam)));
						auto gy = static_cast<int>(graphics::convertToGameY(GET_Y_LPARAM(msg.lParam)));
						if (this->getMap().getTerrainGraph(false).verifyCoordinates(gx, gy)) {
							this->start_gx = gx;
							this->start_gy = gy;
						}
						break;
					}

					case WM_MOUSEMOVE:
					{
						if (msg.wParam == MK_LBUTTON) {
							// Update end coordinates
							auto gx = static_cast<int>(graphics::convertToGameX(GET_X_LPARAM(msg.lParam)));
							auto gy = static_cast<int>(graphics::convertToGameY(GET_Y_LPARAM(msg.lParam)));
							if (this->getMap().getTerrainGraph(false).verifyCoordinates(gx, gy)) {
								this->end_gx = gx;
								this->end_gy = gy;
							}
						}
						else if (msg.wParam == MK_RBUTTON) {
							// Cancel action
							this->start_gx = -1;
							this->start_gy = -1;
							this->end_gx = -1;
							this->end_gy = -1;
						}
						break;
					}
					case WM_LBUTTONUP:
					{
						// Update end coordinates
						auto new_gx = static_cast<int>(graphics::convertToGameX(GET_X_LPARAM(msg.lParam)));
						auto new_gy = static_cast<int>(graphics::convertToGameY(GET_Y_LPARAM(msg.lParam)));
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
								auto& selected_gnode = this->getMap().getTerrainGraph(false).getNode(gx, gy);
								auto& selected_anode = this->getMap().getTerrainGraph(true).getNode(gx, gy);
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
							}
						} // End outer for
						if (this->terrain_modifier_active) {
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
					WaitForSingleObject(sync_mutex, INFINITE);
					if (need_to_update) {
						// Note that we still have the mutex...
						// Update first if necessary
						game::g_my_game->debugUpdate(game::DebugUpdateStates::Terrain_Changed);
						ReleaseMutex(sync_mutex);
					}
					HRESULT hr = my_renderer->render(game::g_my_game, this->start_gx, this->start_gy,
						this->end_gx, this->end_gy, true);
					if (hr == D2DERR_RECREATE_TARGET) {
						my_resources->discardDeviceResources();
						my_resources->createDeviceResources(this->hwnd);
					}
					ReleaseMutex(sync_mutex);
				}
			}
			CloseHandle(sync_mutex);
			CloseHandle(draw_event);
		}
	}
}