// File Author: Isaiah Hoffman
// File Created: March 9, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <stdexcept>
#include <string>
#include <d3d11.h>
#include "./globals.hpp"
#include "./graphics/graphics_DX.hpp"

// Should I use the pragma or use project properties?
#pragma comment(lib, "d3d11.lib")

using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace graphics {
		int screen_width {800};
		int screen_height {600};
		int grid_width {40};
		int grid_height {40};
	}

	namespace winapi {
		LRESULT CALLBACK winmain_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		// TODO: Move this elsewhere later
		LRESULT CALLBACK winmain_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			switch (msg) {
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			default:
				break;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
}

namespace ih = hoffman::isaiah;

// WinMain function
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	try {
		// Register Window Class
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ih::winapi::winmain_proc;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		// wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wc.lpszClassName = ih::winapi::main_class_name;
		if (RegisterClassEx(&wc) == 0) {
			throw std::runtime_error {"Error Code #"s + std::to_string(GetLastError()) + " occurred "s
				"while trying to register window class."s};
		}
		// Determine window size
		RECT wrect = {0, 0, ih::graphics::screen_width, ih::graphics::screen_height};
		constexpr const auto my_dwStyle {WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_SIZEBOX};
		constexpr const auto my_dwExStyle {0};
		AdjustWindowRectEx(&wrect, my_dwStyle, FALSE, my_dwExStyle);
		// Create Window
		HWND hwnd = CreateWindowEx(my_dwExStyle, ih::winapi::main_class_name, ih::winapi::main_window_name, my_dwStyle,
			CW_USEDEFAULT, CW_USEDEFAULT, wrect.right - wrect.left, wrect.bottom - wrect.top,
			nullptr, nullptr, hInstance, nullptr);
		if (!hwnd) {
			throw std::runtime_error {"Error Code #"s + std::to_string(GetLastError()) + " occurred "s
				"while trying to create the window."s};
		}
		// Display Window
		ShowWindow(hwnd, nCmdShow);
		// Initialize Direct3D
		ih::graphics::DX::initialize_d3d(hwnd);
		// Message Loop -- Use peekMessage to increase framerate at expense of increased CPU usage
		MSG msg;
		BOOL is_done = FALSE;
		while (!is_done) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT || msg.message == WM_DESTROY) {
					// End loop
					is_done = true;
				}
				/*
				if (hRet == -1) {
				throw std::runtime_error {"Error Code #"s + std::to_string(GetLastError()) + " occurred "s
				"during the program's message loop."s};
				}
				*/
			}
			else {
				// Use time to handle game logic and rendering
				ih::graphics::render_frame();
				// If there is nothing to do, simply sleep to lower CPU usage
				Sleep(1);
			}
		}
	}
	catch (const std::exception& e) {
		// (Also makes a good fallback for uncaught exceptions that occur.)
		MessageBox(nullptr, e.what(), ih::winapi::main_window_name, MB_OK | MB_ICONEXCLAMATION);
		// (Yes, this is safe even if D3D was never initialized.)
		ih::graphics::DX::clean_up_d3d();
		return 1;
	}
	// Clean up after Direct3D
	ih::graphics::DX::clean_up_d3d();
	return 0;
}