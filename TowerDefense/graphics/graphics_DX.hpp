#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "../targetver.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <string>

#pragma comment(lib, "d3d11.lib")

namespace hoffman::isaiah {
	namespace winapi {
		[[noreturn]] void handleWindowsError(std::wstring lpszFunction);

		class MainWindow {
		public:
			/// <summary>Creates a new main window.</summary>
			/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
			/// <param name="nCmdShow">The nCmdShow parameter given by the WinMain function.</param>
			MainWindow(HINSTANCE h_inst, int nCmdShow);
			/// <summary>Return's the name of the window.</summary>
			constexpr static const wchar_t* getWindowName() noexcept {
				return MainWindow::window_name;
			}
			static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			// Getters
			HWND getHWND() const noexcept {
				return this->hwnd;
			}
		private:
			/// <summary>The handle to the application instance.</summary>
			HINSTANCE h_instance;
			/// <summary>The handle to the window.</summary>
			HWND hwnd {nullptr};
			/// <summary>The handle to the window's menu.</summary>
			HMENU h_menu {nullptr};
			/// <summary>The current window dimensions.</summary>
			RECT rc {};
			// The class name
			static constexpr const wchar_t* class_name {L"my_game"};
			static constexpr const wchar_t* window_name {L"Isaiah's tower defense game"};
		};
	}

	namespace graphics {
		// For use with COM pointers
		template <typename T>
		void SafeRelease(T** ppT) {
			if (*ppT) {
				(*ppT)->Release();
				*ppT = nullptr;
			}
		}
	}

	namespace graphics::DX {
		/// <summary>Stores the Direct3D device resources.</summary>
		class DeviceResources {
		public:
			/// <summary>Initializes the Direct3D device resources.</summary>
			/// <param name="hwnd">Handle to the main window.</param>
			DeviceResources(HWND hwnd);
			// I couldn't get _com_ptr_t to work nor the header <wrl.h> to work,
			// so this will have to suffice.
			// I considered std::unique_ptr or std::shared_ptr, but I'm not sure
			// if those work well with COM objects.
			// The main thing to watch for is having multiple references to the
			// same object --> for the most part, this class "owns" the resources
			// freed by this class's destructor.
			/// <summary>Releases COM resources used by the program.</summary>
			~DeviceResources() noexcept {
				SafeRelease(&this->depth_stencil_view);
				SafeRelease(&this->depth_stencil);
				SafeRelease(&this->swap_chain);
				SafeRelease(&this->dxgi_device);
				SafeRelease(&this->device_context);
				SafeRelease(&this->device);
			}
			/// <summary>Creates the swap chain and additional basic Direct3D device resources.</summary>
			void createWindowResources(HWND hwnd);
			/// <summary>Presents the scene on the primary surface.</summary>
			void present() const {
				this->swap_chain->Present(1, 0);
			}
			// Getters
			ID3D11Device* getDevice() const noexcept {
				return this->device;
			}
			ID3D11DeviceContext* getDeviceContext() const noexcept {
				return this->device_context;
			}
			ID3D11RenderTargetView* getRenderTargetView() const noexcept {
				return this->render_target;
			}
			ID3D11DepthStencilView* getDepthStencilView() const noexcept {
				return this->depth_stencil_view;
			}
			/// <returns>The current aspect ratio.</returns>
			float getAspectRatio() const noexcept {
				return static_cast<float>(this->back_buffer_desc.Width) /
					static_cast<float>(this->back_buffer_desc.Height);
			}
		protected:
			// (This function is automatically called by the constructor alongside
			// createWindowResources())
			// (Also note that if this function or createWindowResources() fails,
			// the entire program will be terminated.)
			/// <summary>Creates the initial Direct3D device resources.</summary>
			void createDeviceResources();
		private:
			// Fields
			/// <summary>Pointer to the Direct3D device.</summary>
			ID3D11Device* device {nullptr};
			/// <summary>Pointer to the Direct3D device context.</summary>
			ID3D11DeviceContext* device_context {nullptr};
			/// <summary>The current feature level of the program.</summary>
			D3D_FEATURE_LEVEL feature_level {};
			/// <summary>The DXGI device object to use in other factories, such as Direct2D.</summary>
			IDXGIDevice1* dxgi_device {nullptr};
			/// <summary>Pointer to the Direct3D swap chain.</summary>
			IDXGISwapChain* swap_chain {nullptr};
			/// <summary>Pointer to the back buffer list.</summary>
			ID3D11Texture2D* back_buffer {nullptr};
			/// <summary>Pointer to the Direct3D render target view.</summary>
			ID3D11RenderTargetView* render_target {nullptr};
			/// <summary>The description of the back buffer.</summary>
			D3D11_TEXTURE2D_DESC back_buffer_desc {};
			/// <summary>Pointer to the depth-stencil buffer.</summary>
			ID3D11Texture2D* depth_stencil {nullptr};
			/// <summary>Pointer to the depth-stencil buffer view.</summary>
			ID3D11DepthStencilView* depth_stencil_view {nullptr};
			/// <summary>The current viewport dimensions.</summary>
			D3D11_VIEWPORT viewport {};
		};
	}
}