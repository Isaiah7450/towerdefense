#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <memory>
#include "./../globals.hpp"
#include "./graphics_DX.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declaration
		class MyGame;
	}
	namespace terrain_editor {
		// Forward declaration
		class TerrainEditor;
	}

	namespace graphics {
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
		// Measurements
		template <typename T = double>
		constexpr T getGameSquareWidth() noexcept {
			return static_cast<T>(graphics::screen_width - (graphics::getLeftMarginSize() + graphics::getRightMarginSize()))
				/ static_cast<T>(graphics::grid_width);
		}
		template <typename T = double>
		constexpr T getGameSquareHeight() noexcept {
			return static_cast<T>(graphics::screen_height - (graphics::getTopMarginSize() + graphics::getBottomMarginSize()))
				/ static_cast<T>(graphics::grid_height);
		}
		// Coordinate conversion
		constexpr double convertToGameX(double sx) noexcept {
			return (sx - graphics::getLeftMarginSize()) / graphics::getGameSquareWidth();
		}
		constexpr double convertToScreenX(double gx) noexcept {
			return (gx * graphics::getGameSquareWidth()) + graphics::getLeftMarginSize();
		}
		constexpr double convertToGameY(double sy) noexcept {
			return (sy - graphics::getTopMarginSize()) / graphics::getGameSquareHeight();
		}
		constexpr double convertToScreenY(double gy) noexcept {
			return (gy * graphics::getGameSquareHeight()) + graphics::getTopMarginSize();
		}

		// The alias isn't really necessary but D3DCOLORVALUE is defined
		// in <d2d1.h>, so the alieas serves to make it where one is not
		// using a structure not directly included in a file.
		using Color = D3DCOLORVALUE;

		/// <summary>Class that handles rendering of 2D elements.</summary>
		class Renderer2D {
		public:
			Renderer2D(std::shared_ptr<DX::DeviceResources2D> dev_res) :
				device_resources {dev_res} {
			}

			/// <summary>Paints a game square a certain color.</summary>
			/// <param name="gx">The game x-coordinate of the square.</param>
			/// <param name="gy">The game y-coordinate of the square.</param>
			/// <param name="o_color">The color to outline the square with.</param>
			/// <param name="f_color">The color to fill the square with.</param>
			void paintSquare(double gx, double gy, Color o_color, Color f_color) const noexcept;
			/// <summary>Draws the outline of a geometry.</summary>
			/// <param name="my_geom">The geometry to outline.</param>
			/// <param name="o_color">The color to outline the geometry with.</param>
			void drawGeometry(ID2D1Geometry* my_geom, Color o_color) const noexcept;
			/// <summary>Fills in the interior of a geometry.</summary>
			/// <param name="my_geom">The geometry to fill.</param>
			/// <param name="f_color">The color to fill the geometry with.</param>
			void fillGeometry(ID2D1Geometry* my_geom, Color f_color) const noexcept;
			
			/// <summary>Draws the current scene based on the game state.</summary>
			/// <param name="my_game">Shared pointer to object that contains the current game state.</param>
			/// <param name="mouse_gx">The starting game x-coordinate of the mouse.</param>
			/// <param name="mouse_gy">The starting game y-coordinate of the mouse.</param>
			/// <param name="mouse_end_gx">The ending game x-coordinate of the mouse.</param>
			/// <param name="mouse_end_gy">The ending game y-coordinate of the mouse.</param>
			HRESULT render(const std::shared_ptr<game::MyGame> my_game, int mouse_gx, int mouse_gy,
				int mouse_end_gx, int mouse_end_gy) const;
		protected:
			// Setters
			void setOutlineColor(Color o_color) const noexcept {
				this->device_resources->getOutlineBrush()->SetColor(o_color);
			}
			void setFillColor(Color f_color) const noexcept {
				this->device_resources->getFillBrush()->SetColor(f_color);
			}
			void setTextColor(Color t_color) const noexcept {
				this->device_resources->getTextBrush()->SetColor(t_color);
			}
			/// <summary>This function sets the outline and fill colors used in most drawing operations.</summary>
			/// <param name="o_color">The new color to use to outline shapes with.</param>
			/// <param name="f_color">The new color to use to fill shapes with.</param>
			void setBrushColors(Color o_color, Color f_color) const noexcept {
				this->setOutlineColor(o_color);
				this->setFillColor(f_color);
			}

			// Utility functions
			/// <summary>Fills a rectangle using the current fill color.</summary>
			/// <param name="my_rect">Stores the dimensions of the rectangle to fill.</param>
			void fillRectangle(D2D1_RECT_F my_rect) const noexcept {
				this->device_resources->getRenderTarget()->FillRectangle(my_rect, this->device_resources->getFillBrush());
			}
			/// <summary>Outlines a rectangle using the current outline color.</summary>
			/// <param name="my_rect">Stores the dimensions of the rectangle to outline.</param>
			void outlineRectangle(D2D1_RECT_F my_rect) const noexcept {
				this->device_resources->getRenderTarget()->DrawRectangle(my_rect, this->device_resources->getOutlineBrush());
			}
			// Creates a rectangle definition using the provided left x, top y, width,
			// and height of the rectangle
			constexpr static D2D1_RECT_F createRectangle(float lx, float ty, float w, float h) noexcept {
				return D2D1_RECT_F {lx, ty, lx + w, ty + h};
			}
		private:
			/// <summary>Shared pointer to the resources used by the renderer.</summary>
			mutable std::shared_ptr<DX::DeviceResources2D> device_resources;
		};
	}
}