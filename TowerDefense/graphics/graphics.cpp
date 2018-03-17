// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <ppl.h>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include "./graphics.hpp"
#include "./graphics_DX.hpp"

namespace hoffman::isaiah {
	namespace graphics::DX {
		Renderer::Renderer(std::shared_ptr<DX::DeviceResources> dev_res) :
			device_resources {dev_res} {
		}

		void Renderer::createDeviceDependentResources() {
			try {
				this->createShaders();
				this->createCube();
			}
			catch (const std::wstring& e) {
				MessageBox(nullptr, e.c_str(), L"Error", MB_OK);
				ExitProcess(1);
			}
		}

		void Renderer::createWindowSizeDependentResources() {
			this->createViewAndPerspective();
		}

		void Renderer::createShaders() {
			auto* device = this->device_resources->getDevice();
			constexpr const size_t dest_size {4096 * 50};
			DWORD bytes_read {0};
			std::unique_ptr<BYTE[]> bytes = std::make_unique<BYTE[]>(dest_size);
			// Load vertex shader bytecode
			HANDLE my_shader = CreateFile(L"vertex_shader.cso", GENERIC_READ, FILE_SHARE_READ,
				nullptr, OPEN_EXISTING, 0, nullptr);
			if (my_shader == INVALID_HANDLE_VALUE) {
				winapi::handleWindowsError(L"Opening of vertex_shader.cso");
			}
			if (!ReadFile(my_shader, bytes.get(), dest_size, &bytes_read, nullptr)) {
				CloseHandle(my_shader);
				winapi::handleWindowsError(L"Reading of vertex_shader.cso");
			}
			CloseHandle(my_shader);
			HRESULT hr = device->CreateVertexShader(bytes.get(), bytes_read, nullptr, &this->vertex_shader);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Vertex shader creation");
			}
			// Create input layout
			D3D11_INPUT_ELEMENT_DESC ia_desc[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};
			hr = device->CreateInputLayout(ia_desc, ARRAYSIZE(ia_desc), bytes.get(), bytes_read, &this->input_layout);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Input layout creation");
			}
			// Load pixel shader bytecode
			my_shader = CreateFile(L"pixel_shader.cso", GENERIC_READ, FILE_SHARE_READ,
				nullptr, OPEN_EXISTING, 0, nullptr);
			if (my_shader == INVALID_HANDLE_VALUE) {
				winapi::handleWindowsError(L"Opening of pixel_shader.cso");
			}
			if (!ReadFile(my_shader, bytes.get(), dest_size, &bytes_read, nullptr)) {
				CloseHandle(my_shader);
				winapi::handleWindowsError(L"Reading of pixel_shader.cso");
			}
			CloseHandle(my_shader);
			hr = device->CreatePixelShader(bytes.get(), bytes_read, nullptr, &this->pixel_shader);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Pixel shader creation");
			}
			CD3D11_BUFFER_DESC cb_desc {sizeof(ConstantBufferStruct), D3D11_BIND_CONSTANT_BUFFER};
			hr = device->CreateBuffer(&cb_desc, nullptr, &this->constant_buffer);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Constant buffer creation");
			}
		}

		void Renderer::createCube() {
			ID3D11Device* device = this->device_resources->getDevice();
			// Create cube geometry
			VertexPositionColor cube_vertices[] = {
				{DirectX::XMFLOAT3 {-0.5f, -0.5f, -0.5f}, DirectX::XMFLOAT3 {0.f, 0.f, 0.f},},
				{DirectX::XMFLOAT3 {-0.5f, -0.5f, 0.5f}, DirectX::XMFLOAT3 {0.f, 0.f, 1.f},},
				{DirectX::XMFLOAT3 {-0.5f, 0.5f, -0.5f}, DirectX::XMFLOAT3 {0.f, 1.f, 0.f},},
				{DirectX::XMFLOAT3 {-0.5f, 0.5f, 0.5f}, DirectX::XMFLOAT3 {0.f, 1.f, 1.f},},
				{DirectX::XMFLOAT3 {0.5f, -0.5f, -0.5f}, DirectX::XMFLOAT3 {1.f, 0.f, 0.f},},
				{DirectX::XMFLOAT3 {0.5f, -0.5f, 0.5f}, DirectX::XMFLOAT3 {1.f, 0.f, 1.f},},
				{DirectX::XMFLOAT3 {0.5f, 0.5f, -0.5f}, DirectX::XMFLOAT3 {1.f, 1.f, 0.f},},
				{DirectX::XMFLOAT3 {0.5f, 0.5f, 0.5f}, DirectX::XMFLOAT3 {1.f, 1.f, 1.f},}
			};
			// Create vertex buffer
			CD3D11_BUFFER_DESC v_desc {sizeof(cube_vertices), D3D11_BIND_VERTEX_BUFFER};
			D3D11_SUBRESOURCE_DATA v_data;
			ZeroMemory(&v_data, sizeof(D3D11_SUBRESOURCE_DATA));
			v_data.pSysMem = cube_vertices;
			v_data.SysMemPitch = 0;
			v_data.SysMemSlicePitch = 0;
			HRESULT hr = device->CreateBuffer(&v_desc, &v_data, &this->vertex_buffer);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Vertex buffer creation");
			}
			// Create index buffer
			unsigned short cube_indices[] = {
				0, 2, 1, // -x
				1, 2, 3,
				4, 5, 6, // +x
				5, 7, 6,
				0, 1, 5, // -y
				0, 5, 4,
				2, 6, 7, // +y
				2, 7, 3,
				0, 4, 6, // -z
				0, 6, 2,
				1, 3, 7, // +z
				1, 7, 5
			};
			this->index_count = ARRAYSIZE(cube_indices);
			CD3D11_BUFFER_DESC i_desc {sizeof(cube_indices), D3D11_BIND_INDEX_BUFFER};
			D3D11_SUBRESOURCE_DATA i_data {cube_indices, 0, 0};
			hr = device->CreateBuffer(&i_desc, &i_data, &this->index_buffer);
			if (FAILED(hr)) {
				winapi::handleWindowsError(L"Index buffer creation");
			}
		}

		void Renderer::createViewAndPerspective() {
			// Use DirectXMath to create view and perspective matrices.
			DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.7f, 1.5f, 0.f);
			DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, -0.1f, 0.0f, 0.f);
			DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f);
			DirectX::XMStoreFloat4x4(&this->constant_buffer_data.view,
				DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(eye, at, up)));
			const float aspect_ratio = this->device_resources->getAspectRatio();
			DirectX::XMStoreFloat4x4(&this->constant_buffer_data.projection,
				DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovRH(DirectX::XMConvertToRadians(70),
					aspect_ratio, 0.01f, 100.0f)));
		}

		void Renderer::update() {
			// Rotate the cube 1 degree per frame
			DirectX::XMStoreFloat4x4(&this->constant_buffer_data.world,
				DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(
					DirectX::XMConvertToRadians(static_cast<float>(++this->frame_count)))));
			if (this->frame_count == MAXUINT) {
				this->frame_count = 0;
			}
		}

		void Renderer::render() {
			auto* context = this->device_resources->getDeviceContext();
			auto* render_target = this->device_resources->getRenderTargetView();
			auto* depth_stencil = this->device_resources->getDepthStencilView();
			context->UpdateSubresource(this->constant_buffer, 0, nullptr,
				&this->constant_buffer_data, 0, 0);
			// Clear render target and z-buffer
			constexpr const float background_color[] = {0.0f, 0.0f, 0.0f, 1.0f};
			context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				1.0f, 0);
			// Set render target
			context->OMSetRenderTargets(1, &render_target, depth_stencil);
			// Set up the IA stage by setting input topology and layout
			UINT stride = sizeof(VertexPositionColor);
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, &this->vertex_buffer, &stride, &offset);
			context->IASetIndexBuffer(this->index_buffer, DXGI_FORMAT_R16_UINT, 0);
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->IASetInputLayout(this->input_layout);
			// Set up the vertex shader stage
			context->VSSetShader(this->vertex_shader, nullptr, 0);
			context->VSSetConstantBuffers(0, 1, &this->constant_buffer);
			// Set up the pixel shader stage
			context->PSSetShader(this->pixel_shader, nullptr, 0);
			// Tell Direct3D to start sending commands to the graphics device
			context->DrawIndexed(this->index_count, 0, 0);
		}
	}
}