#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "./graphics_DX.hpp"

namespace hoffman::isaiah {
	namespace graphics {
		/// <summary>Class used to render a scene.</summary>
		class Renderer {
		public:
			/// <summary>Constructs a new renderer that is used to render a scene.</summary>
			/// <param name="dev_res">Shared pointer to the device resources used by the program.</param>
			Renderer(std::shared_ptr<DX::DeviceResources> dev_res);
			/// <summary>Releases COM objects.</summary>
			~Renderer() noexcept {
				SafeRelease(&this->index_buffer);
				SafeRelease(&this->vertex_buffer);
				SafeRelease(&this->constant_buffer);
				SafeRelease(&this->pixel_shader);
				SafeRelease(&this->input_layout);
				SafeRelease(&this->vertex_shader);
			}
			/// <summary>Creates device dependent resources.</summary>
			void createDeviceDependentResources();
			/// <summary>Creates window size dependent resources.</summary>
			void createWindowSizeDependentResources();
			/// <summary>Updates the current scene.</summary>
			void update();
			/// <summary>Renders the current scene.</summary>
			void render();
		protected:
			/// <summary>Creates and loads necessary shaders.</summary>
			void createShaders();
			/// <summary>Updates the current view and perspective in response to a window view change.</summary>
			void createViewAndPerspective();
		private:
			/// <summary>Structure that contains variables for rendering the cube.</summary>
			struct ConstantBufferStruct {
				DirectX::XMFLOAT4X4 world;
				DirectX::XMFLOAT4X4 view;
				DirectX::XMFLOAT4X4 projection;
			};
			// Assert that the constant buffer remains 16-byte aligned.
			static_assert((sizeof(ConstantBufferStruct) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
			/// <summary>Structure containing per-vertex data.</summary>
			struct VertexPositionColor {
				DirectX::XMFLOAT3 pos;
				DirectX::XMFLOAT3 color;
			};
			/// <summary>Structure containing extended per-vertex data.</summary>
			struct VertexPositionColorTangent {
				DirectX::XMFLOAT3 pos;
				DirectX::XMFLOAT3 normal;
				DirectX::XMFLOAT3 tangent;
			};

			/// <summary>Shared pointer to the Direct3D device resources.</summary>
			std::shared_ptr<DX::DeviceResources> device_resources;
			/// <summary>Pointer to the vertex shader.</summary>
			ID3D11VertexShader* vertex_shader {nullptr};
			/// <summary>Pointer to the input layout.</summary>
			ID3D11InputLayout* input_layout {nullptr};
			/// <summary>Pointer to the pixel shader.</summary>
			ID3D11PixelShader* pixel_shader {nullptr};
			/// <summary>Pointer to the constant buffer.</summary>
			ID3D11Buffer* constant_buffer {nullptr};
			/// <summary>Pointer to the vertex buffer.</summary>
			ID3D11Buffer* vertex_buffer {nullptr};
			/// <summary>The number of indices of the cube.</summary>
			unsigned int index_count {0};
			/// <summary>Pointer to the index buffer.</summary>
			ID3D11Buffer* index_buffer {nullptr};
			/// <summary>Stores the current view and perspective.</summary>
			ConstantBufferStruct constant_buffer_data {};
			/// <summary>The current frame number.</summary>
			unsigned int frame_count {0};
		};
	}
}