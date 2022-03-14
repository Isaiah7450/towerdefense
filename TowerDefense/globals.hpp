#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <array>
#include <string>

namespace hoffman_isaiah {
	namespace winapi {
		// Interfaces/enums:
		/// <summary>Interface for dialog boxes.</summary>
		class IDialog {
		public:
			IDialog() noexcept = default;
			// Rule of 5 and virtual dtor:
			virtual ~IDialog() noexcept = default;
			IDialog(const IDialog&) = default;
			IDialog(IDialog&&) = default;
			IDialog& operator=(const IDialog&) = default;
			IDialog& operator=(IDialog&&) = default;
		protected:
			/// <summary>Initializes the dialog.</summary>
			/// <param name="hwnd">Handle to the dialog box window.</param>
			virtual void initDialog(HWND hwnd) = 0;
		};
		// Prototypes
		/// <summary>Attempts to provide a human-readable error message
		/// if an error occurs while trying to initialize the program.</summary>
		/// <param name="lpszFunction">The source of the error message.</param>
		[[noreturn]] void handleWindowsError(std::wstring lpszFunction);

		// For use with COM pointers
		template <typename T>
		struct ReleaseCOM {
			void operator()(T* pT) {
				pT->Release();
			}
		};
	}
	namespace graphics {
		// Forward declarations
		class Renderer2D;
	}

	namespace graphics::DX {
		// Forward declarations
		class DeviceResources2D;
	}

	namespace game {
		// Forward declarations
		class GameMap;
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

		// Get margin sizes
		constexpr double getLeftMarginSize() noexcept {
			return graphics::screen_width * graphics::margin_left;
		}
		constexpr double getRightMarginSize() noexcept {
			return graphics::screen_width * graphics::margin_right;
		}
		constexpr double getTopMarginSize() noexcept {
			return graphics::screen_height * graphics::margin_top;
		}
		constexpr double getBottomMarginSize() noexcept {
			return graphics::screen_height * graphics::margin_bottom;
		}
	}

	namespace pathfinding {
		// Global classes
		/// <summary>Enumeration of various heuristic strategies to estimate h during pathfinding.</summary>
		enum class HeuristicStrategies {
			Manhattan, Diagonal, Euclidean, Max_Dx_Dy, Sentinel_DO_NOT_USE
		};
	}

	inline std::wstring operator*(pathfinding::HeuristicStrategies strat) {
		constexpr const std::array<const wchar_t*,
			static_cast<int>(pathfinding::HeuristicStrategies::Sentinel_DO_NOT_USE)> my_strs = {
			L"Manhattan", L"Diagonal", L"Euclidean", L"Max DX DY"
		};
		return my_strs.at(static_cast<int>(strat));
	}

	namespace game {
		// To conserve CPU, the program only redraws the screen once a frame
		// where the following number dictates the number of graphical frames
		// in a second.
		constexpr const int graphics_framerate = 55;
		// This framerate controls how often the game state is updated
		// per second.
		constexpr const int logic_framerate = 120;
#if defined(DEBUG) || defined(_DEBUG)
#ifdef _M_X64
#define MY_PROJECT_FORMAT L"dx64aay"
#else
#define MY_PROJECT_FORMAT L"dx86azk"
#endif // _M_X64
#else
#ifdef _M_X64
#define MY_PROJECT_FORMAT L"rkx64amn"
#else
#define MY_PROJECT_FORMAT L"rkx86all"
#endif // _M_X64
#endif // defined(DEBUG) || defined(_DEBUG)
		// The name of the default save file.
		constexpr const wchar_t* default_save_file_name = MY_PROJECT_FORMAT L"_xe8t6418hdefj.dat";
		// The name of the global save data file. (Data relevant across saves is stored here.)
		constexpr const wchar_t* global_save_file_name = MY_PROJECT_FORMAT L"_xe8t3148hdefj.dat";
	}
#undef MY_PROJECT_FORMAT
}