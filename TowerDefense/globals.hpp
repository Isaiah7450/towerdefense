#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018

namespace hoffman::isaiah {
	namespace winapi {
		// Global constants
		constexpr const auto main_class_name {"my_game"};
		constexpr const auto main_window_name {"Isaiah's tower defense game"};
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