// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <memory>
#include <string>
#include "./../globals.hpp"
#include "./graphics_DX.hpp"

namespace hoffman_isaiah {
	namespace graphics::DX {
		void DeviceResources2D::createDeviceIndependentResources() {
			// Create Direct2D factory
			ID2D1Factory* raw_factory {nullptr};
			HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &raw_factory);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Creation of Direct2D factory");
			}
			this->factory.reset(raw_factory, winapi::ReleaseCOM<ID2D1Factory>());
			IDWriteFactory* raw_write_factory {nullptr};
			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&raw_write_factory));
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Creation of DirectWrite factory");
			}
			this->write_factory.reset(raw_write_factory, winapi::ReleaseCOM<IDWriteFactory>());
			IDWriteTextFormat* raw_text_format {nullptr};
			hr = this->write_factory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.f,
				L"en-us", &raw_text_format);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Creation of text format");
			}
			this->text_format.reset(raw_text_format, winapi::ReleaseCOM<IDWriteTextFormat>());
			hr = this->text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Horizontal centering of text");
			}
			hr = this->text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Vertical centering of text");
			}
			raw_text_format = nullptr;
			hr = this->write_factory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10.f,
				L"en-us", &raw_text_format);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Creation of small text format");
			}
			this->small_text_format.reset(raw_text_format, winapi::ReleaseCOM<IDWriteTextFormat>());
			hr = this->small_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Horizontal centering of small text");
			}
			hr = this->small_text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Vertical centering of small text");
			}
		}

		HRESULT DeviceResources2D::createDeviceResources(HWND hwnd) {
			HRESULT hr = S_OK;
			if (!this->render_target) {
				// Retrieve client area size
				RECT rc;
				GetClientRect(hwnd, &rc);
				const D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
				// Create Direct2D render target
				ID2D1HwndRenderTarget* raw_render_target {nullptr};
				hr = this->factory->CreateHwndRenderTarget(
					D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
					D2D1::HwndRenderTargetProperties(hwnd, size), &raw_render_target);
				this->render_target.reset(raw_render_target, winapi::ReleaseCOM<ID2D1HwndRenderTarget>());
				// Create brushes
				const D2D1::ColorF color_black {0.0f, 0.0f, 0.0f, 1.0f};
				const D2D1::ColorF color_white {1.0f, 1.0f, 1.0f, 1.0f};
				ID2D1SolidColorBrush* raw_brush {nullptr};
				if (SUCCEEDED(hr)) {
					hr = this->render_target->CreateSolidColorBrush(color_black, &raw_brush);
					this->outline_brush.reset(raw_brush, winapi::ReleaseCOM<ID2D1SolidColorBrush>());
				}
				raw_brush = nullptr;
				if (SUCCEEDED(hr)) {
					hr = this->render_target->CreateSolidColorBrush(color_white, &raw_brush);
					this->fill_brush.reset(raw_brush, winapi::ReleaseCOM<ID2D1SolidColorBrush>());
				}
				raw_brush = nullptr;
				if (SUCCEEDED(hr)) {
					hr = this->render_target->CreateSolidColorBrush(color_black, &raw_brush);
					this->text_brush.reset(raw_brush, winapi::ReleaseCOM<ID2D1SolidColorBrush>());
				}
			}
			return hr;
		}
	}
}
