// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./../game/enemy.hpp"
#include "./../game/game_object.hpp"
#include "./../game/my_game.hpp"
#include "./../game/shot.hpp"
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
			this->device_resources->getRenderTarget()->DrawText(text.c_str(), text.size(),
				this->device_resources->getTextFormat(), my_rect, this->device_resources->getTextBrush());
#if defined(DEBUG) || defined(_DEBUG)
			this->outlineRectangle(my_rect);
#endif // DEBUG || _DEBUG
		}

		HRESULT Renderer2D::render(const std::shared_ptr<game::MyGame> my_game, int mouse_gx, int mouse_gy,
			int mouse_end_gx, int mouse_end_gy) const {
			UNREFERENCED_PARAMETER(my_game);
			// Check time before rendering
			static LARGE_INTEGER last_update_time = LARGE_INTEGER {0};
			auto my_times = winapi::MainWindow::getElapsedTime(last_update_time);
			if (my_times.second.QuadPart < math::getMicrosecondsInSecond() / game::graphics_framerate) {
				Sleep(0);
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
			my_game->getMap().draw(*this);
			// Draw shots, towers, and enemies.
			for (const auto& s : my_game->shots) {
				s->draw(*this);
			}
			for (const auto& t : my_game->towers) {
				t->draw(*this);
			}
			for (const auto& e : my_game->enemies) {
				e->draw(*this);
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
				int score = static_cast<int>((my_game->level * 1000.0 + my_game->difficulty * 2500.0)
					* (my_game->challenge_level + 1.0)) + static_cast<int>(my_game->player.getMoney() * 300);
				std::wstring score_text = L"Game over! Final Score: "s + std::to_wstring(score);
				const auto score_rect = Renderer2D::createRectangle(margin_left_sx + 340.f + 100.f + 25.f,
					text_rect_top_sy, 225.f, text_rect_height);
				this->drawText(score_text, Color {0.f, 0.f, 0.f, 1.f}, score_rect);
			}
#if (defined(DEBUG) || defined(_DEBUG)) && 0
			// Paint pathfinder paths
			// (Not sure why if I don't use the preprocessor to
			// comment out this code, it causes a crash every
			// single time after I use revert to last save
			// in the terrain editor on any builds without DEBUG
			// defined.)
			if (my_game->ground_test_pf->checkPathExists()) {
				auto ground_path = my_game->ground_test_pf->findPath();
				while (!ground_path.empty()) {
					auto my_node = ground_path.front();
					ground_path.pop();
					this->paintSquare(my_node->getGameX(), my_node->getGameY(), Color {0.f, 1.f, 0.f, 1.f},
						Color {0.8f, 0.8f, 0.8f, 0.3f});
				}
				auto air_path = my_game->air_test_pf->findPath();
				while (!air_path.empty()) {
					auto my_node = air_path.front();
					air_path.pop();
					this->paintSquare(my_node->getGameX(), my_node->getGameY(), Color {1.f, 0.f, 0.f, 1.f},
						Color {0.8f, 0.8f, 0.8f, 0.3f});
				}
			}
#endif // DEBUG | _DEBUG -> Path Debugging
			// Highlight squares
			if (my_game->getMap().getTerrainGraph(false).verifyCoordinates(mouse_gx, mouse_gy)
				&& my_game->getMap().getTerrainGraph(false).verifyCoordinates(mouse_end_gx, mouse_end_gy)) {
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
			// Release lock
			SetEvent(update_event);
			CloseHandle(update_event);
			return render_target->EndDraw();
		}
	}
}