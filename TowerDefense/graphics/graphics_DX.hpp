#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "../targetver.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <string>

namespace hoffman::isaiah {
	namespace graphics::DX {
		/// <summary>Pointer to the swap chain used by Direct3D.</summary>
		extern IDXGISwapChain* pSwapChain;
		/// <summary>Pointer to the Direct3D device interface.</summary>
		extern ID3D11Device* pDevice;
		/// <summary>Pointer to the Direct3D device context.</summary>
		extern ID3D11DeviceContext* pContext;
		/// <summary>Pointer to the Direct3D rendering target.</summary>
		extern ID3D11RenderTargetView* pRT;
		/// <summary>Initializes Direct3D.</summary>
		/// <param name="hwnd">Handle to the application's window.</param>
		void initialize_d3d(HWND hwnd);
		/// <summary>Cleans up after Direct3D.</summary>
		void clean_up_d3d();

		// Make sure we safely release DirectX objects
		template <typename T>
		void SafeRelease(T** ppT) {
			// i.e.: not null
			if (*ppT) {
				(*ppT)->Release();
				*ppT = nullptr;
			}
		}
	}
	
	namespace graphics {
		/// <summary>Renders a single frame.</summary>
		void render_frame();
	}
}

#pragma comment(lib, "d3d11.lib")