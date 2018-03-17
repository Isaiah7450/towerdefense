// File Author: Isaiah Hoffman
// File Created: March 9, 2018
#include "./targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include "./globals.hpp"
#include "./graphics/graphics_DX.hpp"
#include "./graphics/graphics.hpp"

using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace graphics {
		int screen_width {800};
		int screen_height {600};
		int grid_width {40};
		int grid_height {40};
	}
}

namespace ih = hoffman::isaiah;

// WinMain function
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	auto my_window = ih::winapi::MainWindow {hInstance, nCmdShow};
	auto my_d3d_resources = std::make_shared<ih::graphics::DX::DeviceResources>(my_window.getHWND());
	auto my_renderer = std::make_unique<ih::graphics::DX::Renderer>(my_d3d_resources);
	my_renderer->createDeviceDependentResources();
	my_renderer->createWindowSizeDependentResources();
	// Message Loop
	MSG msg;
	bool endLoop = false;
	while (!endLoop) {
		if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			// Message received
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			endLoop = msg.message == WM_QUIT;
		}
		else {
			// Update scene
			my_renderer->update();
			// Render frames during idle time
			my_renderer->render();
			// Present frames to screen
			my_d3d_resources->present();
			// Sleep to avoid excessive CPU usage
			Sleep(1);
		}
	}
	return 0;
}