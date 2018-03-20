#pragma once
// File Author: Isaiah Hoffman
// File Created: March 19, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <string>

namespace hoffman::isaiah {
	namespace winapi {
		/// <summary>Represents the primary application window.</summary>
		class MainWindow {
		public:
			/// <summary>Creates a new main window.</summary>
			/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
			/// <param name="nCmdShow">The nCmdShow parameter given by the WinMain function.</param>
			MainWindow(HINSTANCE h_inst);
			// Window's procedure
			static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			// Getters
			HWND getHWND() const noexcept {
				return this->hwnd;
			}
			/// <returns>Return's the name of the window.</returns>
			constexpr static const wchar_t* getWindowName() noexcept {
				return MainWindow::window_name;
			}
			/// <summary>Runs the program.</summary>
			/// <param name="n_cmd_show">Parameter inherited from WinMain that
			/// determines the initial state of the window.</param>
			void run(int n_cmd_show);
		private:
			/// <summary>The handle to the application instance.</summary>
			HINSTANCE h_instance;
			/// <summary>The handle to the window.</summary>
			HWND hwnd {nullptr};
			/// <summary>The handle to the window's menu.</summary>
			HMENU h_menu {nullptr};
			/// <summary>The window's current dimensions.</summary>
			RECT rc {};
			// The class name
			static constexpr const wchar_t* class_name {L"my_game"};
			static constexpr const wchar_t* window_name {L"Isaiah's tower defense game"};
		};
	}
}