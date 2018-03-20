#pragma once
// File Author: Isaiah Hoffman
// File Created: March 17, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <memory>
#include <array>
#include "./graphics_DX.hpp"

namespace hoffman::isaiah {
	namespace graphics {
		class ShapeBase {
		public:
			/// <summary>Virtual destructor that releases COM objects.</summary>
			virtual ~ShapeBase() noexcept {
				SafeRelease(&this->index_buffer);
				SafeRelease(&this->vertex_buffer);
			}
		protected:
			ShapeBase(std::shared_ptr<DX::DeviceResources> dev_res) :
				device_resources {dev_res} {
			}
		private:
			/// <summary>Pointer to the device resources in use.</summary>
			std::shared_ptr<DX::DeviceResources> device_resources;
			/// <summary>Pointer to the vertex buffer that defines this shape.</summary>
			ID3D11Buffer* vertex_buffer {nullptr};
			/// <summary>Pointer to the index buffer that defines this shape.</summary>
			ID3D11Buffer* index_buffer {nullptr};
		};

		class ShapeCube : public ShapeBase {
		public:
			ShapeCube(std::shared_ptr<DX::DeviceResources>, std::array<float, 3> scaling);
		private:
		};
	}
}