// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string.h>
#include <strsafe.h>
#include "./graphics_DX.hpp"
#include "./../globals.hpp"

namespace hoffman::isaiah {
	namespace winapi {
		[[noreturn]] void handleWindowsError(std::wstring lpszFunction) {
			// (Copied straight from help files... Hopefully it works!)
			// (I did replace the C-style casts with a C++ version.)
			// Retrieve the system error message for the last-error code
			LPVOID lpMsgBuf {nullptr};
			LPVOID lpDisplayBuf {nullptr};
			const DWORD dw = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
				| FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&lpMsgBuf), 0,
				nullptr);
			// Display the error message and exit the process
			lpDisplayBuf = static_cast<LPVOID>(LocalAlloc(LMEM_ZEROINIT,
				lstrlen(static_cast<LPWSTR>(lpMsgBuf)) + lpszFunction.size() + 120 * sizeof(TCHAR)));
			if (!lpDisplayBuf) {
				MessageBox(nullptr, L"Critical Error: Out of memory!", TEXT("Error"), MB_OK);
				ExitProcess(dw);
			}
			StringCchPrintf(static_cast<LPTSTR>(lpDisplayBuf), LocalSize(lpDisplayBuf) / sizeof(TCHAR),
				TEXT("%s failed with error %d: %s"), lpszFunction.c_str(), dw, static_cast<LPWSTR>(lpMsgBuf));
			MessageBox(nullptr, static_cast<LPWSTR>(lpDisplayBuf), TEXT("Error"), MB_OK);
			LocalFree(lpMsgBuf);
			LocalFree(lpDisplayBuf);
			ExitProcess(dw);
		}

		LRESULT CALLBACK MainWindow::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			switch (msg) {
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				break;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		MainWindow::MainWindow(HINSTANCE h_inst, int nCmdShow) :
			h_instance {h_inst} {
			// Register window class
			WNDCLASSEX wnd_class {
				sizeof(WNDCLASSEX), CS_DBLCLKS, MainWindow::windowProc, 0, 0, this->h_instance, nullptr,
				LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)),
				nullptr, MainWindow::class_name, nullptr
			};
			if (!RegisterClassEx(&wnd_class)) {
				winapi::handleWindowsError(L"Registration of window class");
			}
			// No menu (initialized to nullptr in class definition)
			// Determine window size and viewport
			SetRect(&this->rc, 0, 0, graphics::screen_width, graphics::screen_height);
			constexpr const auto dwStyles = WS_OVERLAPPEDWINDOW & (0xFFFFFFFFL ^ WS_MAXIMIZEBOX ^ WS_SIZEBOX);
			constexpr const auto dwExStyles = 0;
			AdjustWindowRectEx(&this->rc, dwStyles, (this->h_menu ? true : false), dwExStyles);
			// Create window
			this->hwnd = CreateWindowEx(dwExStyles, MainWindow::class_name, MainWindow::window_name,
				dwStyles, CW_USEDEFAULT, CW_USEDEFAULT, this->rc.right - this->rc.left,
				this->rc.bottom - this->rc.top, nullptr, this->h_menu, this->h_instance, nullptr);
			if (!this->hwnd) {
				winapi::handleWindowsError(L"Creation of window");
			}
			ShowWindow(this->hwnd, nCmdShow);
			UpdateWindow(this->hwnd);
		}
	}

	namespace graphics::DX {
		DeviceResources::DeviceResources(HWND hwnd) {
			this->createDeviceResources();
			this->createWindowResources(hwnd);
		}

		void DeviceResources::createDeviceResources() {
			constexpr const D3D_FEATURE_LEVEL feature_levels[] = {
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1
			};
#if defined(DEBUG) || defined(_DEBUG)
			constexpr const UINT device_debug_flags = D3D11_CREATE_DEVICE_DEBUG;
#else
			constexpr const UINT device_debug_flags = 0;
#endif
			// Required for compatibility with Direct2D
			constexpr const UINT device_extra_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
			constexpr const auto device_flags = device_debug_flags | device_extra_flags;
			// Create the device and device context
			HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags,
				feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &this->device,
				&this->feature_level, &this->device_context);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Direct3D device creation");
			}
		}

		void DeviceResources::createWindowResources(HWND hwnd) {
			// Create the swap chain
			DXGI_SWAP_CHAIN_DESC scd;
			ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
			// Start out in Windowed mode
			scd.Windowed = TRUE;
			// Number of back buffers
			scd.BufferCount = 2;
			// Color format to use
			scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			// How to use the swap chain's buffers
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			// Multisampling (Anti-aliasing) count
			scd.SampleDesc.Count = 1;
			// Also deals with anti-aliasing
			scd.SampleDesc.Quality = 0;
			scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			// Output window
			scd.OutputWindow = hwnd;
			HRESULT hr = this->device->QueryInterface(&this->dxgi_device);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Creation of DXGI device");
			}
			IDXGIAdapter* adapter {nullptr};
			IDXGIFactory* factory {nullptr};
			hr = this->dxgi_device->GetAdapter(&adapter);
			try {
				if (FAILED(hr)) {
					throw std::wstring {L"Obtaining the DXGI device adapter"};
				}
				hr = adapter->GetParent(IID_PPV_ARGS(&factory));
				if (FAILED(hr)) {
					throw std::wstring {L"Creation of DXGI device factory"};
				}
				hr = factory->CreateSwapChain(this->device, &scd, &this->swap_chain);
				if (FAILED(hr)) {
					throw std::wstring {L"Creation of Direct3D swap chain"};
				}
			}
			catch (const std::wstring& err) {
				// Make sure to release resources
				SafeRelease(&factory);
				SafeRelease(&adapter);
				winapi::handleWindowsError(err);
			}
			// Make sure to release resources
			SafeRelease(&factory);
			SafeRelease(&adapter);
			// Create and get back buffer
			hr = this->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&this->back_buffer));
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Back buffer obtaining");
			}
			hr = this->device->CreateRenderTargetView(this->back_buffer, nullptr, &this->render_target);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Render target view creation");
			}
			this->back_buffer->GetDesc(&this->back_buffer_desc);
			// Create depth-stencil buffer
			CD3D11_TEXTURE2D_DESC depth_stencil_desc {
				DXGI_FORMAT_D24_UNORM_S8_UINT,
				static_cast<UINT>(this->back_buffer_desc.Width),
				static_cast<UINT>(this->back_buffer_desc.Height),
				// Number of Textures and mipmap level and flags
				1, 1, D3D11_BIND_DEPTH_STENCIL
			};
			hr = this->device->CreateTexture2D(&depth_stencil_desc, nullptr, &this->depth_stencil);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Depth-stencil buffer creation");
			}
			// Create depth-stencil buffer view
			CD3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc {D3D11_DSV_DIMENSION_TEXTURE2D};
			hr = this->device->CreateDepthStencilView(this->depth_stencil, &depth_stencil_view_desc,
				&this->depth_stencil_view);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Depth-stencil view creation");
			}
			// Create viewport
			ZeroMemory(&this->viewport, sizeof(D3D11_VIEWPORT));
			this->viewport.Width = static_cast<float>(this->back_buffer_desc.Width);
			this->viewport.Height = static_cast<float>(this->back_buffer_desc.Height);
			this->viewport.MinDepth = 0;
			this->viewport.MaxDepth = 1;
			this->device_context->RSSetViewports(1, &this->viewport);
		}
	}
}