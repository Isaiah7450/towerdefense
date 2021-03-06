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

namespace hoffman_isaiah {
	namespace game {
		// Forward declaration
		class MyGame;
		class EnemyType;
		class TowerType;
		class ShotBaseType;
		class GameMap;
	}

	namespace winapi {
		/// <summary>Disables a menu item.</summary>
		/// <param name="hwnd">Handle to the window.</param>
		/// <param name="menu_offset">The numeric offset of the menu (see order in resource file).</param>
		/// <param name="menu_identifier">The menu's resource identifier.</param>
		inline void disableMenuItem(HWND hwnd, int menu_offset, int menu_identifier) {
			constexpr MENUITEMINFO m_item {
				sizeof(MENUITEMINFO), MIIM_STATE, 0, MFS_DISABLED,
				0, nullptr, nullptr, nullptr, 0, nullptr, 0, nullptr
			};
			SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), menu_offset), menu_identifier, false, &m_item);
		}
		/// <summary>Enables a menu item.</summary>
		/// <param name="hwnd">Handle to the window.</param>
		/// <param name="menu_offset">The numeric offset of the menu (see order in resource file).</param>
		/// <param name="menu_identifier">The menu's resource identifier.</param>
		inline void enableMenuItem(HWND hwnd, int menu_offset, int menu_identifier) {
			constexpr MENUITEMINFO m_item {
				sizeof(MENUITEMINFO), MIIM_STATE, 0, MFS_ENABLED,
				0, nullptr, nullptr, nullptr, 0, nullptr, 0, nullptr
			};
			SetMenuItemInfo(GetSubMenu(GetMenu(hwnd), menu_offset), menu_identifier, false, &m_item);
		}
	}

	namespace terrain_editor {
		// Forward declaration
		class TerrainEditor;
	}

	namespace graphics {
		// The alias isn't really necessary but D3DCOLORVALUE is defined
		// in <d2d1.h>, so the alieas serves to make it where one is not
		// using a structure not directly included in a file.
		using Color = D3DCOLORVALUE;

		/// <summary>Class that handles rendering of 2D elements.</summary>
		class Renderer2D {
		public:
			Renderer2D(DX::DeviceResources2D* dev_res) :
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
			/// <param name="my_map">Reference to the game's terrain map.</param>
			/// <param name="gx">The game x-coordinate of the square.</param>
			/// <param name="gy">The game y-coordinate of the square.</param>
			/// <param name="o_color">The color to outline the square with.</param>
			/// <param name="f_color">The color to fill the square with.</param>
			void paintSquare(const game::GameMap& my_map, double gx, double gy, Color o_color, Color f_color) const noexcept;
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
			/// <param name="draw_rect">Whether or not to draw the enclosing rectangle.</param>
			void drawText(std::wstring text, Color t_color, D2D_RECT_F my_rect, bool draw_rect = true) const noexcept;
			/// <summary>Renders some small text on the screen.</summary>
			/// <param name="text">The text to render on the screen.</param>
			/// <param name="t_color">The color to render the text in.</param>
			/// <param name="my_rect">The enclosing rectangle of the text.</param>
			/// <param name="draw_rect">Whether or not to draw the enclosing rectangle.</param>
			void drawSmallText(std::wstring text, Color t_color, D2D_RECT_F my_rect, bool draw_rect = true) const noexcept;
			
			/// <summary>Draws the current scene based on the game state.</summary>
			/// <param name="my_game">Non-owning pointer to the current game state.</param>
			/// <param name="mouse_gx">The starting game x-coordinate of the mouse.</param>
			/// <param name="mouse_gy">The starting game y-coordinate of the mouse.</param>
			/// <param name="mouse_end_gx">The ending game x-coordinate of the mouse.</param>
			/// <param name="mouse_end_gy">The ending game y-coordinate of the mouse.</param>
			/// <param name="in_editor">Are we in the terrain editor?</param>
			HRESULT render(const game::MyGame* my_game, int mouse_gx, int mouse_gy,
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
			mutable graphics::DX::DeviceResources2D* device_resources;
		};
	}
}