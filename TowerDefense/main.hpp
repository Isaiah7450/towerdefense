#pragma once
// File Author: Isaiah Hoffman
// File Created: March 19, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <string>
#include <utility>

namespace hoffman::isaiah {
	namespace game {
		class MyGame;
	}

	namespace winapi {
		unsigned __stdcall update_thread_init(void* data);

		/// <summary>Represents the primary application window.</summary>
		class MainWindow {
		public:
			/// <summary>Creates a new main window.</summary>
			/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
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
			/// <param name="start_time">The starting time as determined by QueryPerformanceCounter.</param>
			/// <returns>The current time as well as the number of microseconds that have passed since
			/// the last call.</returns>
			static std::pair<LARGE_INTEGER, LARGE_INTEGER> getElapsedTime(LARGE_INTEGER start_time) noexcept {
				// Get current time
				LARGE_INTEGER current_time;
				QueryPerformanceCounter(&current_time);
				LARGE_INTEGER elapsed_microseconds;
				// Get elapsed time
				elapsed_microseconds.QuadPart = current_time.QuadPart - start_time.QuadPart;
				// Convert to microsecond
				elapsed_microseconds.QuadPart *= 1000000;
 				elapsed_microseconds.QuadPart /= MainWindow::qpc_frequency.QuadPart;
				return {current_time, elapsed_microseconds};
			}
		private:
			/// <summary>The handle to the application instance.</summary>
			HINSTANCE h_instance;
			/// <summary>The handle to the window.</summary>
			HWND hwnd {nullptr};
			/// <summary>The handle to the window's menu.</summary>
			HMENU h_menu {nullptr};
			/// <summary>The window's current dimensions.</summary>
			RECT rc {};
			/// <summary>Stores the starting game x-position of the mouse.</summary>
			int start_gx {-1};
			/// <summary>Stores the starting game y-position of the mouse.</summary>
			int start_gy {-1};
			/// <summary>Stores the ending game x-position of the mouse.</summary>
			int end_gx {-1};
			/// <summary>Stores the ending game y-position of the mouse.</summary>
			int end_gy {-1};
			// The class name
			static constexpr const wchar_t* class_name {L"my_game"};
			static constexpr const wchar_t* window_name {L"A Shaping War: Isaiah's tower defense game"};
			/// <summary>Stores the frequency of the high performance counter.</summary>
			static LARGE_INTEGER qpc_frequency;
		};
	}
}