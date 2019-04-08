#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <memory>
#include <vector>
#include <map>
#include "./../globals.hpp"
#include "./graphics_DX.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declaration
		class MyGame;
		class EnemyType;
		class TowerType;
		class ShotBaseType;
		class GameMap;
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
			/// <summary>Updates the text for the health option.</summary>
			/// <param name="hwnd">Handle to the parent window.</param>
			/// <param name="new_price">The new price to buy health.</param>
			void updateHealthOption(HWND hwnd, int new_price) const noexcept;
			/// <summary>Updates the text for the change speed option.</summary>
			/// <param name="hwnd">Handle to the parent window.</param>
			/// <param name="new_update_speed">The new speed to update the game at.</param>
			void updateSpeedOption(HWND hwnd, int new_update_speed) const noexcept;
			/// <summary>Recreates the tower menu with the current list of towers.</summary>
			/// <param name="hwnd">Handle to the parent window.</param>
			/// <param name="towers">The list of tower types that the player can choose from.</param>
			void createTowerMenu(HWND hwnd, const std::vector<std::unique_ptr<game::TowerType>>& towers) const noexcept;
			/// <summary>Recreates the shots menu with the current list of shots.</summary>
			/// <param name="hwnd">Handle to the parent window.</param>
			/// <param name="shots">The list of shot types.</param>
			void createShotMenu(HWND hwnd, const std::map<std::wstring, std::unique_ptr<game::ShotBaseType>>& shots) const noexcept;
			/// <summary>Recreates the enemies menu with the current list of enemies (that have been seen before).</summary>
			/// <param name="hwnd">Handle to the parent window.</param>
			/// <param name="enemies">The list of enemy types.</param>
			/// <param name="seen_before">A list of boolean values that indicate whether a particular enemy
			/// type has been seen before.</param>
			void createEnemyMenu(HWND hwnd, const std::vector<std::unique_ptr<game::EnemyType>>& enemies,
				std::map<std::wstring, bool> seen_before) const noexcept;
			/// <summary>Updates the currently selected tower on the tower menu.</summary>
			/// <param name="hwnd">Handle to the parent window.</param>
			/// <param name="selected_tower">The index of the tower type currently selected starting from 1.</param>
			void updateSelectedTower(HWND hwnd, int selected_tower) const noexcept;

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

			/// <summary>Renders some text on the screen.</summary>
			/// <param name="text">The text to render on the screen.</param>
			/// <param name="t_color">The color to render the text in.</param>
			/// <param name="my_rect">The enclosing rectangle of the text.</param>
			void drawText(std::wstring text, Color t_color, D2D_RECT_F my_rect) const noexcept;
			
			/// <summary>Draws the current scene based on the game state.</summary>
			/// <param name="my_game">Shared pointer to object that contains the current game state.</param>
			/// <param name="mouse_gx">The starting game x-coordinate of the mouse.</param>
			/// <param name="mouse_gy">The starting game y-coordinate of the mouse.</param>
			/// <param name="mouse_end_gx">The ending game x-coordinate of the mouse.</param>
			/// <param name="mouse_end_gy">The ending game y-coordinate of the mouse.</param>
			/// <param name="in_editor">Are we in the terrain editor?</param>
			HRESULT render(const std::shared_ptr<game::MyGame> my_game, int mouse_gx, int mouse_gy,
				int mouse_end_gx, int mouse_end_gy, bool in_editor = false) const;
			/// <summary>Draws the current scene based on the editor's state.</summary>
			/// <param name="my_editor">Reference to the editor being drawn.</param>
			/// <param name="mouse_gx">The starting game x-coordinate of the mouse.</param>
			/// <param name="mouse_gy">The starting game y-coordinate of the mouse.</param>
			/// <param name="mouse_end_gx">The ending game x-coordinate of the mouse.</param>
			/// <param name="mouse_end_gy">The ending game y-coordinate of the mouse.</param>
			HRESULT render(const terrain_editor::TerrainEditor& my_editor, int mouse_gx, int mouse_gy,
				int mouse_end_gx, int mouse_end_gy) const;
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
			// Utility functions and general drawing functions
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
			/// <summary>Outlines an ellipse using the current outline color.</summary>
			/// <param name="my_ellipse">Stores the dimensions of the ellipse to outline.</param>
			void outlineEllipse(D2D1_ELLIPSE my_ellipse) const noexcept {
				this->device_resources->getRenderTarget()->DrawEllipse(my_ellipse, this->device_resources->getOutlineBrush());
			}
			// Creates an ellipse definition using the provided center and "radii" of the ellipse
			constexpr static D2D1_ELLIPSE createEllipse(float cx, float cy, float rw, float rh) noexcept {
				return D2D1_ELLIPSE {D2D1_POINT_2F {cx, cy}, rw, rh};
			}
			// Creates a rectangle definition using the provided left x, top y, width,
			// and height of the rectangle
			constexpr static D2D1_RECT_F createRectangle(float lx, float ty, float w, float h) noexcept {
				return D2D1_RECT_F {lx, ty, lx + w, ty + h};
			}
		protected:
			/// <summary>Highlights the currently selected squares.</summary>
			/// <param name="map">Reference to the game map being painted.</param>
			/// <param name="mouse_gx">The starting game x-coordinate of the mouse.</param>
			/// <param name="mouse_gy">The starting game y-coordinate of the mouse.</param>
			/// <param name="mouse_end_gx">The ending game x-coordinate of the mouse.</param>
			/// <param name="mouse_end_gy">The ending game y-coordinate of the mouse.</param>
			void paintMouseSquares(const game::GameMap& map, int mouse_gx, int mouse_gy, int mouse_end_gx, int mouse_end_gy) const noexcept;
		private:
			/// <summary>Shared pointer to the resources used by the renderer.</summary>
			mutable std::shared_ptr<DX::DeviceResources2D> device_resources;
		};
	}
}