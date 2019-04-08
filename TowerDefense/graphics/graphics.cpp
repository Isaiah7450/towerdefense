// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../resource.h"
#include "./../game/enemy_type.hpp"
#include "./../game/enemy.hpp"
#include "./../game/game_object.hpp"
#include "./../game/my_game.hpp"
#include "./../game/shot_types.hpp"
#include "./../game/shot.hpp"
#include "./../game/tower_types.hpp"
#include "./../game/tower.hpp"
#include "./../pathfinding/graph_node.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./../terrain/editor.hpp"
#include "./graphics_DX.hpp"
#include "./graphics.hpp"
#include "./shapes.hpp"
using namespace std::literals::string_literals;
namespace hoffman::isaiah {
	namespace graphics {
		void Renderer2D::updateHealthOption(HWND hwnd, int new_price) const noexcept {
			auto my_menu = GetSubMenu(GetMenu(hwnd), 1);
			MENUITEMINFO my_info {};
			my_info.cbSize = sizeof(MENUITEMINFO);
			my_info.fMask = MIIM_DATA | MIIM_STRING;
			GetMenuItemInfo(my_menu, ID_MM_ACTIONS_BUY_HEALTH, FALSE, &my_info);
			std::wstring new_string = L"Buy Health: $" + std::to_wstring(new_price) + L"\0";
			my_info.dwTypeData = new_string.data();
			SetMenuItemInfo(my_menu, ID_MM_ACTIONS_BUY_HEALTH, FALSE, &my_info);
			DrawMenuBar(hwnd);
		}

		void Renderer2D::updateSpeedOption(HWND hwnd, int new_update_speed) const noexcept {
			auto my_menu = GetSubMenu(GetMenu(hwnd), 1);
			MENUITEMINFO my_info {};
			my_info.cbSize = sizeof(MENUITEMINFO);
			my_info.fMask = MIIM_DATA | MIIM_STRING;
			GetMenuItemInfo(my_menu, ID_MM_ACTIONS_CHANGE_SPEED, FALSE, &my_info);
			std::wstring new_string = L"";
			if (new_update_speed == 1) {
				new_string = L"Reset Speed to 1x";
			}
			else {
				new_string = L"Increase Speed to " + std::to_wstring(new_update_speed) + L"x";
			}
			my_info.dwTypeData = new_string.data();
			SetMenuItemInfo(my_menu, ID_MM_ACTIONS_CHANGE_SPEED, FALSE, &my_info);
			DrawMenuBar(hwnd);
		}

		void Renderer2D::createTowerMenu(HWND hwnd,
			const std::vector<std::shared_ptr<game::TowerType>>& towers) const noexcept {
			auto my_menu = GetSubMenu(GetMenu(hwnd), 2);
			// Delete what may have previously been in the menu...
			while (GetMenuItemCount(my_menu) > 2) {
				DeleteMenu(my_menu, 2, MF_BYPOSITION);
			}
			// Add menu items
			for (uintptr_t i = 0; i < towers.size(); ++i) {
				std::wstring my_tower_str = towers[i]->getName() + L": $"s
					+ std::to_wstring(static_cast<int>(std::ceil(towers[i]->getCost())));
				AppendMenu(my_menu, MF_STRING, ID_MM_TOWERS_NONE + i + 1, my_tower_str.c_str());
			}
			this->updateSelectedTower(hwnd, ID_MM_TOWERS_NONE);
		}

		void Renderer2D::createShotMenu(HWND hwnd,
			const std::map<std::wstring, std::shared_ptr<game::ShotBaseType>>& shots) const noexcept {
			auto my_menu = GetSubMenu(GetMenu(hwnd), 3);
			// Delete what may have previously been in the menu...
			while (GetMenuItemCount(my_menu) > 0) {
				DeleteMenu(my_menu, 0, MF_BYPOSITION);
			}
			// Add menu items
			uintptr_t i = 0;
			for (const auto& stype : shots) {
				AppendMenu(my_menu, MF_STRING, ID_MM_SHOTS_PLACEHOLDER + i, stype.first.c_str());
				++i;
			}
		}

		void Renderer2D::createEnemyMenu(HWND hwnd,
			const std::vector<std::shared_ptr<game::EnemyType>>& enemies,
			std::map<std::wstring, bool> seen_before) const noexcept {
			auto my_menu = GetSubMenu(GetMenu(hwnd), 4);
			// Clear the menu.
			while (GetMenuItemCount(my_menu) > 0) {
				DeleteMenu(my_menu, 0, MF_BYPOSITION);
			}
			// Add menu items
			uintptr_t i = 0;
			for (const auto& etype : enemies) {
				if (seen_before.at(etype->getName())) {
					AppendMenu(my_menu, MF_STRING, ID_MM_ENEMIES_PLACEHOLDER + i, etype->getName().c_str());
				}
				else {
					AppendMenu(my_menu, MF_STRING, ID_MM_ENEMIES_PLACEHOLDER + i, L"???");
				}
				++i;
			}
		}

		void Renderer2D::updateSelectedTower(HWND hwnd, int selected_tower) const noexcept {
			auto my_menu = GetSubMenu(GetMenu(hwnd), 2);
			const auto num_items = GetMenuItemCount(my_menu);
			CheckMenuRadioItem(my_menu, ID_MM_TOWERS_NONE, ID_MM_TOWERS_NONE + num_items, selected_tower,
				MF_BYCOMMAND);
			DrawMenuBar(hwnd);
		}

		void Renderer2D::paintSquare(double gx, double gy, Color o_color, Color f_color) const noexcept {
			this->setBrushColors(o_color, f_color);
			const float slx = static_cast<float>(graphics::convertToScreenX(gx));
			const float sty = static_cast<float>(graphics::convertToScreenY(gy));
			const float sw = graphics::getGameSquareWidth<float>();
			const float sh = graphics::getGameSquareHeight<float>();
			const auto my_rect = this->createRectangle(slx, sty, sw, sh);
			this->fillRectangle(my_rect);
			this->outlineRectangle(my_rect);
		}

		void Renderer2D::drawGeometry(ID2D1Geometry* my_geom, Color o_color) const noexcept {
			this->setOutlineColor(o_color);
			this->device_resources->getRenderTarget()->DrawGeometry(my_geom, this->device_resources->getOutlineBrush());
		}

		void Renderer2D::fillGeometry(ID2D1Geometry* my_geom, Color f_color) const noexcept {
			this->setFillColor(f_color);
			this->device_resources->getRenderTarget()->FillGeometry(my_geom, this->device_resources->getFillBrush());
		}

		void Renderer2D::drawText(std::wstring text, Color t_color, D2D_RECT_F my_rect) const noexcept {
			this->setTextColor(t_color);
			this->device_resources->getRenderTarget()->DrawText(text.c_str(), static_cast<UINT32>(text.size()),
				this->device_resources->getTextFormat(), my_rect, this->device_resources->getTextBrush());
// #if defined(DEBUG) || defined(_DEBUG)
			this->outlineRectangle(my_rect);
// #endif // DEBUG || _DEBUG
		}

		HRESULT Renderer2D::render(const std::shared_ptr<game::MyGame> my_game, int mouse_gx, int mouse_gy,
			int mouse_end_gx, int mouse_end_gy, bool in_editor) const {
			// Check time before rendering
			static LARGE_INTEGER last_update_time {0};
			if (last_update_time.QuadPart == 0) {
				QueryPerformanceCounter(&last_update_time);
			}
			const auto my_times = winapi::MainWindow::getElapsedTime(last_update_time);
			if (my_times.second.QuadPart < math::getMicrosecondsInSecond() / game::graphics_framerate) {
				Sleep(1);
				return S_OK;
			}
			last_update_time = my_times.first;
			// Obtain lock
			auto update_event = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, false, TEXT("can_update"));
			if (!update_event) {
				return S_FALSE;
			}
			ResetEvent(update_event);
			// Do drawing
			auto render_target = this->device_resources->getRenderTarget();
			render_target->BeginDraw();
			render_target->Clear(Color {1.f, 1.f, 1.f, 1.f});
			// Draw terrain
			my_game->getMap().draw(*this, in_editor);
			// Draw shots, towers, and enemies.
			if (!in_editor) {
				for (const auto& s : my_game->shots) {
					s->draw(*this);
				}
				for (const auto& t : my_game->towers) {
					t->draw(*this);
				}
				for (const auto& e : my_game->enemies) {
					e->draw(*this);
				}
			}
#if 0
			my_game->level = 999;
			my_game->difficulty = 99.9922;
			if (my_game->player.getHealth() < 2000) {
				my_game->player.changeHealth(1000);
				my_game->player.changeMoney(25514.472);
			}
#endif
			// Draw text
			const float margin_left_sx = static_cast<float>(graphics::screen_width * graphics::margin_left);
			const float margin_top_sy = static_cast<float>(graphics::screen_height * graphics::margin_top);
			const float text_rect_top_sy = margin_top_sy / 4.f;
			constexpr const float text_rect_height = 15.f;
			this->setOutlineColor(Color {0.5f, 0.5f, 0.5f, 1.f});
			std::wstring level_text = L"Level: "s + std::to_wstring(my_game->level);
			const auto level_rect = Renderer2D::createRectangle(margin_left_sx,
				text_rect_top_sy, 70.f, text_rect_height);
			this->drawText(level_text, Color {0.f, 0.f, 0.f, 1.f}, level_rect);
			std::wostringstream difficulty_text {};
			difficulty_text << L"Difficulty: " << std::setprecision(2) << std::fixed << my_game->difficulty;
			const auto difficulty_rect = Renderer2D::createRectangle(margin_left_sx + 70.f + 25.f,
				text_rect_top_sy, 120.f, text_rect_height);
			this->drawText(difficulty_text.str(), Color {0.f, 0.f, 0.f, 1.f}, difficulty_rect);
			std::wstring life_text = L"Life: "s + std::to_wstring(my_game->player.getHealth());
			const auto life_rect = Renderer2D::createRectangle(margin_left_sx + 95.f + 120.f + 25.f,
				text_rect_top_sy, 75.f, text_rect_height);
			this->drawText(life_text, Color {0.f, 0.f, 0.f, 1.f}, life_rect);
			std::wostringstream cash_text {};
			cash_text << L"Cash: $" << std::setprecision(0) << std::fixed << my_game->player.getMoney();
			const auto cash_rect = Renderer2D::createRectangle(margin_left_sx + 240.f + 75.f + 25.f,
				text_rect_top_sy, 100.f, text_rect_height);
			this->drawText(cash_text.str(), Color {0.f, 0.f, 0.f, 1.f}, cash_rect);
			if (!my_game->player.isAlive()) {
				const long long score = static_cast<long long>((my_game->level * 50.0 + my_game->difficulty * 125.0)
					* (my_game->challenge_level + 1.0)) + static_cast<long long>(my_game->player.getMoney() * 15.0)
					+ (my_game->level > 99 ? 25000ll
						: my_game->level > 90 ? 20000ll
						: my_game->level > 75 ? 15000ll
						: my_game->level > 50 ? 10000ll
						: my_game->level > 25 ? 5000ll
						: my_game->level > 10 ? 2500ll : 0ll);
				std::wstring score_text = L"Game over! Final Score: "s + std::to_wstring(score);
				const auto score_rect = Renderer2D::createRectangle(margin_left_sx + 340.f + 100.f + 25.f,
					text_rect_top_sy, 250.f, text_rect_height);
				this->drawText(score_text, Color {0.f, 0.f, 0.f, 1.f}, score_rect);
			}
#if (defined(DEBUG) || defined(_DEBUG))
			if (my_game->show_test_paths
				&& my_game->ground_test_pf->checkPathExists()) {
				// Paint pathfinder paths.
				auto ground_path = my_game->ground_test_pf->findPath(0);
				while (!ground_path.empty()) {
					auto my_node = ground_path.front();
					ground_path.pop();
					this->paintSquare(my_node.getGameX(), my_node.getGameY(), Color {0.f, 1.f, 0.f, 1.f},
						Color {0.8f, 0.8f, 0.8f, 0.3f});
				}
				auto air_path = my_game->air_test_pf->findPath(0);
				while (!air_path.empty()) {
					auto my_node = air_path.front();
					air_path.pop();
					this->paintSquare(my_node.getGameX(), my_node.getGameY(), Color {1.f, 0.f, 0.f, 1.f},
						Color {0.8f, 0.8f, 0.8f, 0.3f});
				}
			}
#endif // DEBUG | _DEBUG -> Path Debugging
			this->paintMouseSquares(my_game->getMap(), mouse_gx, mouse_gy, mouse_end_gx, mouse_end_gy);
			// Release lock
			SetEvent(update_event);
			CloseHandle(update_event);
			return render_target->EndDraw();
		}

		HRESULT Renderer2D::render(const terrain_editor::TerrainEditor& my_editor, int mouse_gx, int mouse_gy,
			int mouse_end_gx, int mouse_end_gy) const {
			// Check time before rendering
			static LARGE_INTEGER last_update_time {0};
			if (last_update_time.QuadPart == 0) {
				QueryPerformanceCounter(&last_update_time);
			}
			const auto my_times = winapi::MainWindow::getElapsedTime(last_update_time);
			if (my_times.second.QuadPart < math::getMicrosecondsInSecond() / game::graphics_framerate) {
				Sleep(1);
				return S_OK;
			}
			last_update_time = my_times.first;
			// Obtain lock
			auto update_event = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, false, TEXT("can_update"));
			if (!update_event) {
				return S_FALSE;
			}
			ResetEvent(update_event);
			// Do drawing
			auto render_target = this->device_resources->getRenderTarget();
			render_target->BeginDraw();
			render_target->Clear(Color {1.f, 1.f, 1.f, 1.f});
			// Draw terrain
			my_editor.getMap().draw(*this, true);
			this->paintMouseSquares(my_editor.getMap(), mouse_gx, mouse_gy, mouse_end_gx, mouse_end_gy);
			// Release lock
			SetEvent(update_event);
			CloseHandle(update_event);
			return render_target->EndDraw();
		}

		void Renderer2D::paintMouseSquares(const game::GameMap& map, int mouse_gx, int mouse_gy, int mouse_end_gx, int mouse_end_gy) const noexcept {
			// Highlight squares
			if (map.getTerrainGraph(false).verifyCoordinates(mouse_gx, mouse_gy)
				&& map.getTerrainGraph(false).verifyCoordinates(mouse_end_gx, mouse_end_gy)) {
				const auto min_gx = math::get_min(mouse_gx, mouse_end_gx);
				const auto min_gy = math::get_min(mouse_gy, mouse_end_gy);
				const auto max_gx = math::get_max(mouse_gx, mouse_end_gx);
				const auto max_gy = math::get_max(mouse_gy, mouse_end_gy);
				constexpr const graphics::Color o_color {1.0f, 0.f, 0.f, 1.0f};
				constexpr const graphics::Color f_color {0.8f, 0.8f, 0.8f, 0.2f};
				for (int gx = min_gx; gx <= max_gx; ++gx) {
					for (int gy = min_gy; gy <= max_gy; ++gy) {
						this->paintSquare(gx, gy, o_color, f_color);
					}
				}
			}
		}
	}
}
