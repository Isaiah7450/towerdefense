// File Author: Isaiah Hoffman
// File Created: March 29, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <Windowsx.h>
#include "./../resource.h"
#include <process.h>
#include <string>
#include <utility>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../graphics/graphics_DX.hpp"
#include "./../graphics/graphics.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./../game/my_game.hpp"
#include "./editor.hpp"

namespace hoffman::isaiah {
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
			auto* my_game = game::g_my_game.get();
			auto my_editor = std::make_unique<TerrainEditor>(static_cast<HWND>(data), my_game->getMap());
			HINSTANCE my_h_inst = nullptr;
			if (!GetModuleHandleEx(0, nullptr, &my_h_inst)) {
				winapi::handleWindowsError(L"TE Thread: Terrain editor creation");
			}
			my_editor->createWindow(my_h_inst);
			my_editor->run();
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
			// Keep track of draw event
			auto draw_event = OpenEvent(SYNCHRONIZE, false, TEXT("can_draw"));
			if (!draw_event) {
				winapi::handleWindowsError(L"TE Thread: Can draw event creation");
				return;
			}
			// Message Loop
			MSG msg;
			bool keep_looping = true;
			while (keep_looping) {
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					// Handle user interaction and WM_QUIT message
					switch (msg.message) {
					case WM_COMMAND:
					{
						switch (msg.wParam) {
						case ID_TE_ACTIONS_SET_GROUND_START:
						case ID_TE_ACTIONS_SET_AIR_START:
						case ID_TE_ACTIONS_SET_GROUND_END:
						case ID_TE_ACTIONS_SET_AIR_END:
							this->selected_terrain_modifier = msg.wParam;
							this->terrain_modifier_active = true;
							break;
						case ID_TE_TERRAIN_TYPES_NONE:
						case ID_TE_TERRAIN_TYPES_GRASS:
						case ID_TE_TERRAIN_TYPES_SWAMP:
						case ID_TE_TERRAIN_TYPES_FOREST:
						case ID_TE_TERRAIN_TYPES_CAVE:
						case ID_TE_TERRAIN_TYPES_OCEAN:
						case ID_TE_TERRAIN_TYPES_MOUNTAIN:
							this->selected_terrain_type = msg.wParam;
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
							}
						} // End outer for
						if (this->terrain_modifier_active) {
							// Apply terrain modifier
							// Use top-left hand corner of rectangle to apply modifier
							switch (this->selected_terrain_modifier) {
							case ID_TE_ACTIONS_SET_GROUND_START:
								this->getTerrainGraph(false).setStartNode(this->start_gx, this->start_gy);
								break;
							case ID_TE_ACTIONS_SET_AIR_START:
								this->getTerrainGraph(true).setStartNode(this->start_gx, this->start_gy);
								break;
							case ID_TE_ACTIONS_SET_GROUND_END:
								this->getTerrainGraph(false).setGoalNode(this->start_gx, this->start_gy);
								break;
							case ID_TE_ACTIONS_SET_AIR_END:
								this->getTerrainGraph(true).setGoalNode(this->start_gx, this->start_gy);
								break;
							default:
								break;
							}
							this->terrain_modifier_active = false;
						}
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
					WaitForSingleObject(draw_event, INFINITE);
					HRESULT hr = my_renderer->render(game::g_my_game, this->start_gx, this->start_gy,
						this->end_gx, this->end_gy);
					if (hr == D2DERR_RECREATE_TARGET) {
						my_resources->discardDeviceResources();
						my_resources->createDeviceResources(this->hwnd);
					}
				}
			}
			CloseHandle(draw_event);
		}
	}
}