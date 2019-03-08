// File Author: Isaiah Hoffman
// File Created: March 9, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <Windowsx.h>
#include "./resource.h"
#include <process.h>
#include <memory>
#include <string>
#include <strsafe.h>
#include <fstream>
#include <iostream>
#include "./globals.hpp"
#include "./ih_math.hpp"
#include "./file_util.hpp"
#include "./main.hpp"
#include "./graphics/graphics_DX.hpp"
#include "./graphics/graphics.hpp"
#include "./graphics/info_dialogs.hpp"
#include "./pathfinding/grid.hpp"
#include "./game/enemy.hpp"
#include "./game/enemy_type.hpp"
#include "./game/my_game.hpp"
#include "./game/tower.hpp"
#include "./terrain/editor.hpp"

using namespace std::literals::string_literals;
namespace ih = hoffman::isaiah;
// WinMain function
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	auto my_window = ih::winapi::MainWindow {hInstance};
	my_window.run(nCmdShow);
	return 0;
}

namespace hoffman::isaiah {
	namespace graphics {
		int screen_width {860};
		int screen_height {645};
		int grid_width {40};
		int grid_height {35};
	}

	namespace winapi {
		LARGE_INTEGER MainWindow::qpc_frequency {0};

		LRESULT CALLBACK MainWindow::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			switch (msg) {
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				break;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		unsigned __stdcall update_thread_init(void* data) {
			UNREFERENCED_PARAMETER(data);
			try {
				auto update_thread_init_event = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, false, TEXT("update_thread_ready"));
				if (!update_thread_init_event) {
					throw std::runtime_error {"Update thread ready event does not exist."};
				}
				auto update_event = OpenEvent(SYNCHRONIZE, true, TEXT("can_update"));
				if (!update_event) {
					CloseHandle(update_thread_init_event);
					throw std::runtime_error {"Can update event does not exist."};
				}
				auto sync_mutex = OpenMutex(SYNCHRONIZE | MUTEX_MODIFY_STATE, false, TEXT("can_execute"));
				if (!sync_mutex) {
					CloseHandle(update_thread_init_event);
					CloseHandle(update_event);
					throw std::runtime_error {"Can execute mutex does not exist."};
				}
				// Initialize the game's state
				auto my_game = game::g_my_game;
				my_game->init_enemy_types();
				my_game->init_shot_types();
				my_game->init_tower_types();
				my_game->load_global_level_data();
				my_game->load_global_misc_data();
				std::wifstream default_save_file {game::default_save_file_name};
				if (!default_save_file.bad() && !default_save_file.fail()) {
					try {
						my_game->loadGame(default_save_file);
					}
					catch (...) {
						MessageBox(nullptr, L"Error: Corrupted saved file! Reverting to new game.", L"Corrupted Save",
							MB_OK | MB_ICONERROR);
						// Reset state and save over the corrupted file...
						my_game->resetState();
						std::wofstream save_file {game::default_save_file_name};
						if (!save_file.bad() && !save_file.fail()) {
							my_game->saveGame(save_file);
						}
					}
				}
				// Force creation of message queue
				MSG msg;
				PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
				SetEvent(update_thread_init_event);
				// Message Loop
				bool keep_running = true;
				while (keep_running) {
					BOOL ret_value = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
					if (ret_value > 0) {
						switch (msg.message) {
						case WM_COMMAND:
						{
							switch (msg.wParam) {
							case ID_MM_FILE_NEW_GAME:
								WaitForSingleObject(sync_mutex, INFINITE);
								my_game->resetState();
								ReleaseMutex(sync_mutex);
								break;
							case ID_MM_FILE_SAVE_GAME:
							{
								std::wofstream save_file {game::default_save_file_name};
								if (save_file.fail() || save_file.bad()) {
									MessageBox(nullptr, L"Could not save game.", L"Save failed!", MB_ICONEXCLAMATION | MB_OK);
								}
								else {
									WaitForSingleObject(sync_mutex, INFINITE);
									my_game->saveGame(save_file);
									ReleaseMutex(sync_mutex);
								}
								break;
							}
							case ID_MM_ACTIONS_NEXT_WAVE:
								WaitForSingleObject(sync_mutex, INFINITE);
								my_game->startWave();
								ReleaseMutex(sync_mutex);
								break;
							case ID_MM_TOWERS_BUY_TOWER:
							{
								auto my_gx = static_cast<int>(GET_X_LPARAM(msg.lParam));
								auto my_gy = static_cast<int>(GET_Y_LPARAM(msg.lParam));
								WaitForSingleObject(sync_mutex, INFINITE);
								my_game->buyTower(my_gx, my_gy);
								ReleaseMutex(sync_mutex);
								break;
							}
							case ID_MM_TOWERS_SELL_TOWER:
							{
								auto my_gx = static_cast<int>(GET_X_LPARAM(msg.lParam));
								auto my_gy = static_cast<int>(GET_Y_LPARAM(msg.lParam));
								WaitForSingleObject(sync_mutex, INFINITE);
								my_game->sellTower(my_gx, my_gy);
								ReleaseMutex(sync_mutex);
								break;
							}
							default:
								break;
							}
							break;
						}
						case WM_DESTROY:
						case WM_QUIT:
							keep_running = false;
							break;
						default:
							break;
						}
					}
					else if (ret_value == -1) {
						// Error
						winapi::handleWindowsError(L"Update thread ");
					}
					if (keep_running) {
						// Update stuff
						const HANDLE update_handles[] = {
							update_event, sync_mutex
						};
						WaitForMultipleObjects(2, update_handles, true, INFINITE);
						my_game->update();
						ReleaseMutex(sync_mutex);
					}
				}
				CloseHandle(update_thread_init_event);
				CloseHandle(update_event);
				CloseHandle(sync_mutex);
			}
			catch (const util::file::DataFileException& e) {
				MessageBox(nullptr, e.what(), L"Error loading data file", MB_OK | MB_ICONEXCLAMATION);
				std::exit(1);
			}
#if 0
			catch (const std::exception& e) {
				UNREFERENCED_PARAMETER(e);
				std::exit(1);
			}
#endif
			return 0;
		}

		[[noreturn]] void handleWindowsError(std::wstring lpszFunction) {
			// (Copied straight from help files... Hopefully it works!)
			// (I did replace the C-style casts with a C++ version.)
			// Retrieve the system error message for the last-error code
			LPVOID lpMsgBuf {nullptr};
			LPVOID lpDisplayBuf {nullptr};
			const DWORD dw = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
				| FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&lpMsgBuf), 0,
				nullptr);
			// Display the error message and exit the process
			lpDisplayBuf = LocalAlloc(LMEM_ZEROINIT,
				lstrlen(static_cast<LPWSTR>(lpMsgBuf)) + lpszFunction.size() + 120 * sizeof(TCHAR));
			if (!lpDisplayBuf) {
				MessageBox(nullptr, L"Critical Error: Out of memory!", TEXT("Error"), MB_OK);
				ExitProcess(dw);
			}
			StringCchPrintf(static_cast<LPTSTR>(lpDisplayBuf), LocalSize(lpDisplayBuf) / sizeof(TCHAR),
				TEXT("%s failed with error %d: %s"), lpszFunction.c_str(), dw, static_cast<LPWSTR>(lpMsgBuf));
			MessageBox(nullptr, static_cast<LPWSTR>(lpDisplayBuf), TEXT("Error"), MB_OK);
			LocalFree(lpMsgBuf);
			LocalFree(lpDisplayBuf);
			ExitProcess(dw);
		}

		MainWindow::MainWindow(HINSTANCE h_inst) :
			h_instance {h_inst} {
			// Register window class
			WNDCLASSEX wnd_class {
				sizeof(WNDCLASSEX), CS_DBLCLKS, MainWindow::windowProc, 0, 0, this->h_instance, nullptr,
				LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_BACKGROUND+1),
				nullptr, MainWindow::class_name, nullptr
			};
			if (!RegisterClassEx(&wnd_class)) {
				winapi::handleWindowsError(L"Registration of window class");
			}
			// Keep reference to menu
			this->h_menu = LoadMenu(this->h_instance, MAKEINTRESOURCE(IDR_MAIN_MENU));
			// Determine window size and viewport
			SetRect(&this->rc, 0, 0, graphics::screen_width, graphics::screen_height);
			constexpr const auto dwStyles = WS_OVERLAPPEDWINDOW & (0xFFFFFFFFL ^ WS_MAXIMIZEBOX ^ WS_SIZEBOX);
			constexpr const auto dwExStyles = 0;
			AdjustWindowRectEx(&this->rc, dwStyles, (this->h_menu ? true : false), dwExStyles);
			// Create window
			this->hwnd = CreateWindowEx(dwExStyles, MainWindow::class_name, MainWindow::window_name,
				dwStyles, CW_USEDEFAULT, CW_USEDEFAULT, this->rc.right - this->rc.left,
				this->rc.bottom - this->rc.top, nullptr, this->h_menu, this->h_instance, nullptr);
			if (!this->hwnd) {
				winapi::handleWindowsError(L"Creation of window");
			}
			// Setup time counter
			QueryPerformanceFrequency(&MainWindow::qpc_frequency);
		}

		void MainWindow::run(int n_cmd_show) {
			// Create resource manager
			auto my_resources = std::make_shared<graphics::DX::DeviceResources2D>();
			// Create resources
			my_resources->createDeviceIndependentResources();
			if (FAILED(my_resources->createDeviceResources(this->hwnd))) {
				winapi::handleWindowsError(L"Creation of Direct2D resources");
			}
			// Create renderer
			auto my_renderer = std::make_unique<graphics::Renderer2D>(my_resources);
			// Create game state
			constexpr const auto ground_terrain_filename = L"./resources/graphs/ground_graph.txt";
			constexpr const auto air_terrain_filename = L"./resources/graphs/air_graph.txt";
			std::wifstream ground_terrain_file {ground_terrain_filename};
			std::wifstream air_terrain_file {air_terrain_filename};
			if (!ground_terrain_file.good() || !air_terrain_file.good()) {
				MessageBox(hwnd, L"Could not load terrain graphs.", L"Tower defense startup error", MB_OK);
				return;
			}
			// Store in global variable --> It's the only way!
			// Actually, it isn't, but it's the most intuitive (and least overhead involved) way to do it.
			// That said, there are costs (mostly management), so I should be careful with this variable.
			// (Particularly in terms of multithreading and synchronization issues.)
			game::g_my_game = std::make_shared<game::MyGame>(my_resources, 1, ground_terrain_file, air_terrain_file);
			// Show window
			ShowWindow(this->hwnd, n_cmd_show);
			UpdateWindow(this->hwnd);
			// Create update thread
			auto sync_mutex = CreateMutex(nullptr, false, TEXT("can_execute"));
			if (!sync_mutex) {
				winapi::handleWindowsError(L"Can execute mutex creation");
			}
			// (Note: update and draw events necessary to ensure that the game can
			// both update and draw regularly.)
			auto update_event = CreateEvent(nullptr, true, true, TEXT("can_update"));
			if (!update_event) {
				winapi::handleWindowsError(L"Can update event creation");
			}
			auto draw_event = CreateEvent(nullptr, true, true, TEXT("can_draw"));
			if (!draw_event) {
				CloseHandle(update_event);
				winapi::handleWindowsError(L"Can draw event creation");
			}
			auto update_thread_init_event = CreateEvent(nullptr, false, false, TEXT("update_thread_ready"));
			if (!update_thread_init_event) {
				CloseHandle(update_event);
				CloseHandle(draw_event);
				winapi::handleWindowsError(L"Update thread ready creation");
			}
			HANDLE update_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, winapi::update_thread_init,
				nullptr, 0, nullptr));
			if (!update_thread || update_thread == INVALID_HANDLE_VALUE) {
				CloseHandle(update_event);
				CloseHandle(draw_event);
				CloseHandle(update_thread_init_event);
				winapi::handleWindowsError(L"Update thread creation");
			}
			WaitForSingleObject(update_thread_init_event, INFINITE);
			my_renderer->updateHealthOption(hwnd, game::g_my_game->getHealthBuyCost());
			my_renderer->createTowerMenu(hwnd, game::g_my_game->getAllTowerTypes());
			HANDLE terrain_editor_thread {nullptr};
			// Message Loop
#pragma warning(push)
#pragma warning(disable: 26494) // Code Analysis: type.5 --> Always initialize.
			MSG msg;
#pragma warning(pop)
			bool keep_looping = true;
			while (keep_looping) {
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					switch (msg.message) {
					case WM_COMMAND:
					{
						switch (msg.wParam) {
						case ID_MM_FILE_NEW_GAME:
						{
							PostThreadMessage(GetThreadId(update_thread), msg.message, msg.wParam, msg.lParam);
							break;
						}
						case ID_MM_FILE_SAVE_GAME:
						{
							PostThreadMessage(GetThreadId(update_thread), msg.message, msg.wParam, msg.lParam);
							break;
						}
						case ID_MM_FILE_QUIT:
						{
							PostThreadMessage(GetThreadId(update_thread), msg.message, msg.wParam, msg.lParam);
							PostMessage(hwnd, WM_DESTROY, 0, 0);
							break;
						}
						case ID_MM_ACTIONS_NEXT_WAVE:
						{
							PostThreadMessage(GetThreadId(update_thread), msg.message, msg.wParam, msg.lParam);
							break;
						}
						case ID_MM_ACTIONS_TOGGLE_PAUSE:
						{
							game::g_my_game->togglePause();
							break;
						}
						case ID_MM_ACTIONS_BUY_HEALTH:
						{
							// TODO: Move the "buyHealth()" part into the update thread.
							game::g_my_game->buyHealth();
							my_renderer->updateHealthOption(hwnd, game::g_my_game->getHealthBuyCost());
							break;
						}
						case ID_MM_TOWERS_INFO:
						{
							// (Yes, walls are explicitly excluded heree.)
							if (game::g_my_game->getSelectedTower() >= 1
								&& static_cast<size_t>(game::g_my_game->getSelectedTower())
								< game::g_my_game->getAllTowerTypes().size()) {
								const auto pause_state = game::g_my_game->isPaused();
								if (!pause_state) {
									game::g_my_game->togglePause();
								}
								const TowerInfoDialog my_dialog {hwnd, this->h_instance,
									*game::g_my_game->getTowerType(game::g_my_game->getSelectedTower())};
								if (pause_state != game::g_my_game->isPaused()) {
									game::g_my_game->togglePause();
								}
							}
							break;
						}
						case ID_MM_DEVELOP_TERRAIN_EDITOR:
						{
							terrain_editor_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0,
								terrain_editor::terrain_editor_thread_init, static_cast<void*>(hwnd), 0, nullptr));
							// I do not care when the editor thread ends...
							if (terrain_editor_thread && terrain_editor_thread != INVALID_HANDLE_VALUE) {
								CloseHandle(terrain_editor_thread);
							}
							terrain_editor_thread = nullptr;
							break;
						}
						case ID_MM_DEVELOP_SHOW_TEST_PATHS:
						{
							game::g_my_game->toggleShowPaths();
							break;
						}
						default:
							break;
						}
						// Check if the tower menu thingy got selected
						if (msg.wParam >= ID_MM_TOWERS_NONE
							&& msg.wParam <= ID_MM_TOWERS_NONE + game::g_my_game->getAllTowerTypes().size()) {
							my_renderer->updateSelectedTower(hwnd, static_cast<int>(msg.wParam));
							game::g_my_game->selectTower(static_cast<int>(msg.wParam) - ID_MM_TOWERS_NONE - 1);
						}
						break;
					}
					case WM_KEYUP:
					{
						if (GetKeyState(VK_CONTROL) && HIWORD(GetAsyncKeyState(VK_CONTROL))) {
							switch (msg.wParam) {
							case 'N':
								PostThreadMessage(GetThreadId(update_thread), WM_COMMAND, ID_MM_FILE_NEW_GAME, 0);
								break;
							case 'S':
								PostThreadMessage(GetThreadId(update_thread), WM_COMMAND, ID_MM_FILE_SAVE_GAME, 0);
								break;
							case 'Q':
								// Good way to keep mistakes from happening from keypresses
								// That said, having a confirmation message every time is
								// flat out annoying.
								if (MessageBox(hwnd, L"Are you sure you want to quit? (Unsaved data will be lost.)",
									L"Tower defense - Quit?", MB_YESNO) == IDYES) {
									PostMessage(hwnd, WM_COMMAND, ID_MM_FILE_QUIT, 0);
								}
								break;
							default:
								break;
							}
						}
						switch (msg.wParam) {
						case 'W':
							PostThreadMessage(GetThreadId(update_thread), WM_COMMAND, ID_MM_ACTIONS_NEXT_WAVE, 0);
							break;
						case 'P':
							PostMessage(hwnd, WM_COMMAND, ID_MM_ACTIONS_TOGGLE_PAUSE, 0);
							break;
						}
						break;
					}
					case WM_LBUTTONDOWN:
					{
						// Obtain start coordinates
						auto gx = static_cast<int>(graphics::convertToGameX(GET_X_LPARAM(msg.lParam)));
						auto gy = static_cast<int>(graphics::convertToGameY(GET_Y_LPARAM(msg.lParam)));
						if (game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(gx, gy)) {
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
							if (game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(gx, gy)) {
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
						const auto end_sx = static_cast<float>(GET_X_LPARAM(msg.lParam));
						const auto end_sy = static_cast<float>(GET_Y_LPARAM(msg.lParam));
						const auto new_gx = static_cast<int>(graphics::convertToGameX(end_sx));
						const auto new_gy = static_cast<int>(graphics::convertToGameY(end_sy));
						this->end_gx = math::get_max(new_gx, this->start_gx);
						this->end_gy = math::get_max(new_gy, this->start_gy);
						this->start_gx = math::get_min(this->start_gx, new_gx);
						this->start_gy = math::get_min(this->start_gy, new_gy);
						if (game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(this->start_gx, this->start_gy)
							&& game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(this->end_gx, this->end_gy)) {
							if (game::g_my_game->getSelectedTower() == 0) {
								// Wall...
								WaitForSingleObject(sync_mutex, INFINITE);
								for (int gx = this->start_gx; gx <= this->end_gx; ++gx) {
									for (int gy = this->start_gy; gy <= this->end_gy; ++gy) {
										game::g_my_game->buyTower(gx, gy);
									}
								}
								ReleaseMutex(sync_mutex);
							}
							else {
								auto my_new_lparam = MAKELPARAM(this->end_gx, this->end_gy);
								PostThreadMessage(GetThreadId(update_thread), WM_COMMAND,
									ID_MM_TOWERS_BUY_TOWER, my_new_lparam);
							}
						}
						if (game::g_my_game->isInLevel()) {
							const bool pause_state = game::g_my_game->isPaused();
							if (!pause_state) {
								game::g_my_game->togglePause();
							}
							// And no wonder this crashed.... We do indeed have a race
							// condition (but not for long!)
							// Check if the user clicked on an enemy.
							WaitForSingleObject(sync_mutex, INFINITE);
							const auto& enemy_list = game::g_my_game->getEnemies();
							for (const auto& e : enemy_list) {
								if (e->checkHit(end_sx, end_sy)) {
									const winapi::EnemyInfoDialog my_dialog {hwnd, this->h_instance, e->getBaseType()};
									break;
								}
							}
							ReleaseMutex(sync_mutex);
							if (pause_state != game::g_my_game->isPaused()) {
								game::g_my_game->togglePause();
							}
						}
						// Reset coordinates
						this->start_gx = -1;
						this->start_gy = -1;
						this->end_gx = -1;
						this->end_gy = -1;
						break;
					}
					case WM_RBUTTONUP:
					{
						// Get coordinates
						const auto my_gx = static_cast<int>(graphics::convertToGameX(GET_X_LPARAM(msg.lParam)));
						const auto my_gy = static_cast<int>(graphics::convertToGameY(GET_Y_LPARAM(msg.lParam)));
						const auto my_new_lparam = MAKELPARAM(my_gx, my_gy);
						PostThreadMessage(GetThreadId(update_thread), WM_COMMAND, ID_MM_TOWERS_SELL_TOWER, my_new_lparam);
						break;
					}
					default:
						break;
					}
					if (msg.message == WM_QUIT) {
						PostThreadMessage(GetThreadId(update_thread), WM_DESTROY, 0, 0);
						keep_looping = false;
					}
				}
				else {
					// Render scene
					const HANDLE draw_object_handles[] = {
						draw_event, sync_mutex
					};
					WaitForMultipleObjects(2, draw_object_handles, true, INFINITE);
					HRESULT hr = my_renderer->render(game::g_my_game, this->start_gx, this->start_gy,
						this->end_gx, this->end_gy);
					if (hr == D2DERR_RECREATE_TARGET) {
						my_resources->discardDeviceResources();
						my_resources->createDeviceResources(this->hwnd);
					}
					ReleaseMutex(sync_mutex);
					// Sleep a little bit
					Sleep(0);
				}
			}
			CloseHandle(update_thread);
			CloseHandle(update_event);
			CloseHandle(draw_event);
			CloseHandle(sync_mutex);
		}
	}
}