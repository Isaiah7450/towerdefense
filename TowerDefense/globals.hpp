#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include <string>

namespace hoffman::isaiah {
	namespace winapi {
		// Global constants
		constexpr const auto main_class_name {L"my_game"};
		constexpr const auto main_window_name {L"Isaiah's tower defense game"};
		// Prototypes
		/// <summary>Attempts to provide a human-readable error message
		/// if an error occurs while trying to initialize the program.</summary>
		/// <param name="lpszFunction">The source of the error message.</param>
		[[noreturn]] void handleWindowsError(std::wstring lpszFunction);
	}
	namespace graphics {
		// Forward declarations
		class Renderer2D;
	}

	namespace graphics::DX {
		// Forward declarations
		class DeviceResources2D;
	}

	namespace graphics {
		// Interfaces
		/// <summary>Interface for objects that can be drawn.</summary>
		class IDrawable {
		public:
			IDrawable() = default;
			// Default destructor declared virtual
			virtual ~IDrawable() = default;
			// Rule of 5
			IDrawable(const IDrawable& rhs) = default;
			IDrawable(IDrawable&& rhs) = default;
			IDrawable& operator=(const IDrawable& rhs) = default;
			IDrawable& operator=(IDrawable&& rhs) = default;
			/// <summary>Implement this method for anything that can be drawn.</summary>
			virtual void draw(const Renderer2D& renderer) const noexcept = 0;
		};

		// Global constants and variables
		// Variables should be defined in main.cpp
		// The following 4 values define the % of the screen covered by margins
		constexpr const double margin_left = 0.025;
		constexpr const double margin_right = 0.025;
		constexpr const double margin_top = 0.050;
		constexpr const double margin_bottom = 0.033;
		/// <summary>The screen resolution width.</summary>
		extern int screen_width;
		/// <summary>The screen resolution height.</summary>
		extern int screen_height;
		/// <summary>The width of the game grid.</summary>
		extern int grid_width;
		/// <summary>The height of the game grid.</summary>
		extern int grid_height;
	}

	namespace pathfinding {
		// Global classes
		/// <summary>Enumeration of various heuristic strategies to estimate h during pathfinding.</summary>
		enum class HeuristicStrategies {
			Manhattan, Diagonal, Euclidean, Max_Dx_Dy
		};
	}

	namespace game {
		// To conserve CPU, the program only redraws the screen once a frame
		// where the following number dictates the number of graphical frames
		// in a second.
		constexpr const int graphics_framerate = 55;
		// This framerate controls how often the game state is updated
		// per second.
		constexpr const int logic_framerate = 120;
	}
}