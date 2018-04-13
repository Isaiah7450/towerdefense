// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <memory>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../main.hpp"
#include "./graphics_DX.hpp"
#include "./graphics.hpp"
#include "./../pathfinding/graph_node.hpp"
#include "./../pathfinding/grid.hpp"
#include "./../pathfinding/pathfinder.hpp"
#include "./../game/my_game.hpp"
#include "./../terrain/editor.hpp"

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
			my_game->getMap().draw(*this);
			// Paint pathfinder paths
			if (my_game->ground_test_pf->checkPathExists()) {
				auto ground_path = my_game->ground_test_pf->findPath();
				while (!ground_path.empty()) {
					auto my_node = ground_path.front();
					ground_path.pop();
					this->paintSquare(my_node->getGameX(), my_node->getGameY(), Color {0.f, 1.f, 0.f, 1.f},
						Color {0.8f, 0.8f, 0.8f, 0.3f});
				}
			}
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