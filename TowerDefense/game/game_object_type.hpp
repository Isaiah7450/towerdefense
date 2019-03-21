#pragma once
// File Author: Isaiah Hoffman
// File Created: April 18, 2018
#include <string>
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"

namespace hoffman::isaiah {
	namespace game {
		/// <summary>Base class from which all game object template types are derived.</summary>
		class GameObjectType {
		public:
			// Use default destructor... But make it virtual since we're inheriting...
			virtual ~GameObjectType() noexcept = default;
			// Rule of 5.
			GameObjectType(const GameObjectType&) noexcept = delete;
			GameObjectType(GameObjectType&&) noexcept = default;
			GameObjectType& operator=(const GameObjectType&) noexcept = delete;
			GameObjectType& operator=(GameObjectType&&) noexcept = default;
			// Getters
			std::wstring getName() const noexcept {
				return this->name;
			}
			std::wstring getDesc() const noexcept {
				return this->desc;
			}
			graphics::Color getColor() const noexcept {
				return this->color;
			}
			graphics::shapes::ShapeTypes getShape() const noexcept {
				return this->shape;
			}
		protected:
			GameObjectType(std::wstring n, std::wstring d, graphics::Color c, graphics::shapes::ShapeTypes st) :
				name {n},
				desc {d},
				color {c},
				shape {st} {
			}
		private:
			/// <summary>The name of the object.</summary>
			std::wstring name;
			/// <summary>The description of the object.</summary>
			std::wstring desc;
			/// <summary>The default color to use for the object.</summary>
			graphics::Color color;
			/// <summary>The default shape to use for the object.</summary>
			graphics::shapes::ShapeTypes shape;
		};
	}
}