// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <memory>
#include "./graphics_DX.hpp"
#include "./graphics.hpp"

namespace hoffman::isaiah {
	namespace graphics {
		HRESULT Renderer2D::render(const game::MyGame& my_game) const {
			UNREFERENCED_PARAMETER(my_game);
			auto render_target = this->device_resources->getRenderTarget();
			render_target->BeginDraw();
			return render_target->EndDraw();
		}
	}
}