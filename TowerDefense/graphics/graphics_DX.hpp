#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <string>

#pragma comment(lib, "d2d1.lib")

namespace hoffman::isaiah {
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
		/// <summary>Class that stores resources for 2D graphics (and text).</summary>
		class DeviceResources2D {
		public:
			/// <summary>Default constructor that does nothing.</summary>
			DeviceResources2D() noexcept = default;
			/// <summary>Releases all COM resources owned by this class.</summary>
			~DeviceResources2D() noexcept {
				this->discardDeviceResources();
				SafeRelease(&this->factory);
			}
			/// <summary>Creates resources that are independent of the underlying devices.</summary>
			void createDeviceIndependentResources();
			/// <summary>Create resources that are dependent upon the underlying device.</summary>
			/// <param name="hwnd">Handle to the window that the resources are associated with.</param>
			/// <returns>A value indicating whether or not the resources were successfully created.</returns>
			HRESULT createDeviceResources(HWND hwnd);
			/// <summary>Releases the rendering target and associated brushes that are device-dependent resources.</summary>
			void discardDeviceResources() noexcept {
				SafeRelease(&this->text_brush);
				SafeRelease(&this->fill_brush);
				SafeRelease(&this->outline_brush);
				SafeRelease(&this->render_target);
			}

			// Getters
			// (I plan on performing non-constant actions on these
			// returns, so these getters are not marked constant.)
			ID2D1HwndRenderTarget* getRenderTarget() noexcept {
				return this->render_target;
			}
			ID2D1SolidColorBrush* getOutlineBrush() noexcept {
				return this->outline_brush;
			}
			ID2D1SolidColorBrush* getFillBrush() noexcept {
				return this->fill_brush;
			}
			ID2D1SolidColorBrush* getTextBrush() noexcept {
				return this->text_brush;
			}
		private:
			/// <summary>Pointer to the Direct2D factory.</summary>
			ID2D1Factory* factory {nullptr};
			/// <summary>Pointer to the Direct2D rendering target.</summary>
			ID2D1HwndRenderTarget* render_target {nullptr};
			/// <summary>Pointer to the Direct2D brush used for outlining shapes.</summary>
			ID2D1SolidColorBrush* outline_brush {nullptr};
			/// <summary>Pointer to the Direct2D brush used for filling shapes.</summary>
			ID2D1SolidColorBrush* fill_brush {nullptr};
			/// <summary>Pointer to the Direct2D brush used for text.</summary>
			ID2D1SolidColorBrush* text_brush {nullptr};
		};
	}
}