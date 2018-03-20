#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include <string>

namespace hoffman::isaiah {
	namespace winapi {
		// Global constants
		constexpr const auto main_class_name {"my_game"};
		constexpr const auto main_window_name {"Isaiah's tower defense game"};
		// Prototypes
		/// <summary>Attempts to provide a human-readable error message
		/// if an error occurs while trying to initialize the program.</summary>
		/// <param name="lpszFunction">The source of the error message.</param>
		[[noreturn]] void handleWindowsError(std::wstring lpszFunction);
	}
	namespace graphics {
		// Global constants and variables
		// Variables should be defined in main.cpp
		/// <summary>The screen resolution width.</summary>
		extern int screen_width;
		/// <summary>The screen resolution height.</summary>
		extern int screen_height;
		extern int grid_width;
		extern int grid_height;
	}
}