#pragma once
// File Author: Isaiah Hoffman
// File Created: March 13, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <memory>
#include "./graphics_DX.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declaration
		class MyGame;
	}

	namespace graphics {
		/// <summary>Class that handles rendering of 2D elements.</summary>
		class Renderer2D {
		public:
			Renderer2D(std::shared_ptr<DX::DeviceResources2D> dev_res) :
				device_resources {dev_res} {
			}
			
			/// <summary>Draws the current scene based on the game state.</summary>
			/// <param name="my_game">Shared pointer to object that contains the current game state.</param>
			HRESULT render(const game::MyGame& my_game) const;
		protected:
		private:
			std::shared_ptr<DX::DeviceResources2D> device_resources;
		};
	}
}