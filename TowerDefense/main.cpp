// File Author: Isaiah Hoffman
// File Created: March 9, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include <strsafe.h>
#include <fstream>
#include <iostream>
#include "./globals.hpp"
#include "./main.hpp"
#include "./graphics/graphics_DX.hpp"
#include "./graphics/graphics.hpp"
#include "./game/my_game.hpp"
#include "./pathfinding/grid.hpp"

using namespace std::literals::string_literals;
namespace ih = hoffman::isaiah;
// WinMain function
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	auto my_window = std::make_unique<ih::winapi::MainWindow>(hInstance);
	my_window->run(nCmdShow);
	return 0;
}

namespace hoffman::isaiah {
	namespace graphics {
		int screen_width {800};
		int screen_height {600};
		int grid_width {40};
		int grid_height {40};
	}

	namespace winapi {
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
			lpDisplayBuf = static_cast<LPVOID>(LocalAlloc(LMEM_ZEROINIT,
				lstrlen(static_cast<LPWSTR>(lpMsgBuf)) + lpszFunction.size() + 120 * sizeof(TCHAR)));
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

		MainWindow::MainWindow(HINSTANCE h_inst) :
			h_instance {h_inst} {
			// Register window class
			WNDCLASSEX wnd_class {
				sizeof(WNDCLASSEX), CS_DBLCLKS, MainWindow::windowProc, 0, 0, this->h_instance, nullptr,
				LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)),
				nullptr, MainWindow::class_name, nullptr
			};
			if (!RegisterClassEx(&wnd_class)) {
				winapi::handleWindowsError(L"Registration of window class");
			}
			// No menu (initialized to nullptr in class definition)
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
			auto my_game = std::make_shared<game::MyGame>();
			// Show window
			ShowWindow(this->hwnd, n_cmd_show);
			UpdateWindow(this->hwnd);
			// Test input/output of grids
			std::wofstream output_file {L"test.txt"};
			ih::pathfinding::Grid my_grid {0, 0, 2, 2};
			output_file << my_grid;
			output_file.close();
			std::wifstream input_file {L"test.txt"};
			input_file >> my_grid;
			// Message Loop
			MSG msg;
			bool keep_looping = true;
			while (keep_looping) {
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					if (msg.message == WM_QUIT) {
						keep_looping = false;
					}
				}
				else {
					// Update state
					my_game->update();
					// Render scene
					HRESULT hr = my_renderer->render(*my_game);
					if (hr == D2DERR_RECREATE_TARGET) {
						my_resources->discardDeviceResources();
						my_resources->createDeviceResources(this->hwnd);
					}
					// Sleep to save on CPU usage
					Sleep(1);
				}
			}
		}
	}
}