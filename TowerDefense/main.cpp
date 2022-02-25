// File Author: Isaiah Hoffman
// File Created: March 9, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <Windowsx.h>
#include "./resource.h"
#include <commctrl.h>
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
#include "./graphics/other_dialogs.hpp"
#include "./pathfinding/grid.hpp"
#include "./game/enemy.hpp"
#include "./game/enemy_type.hpp"
#include "./game/my_game.hpp"
#include "./game/tower.hpp"
#include "./terrain/editor.hpp"

// Common controls
#pragma comment(lib, "comctl32.lib")

using namespace std::literals::string_literals;
namespace ih = hoffman_isaiah;
// WinMain function
#pragma warning(push)
#pragma warning(disable: 26461) // C26461 con.3: Parameter can be marked as a pointer to constant.
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
#pragma warning(pop)
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	INITCOMMONCONTROLSEX icc {};
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS;
	InitCommonControlsEx(&icc);
	// Initialize window.
	auto my_window = ih::winapi::MainWindow {hInstance};
	my_window.run(nCmdShow);
	return 0;
}

namespace hoffman_isaiah {
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
				// Initialize the game's state
				const std::shared_ptr<game::MyGame> my_game = game::g_my_game;
				// Force creation of message queue
				MSG msg;
				PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
				// Message Loop
				bool keep_running = true;
				while (keep_running) {
					const BOOL ret_value = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
					if (ret_value > 0) {
						switch (msg.message) {
						case WM_COMMAND:
						{
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
				}
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
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cst.
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
				| FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&lpMsgBuf), 0,
				nullptr);
			}
			// Display the error message and exit the process
			lpDisplayBuf = LocalAlloc(LMEM_ZEROINIT,
				lstrlen(static_cast<LPWSTR>(lpMsgBuf)) + lpszFunction.size() + 120 * sizeof(TCHAR));
			if (!lpDisplayBuf) {
				MessageBox(nullptr, L"Critical Error: Out of memory!", TEXT("Error"), MB_OK);
				ExitProcess(dw);
			}
			StringCchPrintf(static_cast<LPTSTR>(lpDisplayBuf), LocalSize(lpDisplayBuf) / sizeof(TCHAR),
				TEXT("%s failed with error %d: %s"), lpszFunction.c_str(), static_cast<int>(dw), static_cast<LPWSTR>(lpMsgBuf));
			MessageBox(nullptr, static_cast<LPWSTR>(lpDisplayBuf), TEXT("Error"), MB_OK);
			LocalFree(lpMsgBuf);
			LocalFree(lpDisplayBuf);
			ExitProcess(dw);
		}

		MainWindow::MainWindow(HINSTANCE h_inst) :
			h_instance {h_inst} {
			// Register window class
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			WNDCLASSEX wnd_class {
				sizeof(WNDCLASSEX), CS_DBLCLKS, MainWindow::windowProc, 0, 0, this->h_instance, nullptr,
				LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_BACKGROUND + 1),
				nullptr, MainWindow::class_name, nullptr
			};
			if (!RegisterClassEx(&wnd_class)) {
				winapi::handleWindowsError(L"Registration of window class");
			}
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
			// Store in global variable --> It's the only way!
			// Actually, it isn't, but it's the most intuitive (and least overhead involved) way to do it.
			// That said, there are costs (mostly management), so I should be careful with this variable.
			// (Particularly in terms of multithreading and synchronization issues.)
			game::g_my_game = std::make_shared<game::MyGame>(my_resources);
			// Show window
			ShowWindow(this->hwnd, n_cmd_show);
			UpdateWindow(this->hwnd);
			// Disable loading custom maps for now.
			winapi::disableMenuItem(hwnd, id_mm_file_offset, ID_MM_FILE_START_CUSTOM_GAME);
#if !defined(DEBUG) && !defined(_DEBUG)
			winapi::disableMenuItem(hwnd, id_mm_develop_offset, ID_MM_DEVELOP_SHOW_TEST_PATHS);
#endif
#pragma warning(push)
#pragma warning(disable: 26490) // C26490 => Do not use reinterpret_cast.
			/*
			* @TODO: Transport update thread initialization code here.
			HANDLE update_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, winapi::update_thread_init,
				nullptr, 0, nullptr));
#pragma warning(pop)
			*/
			// Will need to clean up the code one way or another later,
			// but this is convenient for now.
			auto* my_game = game::g_my_game.get();
			my_game->load_config_data();
			my_game->init_enemy_types();
			my_game->init_shot_types();
			my_game->init_tower_types();
			my_game->load_tower_upgrades_data();
			my_game->load_global_level_data();
			my_game->load_global_misc_data();
			// Load save data.
			std::wifstream default_save_file {game::g_my_game->getUserDataPath() + game::default_save_file_name};
			if (!default_save_file.bad() && !default_save_file.fail()) {
				try {
					game::g_my_game->loadGame(default_save_file);
				}
				catch (...) {
					MessageBox(nullptr, L"Error: Corrupted or old save file detected. Save files made in"
						L" version 3.3.1 or earlier are no longer supported.", L"Corrupted Save",
						MB_OK | MB_ICONERROR);
					// Reset state and save over the corrupted file...
					const std::wstring map_name = game::g_my_game->getDefaultMapName(ID_CHALLENGE_LEVEL_NORMAL);
					game::g_my_game->resetState(ID_CHALLENGE_LEVEL_NORMAL - ID_CHALLENGE_LEVEL_EASY, map_name);
					std::wofstream save_file {game::g_my_game->getUserDataPath() + game::default_save_file_name};
					if (!save_file.bad() && !save_file.fail()) {
						game::g_my_game->saveGame(save_file);
					}
				}
			}
			else {
				// Do difficulty selection.
				const auto my_clevel_dialog = winapi::ChallengeLevelDialog {hwnd, this->h_instance};
				const std::wstring map_name = game::g_my_game->getDefaultMapName(my_clevel_dialog.getChallengeLevel() > IDCANCEL
					? my_clevel_dialog.getChallengeLevel() : ID_CHALLENGE_LEVEL_NORMAL);
				game::g_my_game->resetState(my_clevel_dialog.getChallengeLevel() - ID_CHALLENGE_LEVEL_EASY, map_name);
			}
			my_renderer->updateHealthOption(hwnd, game::g_my_game->getHealthBuyCost());
			my_renderer->updateSpeedOption(hwnd, game::g_my_game->getNextUpdateSpeed());
			my_renderer->createTowerMenu(hwnd, game::g_my_game->getAllTowerTypes());
			my_renderer->createShotMenu(hwnd, game::g_my_game->getAllShotTypes());
			my_renderer->createEnemyMenu(hwnd, game::g_my_game->getAllEnemyTypes(),
				game::g_my_game->getSeenEnemies());
			try {
				game::g_my_game->loadGlobalData();
			}
			catch (const util::file::DataFileException&) {
				MessageBox(nullptr, L"Error: Corrupted global save file. Overwriting with default values.",
					L"Corrupted Save", MB_OK | MB_ICONERROR);
				game::g_my_game->saveGlobalData();
				try {
					game::g_my_game->loadGlobalData();
				}
				catch (...) {
					// Ignore.
				}
			}
			if (game::g_my_game->canStartCustomGames()) {
				winapi::enableMenuItem(hwnd, 0, ID_MM_FILE_START_CUSTOM_GAME);
			}
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
						handle_wm_command(game::g_my_game.get(), my_renderer.get(),
							msg.wParam, msg.lParam);
						if (msg.wParam >= ID_MM_TOWERS_MARK_TILES
							&& msg.wParam <= ID_MM_TOWERS_NONE + game::g_my_game->getAllTowerTypes().size()) {
							// Check if the tower menu thingy got selected
							my_renderer->updateSelectedTower(hwnd, static_cast<int>(msg.wParam));
							game::g_my_game->selectTower(static_cast<int>(msg.wParam) - ID_MM_TOWERS_NONE - 1);
						}
						else if (msg.wParam >= ID_MM_SHOTS_PLACEHOLDER
							&& msg.wParam < ID_MM_SHOTS_PLACEHOLDER + game::g_my_game->getAllShotTypes().size()) {
							// Show info about a selected shot.
							const auto selected_shot = msg.wParam - ID_MM_SHOTS_PLACEHOLDER;
							const auto pause_state = game::g_my_game->isPaused();
							if (!pause_state) {
								game::g_my_game->togglePause();
							}
							const game::ShotBaseType* my_shot {nullptr};
							unsigned int i = 0;
							for (const auto& stype : game::g_my_game->getAllShotTypes()) {
								if (i == selected_shot) {
									my_shot = stype.second.get();
									break;
								}
								++i;
							}
							if (my_shot) {
								const ShotBaseInfoDialog my_dialog {hwnd, this->h_instance,
									*my_shot};
								if (pause_state != game::g_my_game->isPaused()) {
									game::g_my_game->togglePause();
								}
							}
						}
						else if (msg.wParam >= ID_MM_ENEMIES_PLACEHOLDER
							&& msg.wParam < ID_MM_ENEMIES_PLACEHOLDER + game::g_my_game->getAllEnemyTypes().size()) {
							// Show info about a selected enemy.
							const int selected_enemy = static_cast<int>(msg.wParam - ID_MM_ENEMIES_PLACEHOLDER);
							const auto pause_state = game::g_my_game->isPaused();
							if (!pause_state) {
								game::g_my_game->togglePause();
							}
							const game::EnemyType* my_enemy {game::g_my_game->getEnemyType(selected_enemy)};
							if (game::g_my_game->getSeenEnemies().at(my_enemy->getName())) {
								const EnemyInfoDialog my_dialog {hwnd, this->h_instance,
									*my_enemy};
							}
							if (pause_state != game::g_my_game->isPaused()) {
								game::g_my_game->togglePause();
							}
						}
						break;
					}
					case WM_KEYUP:
					{
						if (GetKeyState(VK_CONTROL) && HIWORD(GetAsyncKeyState(VK_CONTROL))) {
							switch (msg.wParam) {
							case 'N':
								PostMessage(this->hwnd, WM_COMMAND, ID_MM_FILE_NEW_GAME, 0);
								break;
							case 'S':
								PostMessage(this->hwnd, WM_COMMAND, ID_MM_FILE_SAVE_GAME, 0);
								break;
							case 'Q':
								// Good way to keep mistakes from happening from keypresses
								// That said, having a confirmation message every time is
								// flat out annoying.
								if (MessageBox(hwnd, L"Are you sure you want to quit?",
									L"Tower defense - Quit?", MB_YESNO) == IDYES) {
									PostMessage(this->hwnd, WM_COMMAND, ID_MM_FILE_QUIT, 0);
								}
								break;
							default:
								break;
							}
						}
						switch (msg.wParam) {
						case 'W':
							PostMessage(this->hwnd, WM_COMMAND, ID_MM_ACTIONS_NEXT_WAVE, 0);
							break;
						case 'P':
							PostMessage(this->hwnd, WM_COMMAND, ID_MM_ACTIONS_TOGGLE_PAUSE, 0);
							break;
						case VK_OEM_PLUS:
							PostMessage(this->hwnd, WM_COMMAND, ID_MM_ACTIONS_CHANGE_SPEED, 0);
							break;
						default:
							break;
						}
						break;
					}
					case WM_LBUTTONDBLCLK:
					{
						// Display info about placed towers.
						if (!game::g_my_game->isInLevel()) {
							const auto gx = static_cast<int>(game::g_my_game->getMap().convertToGameX(GET_X_LPARAM(msg.lParam)));
							const auto gy = static_cast<int>(game::g_my_game->getMap().convertToGameY(GET_Y_LPARAM(msg.lParam)));
							const bool pause_state = game::g_my_game->isPaused();
							if (!pause_state) {
								game::g_my_game->togglePause();
							}
							for (auto& t : game::g_my_game->getTowers()) {
								if (static_cast<int>(t->getGameX()) == gx
									&& static_cast<int>(t->getGameY()) == gy) {
									const TowerPlacedInfoDialog my_dialog {hwnd, this->h_instance, *t};
									break;
								}
							}
							if (pause_state != game::g_my_game->isPaused()) {
								game::g_my_game->togglePause();
							}
						}
						break;
					}
					case WM_LBUTTONDOWN:
					case WM_RBUTTONDOWN:
					{
						// Obtain start coordinates
						const auto gx = static_cast<int>(game::g_my_game->getMap().convertToGameX(GET_X_LPARAM(msg.lParam)));
						const auto gy = static_cast<int>(game::g_my_game->getMap().convertToGameY(GET_Y_LPARAM(msg.lParam)));
						if (game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(gx, gy)) {
							this->start_gx = gx;
							this->start_gy = gy;
						}
						break;
					}
					case WM_MOUSEMOVE:
					{
						// Update end coordinates
						const auto gx = static_cast<int>(game::g_my_game->getMap().convertToGameX(GET_X_LPARAM(msg.lParam)));
						const auto gy = static_cast<int>(game::g_my_game->getMap().convertToGameY(GET_Y_LPARAM(msg.lParam)));
						if (game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(gx, gy)) {
							this->end_gx = gx;
							this->end_gy = gy;
						}
						else if (msg.wParam == MK_RBUTTON) {
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
						const auto new_gx = static_cast<int>(game::g_my_game->getMap().convertToGameX(end_sx));
						const auto new_gy = static_cast<int>(game::g_my_game->getMap().convertToGameY(end_sy));
						this->end_gx = math::get_max(new_gx, this->start_gx);
						this->end_gy = math::get_max(new_gy, this->start_gy);
						this->start_gx = math::get_min(this->start_gx, new_gx);
						this->start_gy = math::get_min(this->start_gy, new_gy);
						if (game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(this->start_gx, this->start_gy)
							&& game::g_my_game->getMap().getTerrainGraph(false).verifyCoordinates(this->end_gx, this->end_gy)) {
							if (game::g_my_game->getSelectedTower() == ID_MM_TOWERS_MARK_TILES - ID_MM_TOWERS_NONE - 1) {
								// Mark tiles
								for (int gx = this->start_gx; gx <= this->end_gx; ++gx) {
									for (int gy = this->start_gy; gy <= this->end_gy; ++gy) {
										game::g_my_game->getMap().getHighlightGraph().getNode(gx, gy).setBlockage(true);
									}
								}
							}
							else if (game::g_my_game->getSelectedTower() == ID_MM_TOWERS_UNMARK_TILES - ID_MM_TOWERS_NONE - 1) {
								// Unmark tiles
								for (int gx = this->start_gx; gx <= this->end_gx; ++gx) {
									for (int gy = this->start_gy; gy <= this->end_gy; ++gy) {
										game::g_my_game->getMap().getHighlightGraph().getNode(gx, gy).setBlockage(false);
									}
								}
							}
							else if (game::g_my_game->getSelectedTower() == ID_MM_TOWERS_NONE - ID_MM_TOWERS_NONE - 1) {
								// Invert coverage showing.
								for (int gx = this->start_gx; gx <= this->end_gx; ++gx) {
									for (int gy = this->start_gy; gy <= this->end_gy; ++gy) {
										const auto my_new_lparam = MAKELPARAM(gx, gy);
										handle_update_wm_command(my_game,
											ID_MM_TOWERS_BUY_TOWER, my_new_lparam);
									}
								}
							}
							else if (game::g_my_game->getSelectedTower() == 0) {
								// Wall...
								for (int gx = this->start_gx; gx <= this->end_gx; ++gx) {
									for (int gy = this->start_gy; gy <= this->end_gy; ++gy) {
										game::g_my_game->buyTower(gx, gy);
									}
								}
							}
							else {
								const auto my_new_lparam = MAKELPARAM(this->end_gx, this->end_gy);
								handle_update_wm_command(my_game, ID_MM_TOWERS_BUY_TOWER, my_new_lparam);
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
							const auto& enemy_list = game::g_my_game->getEnemies();
							for (const auto& e : enemy_list) {
								if (e->checkHit(end_sx, end_sy)) {
									const winapi::EnemyInfoDialog my_dialog {hwnd, this->h_instance, e->getBaseType()};
									break;
								}
							}
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
						const auto my_gx = static_cast<int>(game::g_my_game->getMap().convertToGameX(GET_X_LPARAM(msg.lParam)));
						const auto my_gy = static_cast<int>(game::g_my_game->getMap().convertToGameY(GET_Y_LPARAM(msg.lParam)));
						if (my_gx == start_gx && my_gy == start_gy) {
							const auto my_new_lparam = MAKELPARAM(my_gx, my_gy);
							handle_update_wm_command(my_game, ID_MM_TOWERS_SELL_TOWER, my_new_lparam);
						}
						// Reset coordinates
						this->start_gx = -1;
						this->start_gy = -1;
						this->end_gx = -1;
						this->end_gy = -1;
						break;
					}
					default:
						break;
					}
					if (msg.message == WM_QUIT) {
						keep_looping = false;
					}
				}
				else {
					// Render scene
					const HRESULT hr = my_renderer->render(game::g_my_game, this->start_gx, this->start_gy,
						this->end_gx, this->end_gy);
					if (!game::g_my_game->isInLevel()) {
						my_renderer->createEnemyMenu(hwnd, game::g_my_game->getAllEnemyTypes(),
							game::g_my_game->getSeenEnemies());
						if (game::g_my_game->canStartCustomGames()) {
							winapi::enableMenuItem(hwnd, 0, ID_MM_FILE_START_CUSTOM_GAME);
						}
					}
					if (hr == D2DERR_RECREATE_TARGET) {
						my_resources->discardDeviceResources();
						my_resources->createDeviceResources(this->hwnd);
					}
					// Update scene.
					// Check time before updating...
					static LARGE_INTEGER last_update_time = LARGE_INTEGER {0};
					if (last_update_time.QuadPart == 0) {
						QueryPerformanceCounter(&last_update_time);
					}
					const auto my_times = winapi::MainWindow::getElapsedTime(last_update_time);
					if (my_times.second.QuadPart >= math::getMicrosecondsInSecond() / game::logic_framerate) {
						last_update_time = my_times.first;
						my_game->update();
					}
				}
			}
		}

		void MainWindow::handle_wm_command(game::MyGame* my_game,
			graphics::Renderer2D* my_renderer, WPARAM wparam, LPARAM lparam) {
			UNREFERENCED_PARAMETER(lparam);
			switch (wparam) {
			case ID_MM_FILE_NEW_GAME:
			{
				if (!lparam) {
					const auto my_clevel_dialog = winapi::ChallengeLevelDialog {hwnd, this->h_instance};
					[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
					handle_update_wm_command(my_game, wparam,
						reinterpret_cast<LPARAM>(&my_clevel_dialog));
					}
				}
				break;
			}
			case ID_MM_FILE_START_CUSTOM_GAME:
			{
				const auto my_dialog = winapi::StartCustomGameDialog {hwnd, this->h_instance};
				if (my_dialog.getChallengeLevel() != IDCANCEL) {
					my_game->resetState(my_dialog.getChallengeLevel() - ID_CHALLENGE_LEVEL_EASY, my_dialog.getMapName(), true);
				}
				break;
			}
			case ID_MM_FILE_SAVE_GAME:
			{
				handle_update_wm_command(my_game, wparam, lparam);
				break;
			}
			case ID_MM_FILE_QUIT:
			{
				handle_update_wm_command(my_game, ID_MM_FILE_SAVE_GAME, 0);
				handle_update_wm_command(my_game, wparam, lparam);
				PostMessage(hwnd, WM_DESTROY, 0, 0);
				break;
			}
			case ID_MM_ACTIONS_NEXT_WAVE:
			{
				if (my_game->canStartCustomGames()) {
					winapi::enableMenuItem(hwnd, 0, ID_MM_FILE_START_CUSTOM_GAME);
				}
				handle_update_wm_command(my_game, wparam, lparam);
				break;
			}
			case ID_MM_ACTIONS_TOGGLE_PAUSE:
			{
				my_game->togglePause();
				break;
			}
			case ID_MM_ACTIONS_BUY_HEALTH:
			{
				my_game->buyHealth();
				my_renderer->updateHealthOption(hwnd, my_game->getHealthBuyCost());
				break;
			}
			case ID_MM_ACTIONS_CHANGE_SPEED:
			{
				my_game->changeUpdateSpeed();
				my_renderer->updateSpeedOption(hwnd, my_game->getNextUpdateSpeed());
				break;
			}
			case ID_MM_ACTIONS_TOGGLE_ALL_RADII:
			{
				handle_update_wm_command(my_game, wparam, lparam);
				break;
			}
			case ID_MM_ACTIONS_VIEW_GLOBAL_STATS:
			{
				const winapi::GlobalStatsDialog my_dialog {this->getHWND(), this->h_instance, *my_game};
				break;
			}
			case ID_MM_TOWERS_INFO:
			{
				// (Yes, walls are explicitly excluded heree.)
				if (my_game->getSelectedTower() >= 1
					&& static_cast<size_t>(my_game->getSelectedTower())
					< my_game->getAllTowerTypes().size()) {
					const auto pause_state = my_game->isPaused();
					if (!pause_state) {
						my_game->togglePause();
					}
					const TowerInfoDialog my_dialog {hwnd, this->h_instance,
						*my_game->getTowerType(my_game->getSelectedTower())};
					if (pause_state != my_game->isPaused()) {
						my_game->togglePause();
					}
				}
				break;
			}
			case ID_MM_DEVELOP_TERRAIN_EDITOR:
			{
				[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
				HANDLE terrain_editor_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0,
					terrain_editor::terrain_editor_thread_init, static_cast<void*>(hwnd), 0, nullptr));
				// I do not care when the editor thread ends...
				if (terrain_editor_thread && terrain_editor_thread != INVALID_HANDLE_VALUE) {
					CloseHandle(terrain_editor_thread);
				}
				terrain_editor_thread = nullptr;
				}
				break;
			}
			case ID_MM_DEVELOP_SHOW_TEST_PATHS:
			{
				my_game->toggleShowPaths();
				break;
			}
			default:
				break;
			}
		}

		void MainWindow::handle_update_wm_command(game::MyGame* my_game, WPARAM wparam,
			LPARAM lparam) {
			switch (wparam) {
			case ID_MM_FILE_NEW_GAME:
			{
				[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpet_cast.
				const auto& my_clevel_dialog = *reinterpret_cast<const ChallengeLevelDialog*>(lparam);
				if (my_clevel_dialog.getChallengeLevel() != IDCANCEL) {
					const auto new_clevel = my_clevel_dialog.getChallengeLevel() - ID_CHALLENGE_LEVEL_EASY;
					my_game->resetState(new_clevel, my_game->getDefaultMapName(my_clevel_dialog.getChallengeLevel()));
				}
				}
				break;
			}
			case ID_MM_FILE_SAVE_GAME:
			{
				std::wofstream save_file {game::g_my_game->getUserDataPath() + game::default_save_file_name};
				if (save_file.fail() || save_file.bad()) {
					MessageBox(nullptr, L"Could not save game.", L"Save failed!", MB_ICONEXCLAMATION | MB_OK);
				}
				else {
					my_game->saveGame(save_file);
				}
				break;
			}
			case ID_MM_ACTIONS_NEXT_WAVE:
				my_game->startWave();
				break;
			case ID_MM_ACTIONS_TOGGLE_ALL_RADII:
				my_game->toggleAllRadii();
				break;
			case ID_MM_TOWERS_BUY_TOWER:
			{
				const auto my_gx = static_cast<int>(GET_X_LPARAM(lparam));
				const auto my_gy = static_cast<int>(GET_Y_LPARAM(lparam));
				my_game->buyTower(my_gx, my_gy);
				break;
			}
			case ID_MM_TOWERS_SELL_TOWER:
			{
				const auto my_gx = static_cast<int>(GET_X_LPARAM(lparam));
				const auto my_gy = static_cast<int>(GET_Y_LPARAM(lparam));
				my_game->sellTower(my_gx, my_gy);
				break;
			}
			default:
				break;
			}
		}
	}
}