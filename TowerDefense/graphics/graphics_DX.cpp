// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <string>
#include <stdexcept>
#include "./graphics_DX.hpp"
#include "./../globals.hpp"

#pragma comment(lib, "d3d11.lib")

using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace graphics::DX {
		IDXGISwapChain* pSwapChain {nullptr};
		ID3D11Device* pDevice {nullptr};
		ID3D11DeviceContext* pContext {nullptr};
		ID3D11RenderTargetView* pRT {nullptr};

		void initialize_d3d(HWND hwnd) {
			// Structure holding info about swap chain
			DXGI_SWAP_CHAIN_DESC scd;
			ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
			// Number of back buffers
			scd.BufferCount = 1;
			// 32-bit color
			scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			// Buffer screen resolution
			scd.BufferDesc.Width = graphics::screen_width;
			scd.BufferDesc.Height = graphics::screen_height;
			// How to use swap chain
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			scd.OutputWindow = hwnd;
			// Number of multisamples (???)
			// Oh, it is for anti-aliasing (smooths jagged corners)
			scd.SampleDesc.Count = 4;
			// Window/Full-Screen Mode
			scd.Windowed = TRUE;
			// Flags -> Currently, allows ALT+ENTER to be used to switch between full-screen and windowed mode
			scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			// Create device, device context, and swap chain using scd info
			D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
				nullptr, 0, D3D11_SDK_VERSION, &scd, &DX::pSwapChain, &DX::pDevice, nullptr, &DX::pContext);
			// Get address of back buffer
			ID3D11Texture2D* pBackBuffer;
			HRESULT hr = DX::pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));
			if (FAILED(hr)) {
				throw std::runtime_error {"Error Code #"s + std::to_string(hr) + " occurred "s
					"when attempting to initialize Direct3D."s};
			}
			// Use back buffer address to create render target
			DX::pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &DX::pRT);
			SafeRelease(&pBackBuffer);
			// Set render target as back buffer
			DX::pContext->OMSetRenderTargets(1, &DX::pRT, nullptr);
			// Set the viewport which normalizes pixel coordinates
			D3D11_VIEWPORT viewport;
			ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = static_cast<FLOAT>(graphics::screen_width);
			viewport.Height = static_cast<FLOAT>(graphics::screen_height);
			DX::pContext->RSSetViewports(1, &viewport);
		}

		void clean_up_d3d() {
			// Switch to windowed mode
			DX::pSwapChain->SetFullscreenState(FALSE, nullptr);
			// Close and release existing COM objects
			SafeRelease(&DX::pRT);
			SafeRelease(&DX::pContext);
			SafeRelease(&DX::pDevice);
			SafeRelease(&DX::pSwapChain);
		}
	}

	namespace graphics {
		void render_frame() {
			// Clear back buffer's background color
			constexpr const FLOAT rgba[4] {0.0f, 0.0f, 0.0f, 1.0f};
			DX::pContext->ClearRenderTargetView(DX::pRT, rgba);
			// TODO: Do 3D rendering...
			// Switch back buffer and front buffer
			DX::pSwapChain->Present(0, 0);
		}
	}
}