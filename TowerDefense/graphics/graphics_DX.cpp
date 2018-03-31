// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <string>
#include "./../globals.hpp"
#include "./graphics_DX.hpp"

namespace hoffman::isaiah {
	namespace graphics::DX {
		void DeviceResources2D::createDeviceIndependentResources() {
			// Create Direct2D factory
			HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->factory);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Creation of Direct2D factory");
			}
		}

		HRESULT DeviceResources2D::createDeviceResources(HWND hwnd) {
			HRESULT hr = S_OK;
			if (!this->render_target) {
				// Retrieve client area size
				RECT rc;
				GetClientRect(hwnd, &rc);
				D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
				// Create Direct2D render target
				hr = this->factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
					D2D1::HwndRenderTargetProperties(hwnd, size), &this->render_target);
				// Create brushes
				const D2D1::ColorF color_black {0.0f, 0.0f, 0.0f, 1.0f};
				const D2D1::ColorF color_white {1.0f, 1.0f, 1.0f, 1.0f};
				if (SUCCEEDED(hr)) {
					hr = this->render_target->CreateSolidColorBrush(color_black, &this->outline_brush);
				}
				if (SUCCEEDED(hr)) {
					hr = this->render_target->CreateSolidColorBrush(color_white, &this->fill_brush);
				}
				if (SUCCEEDED(hr)) {
					hr = this->render_target->CreateSolidColorBrush(color_black, &this->text_brush);
				}
			}
			return hr;
		}
	}
}