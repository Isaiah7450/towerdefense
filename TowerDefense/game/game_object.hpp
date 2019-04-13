#pragma once
// File Author: Isaiah Hoffman
// File Created: May 15, 2018
#include <memory>
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./../pathfinding/grid.hpp"

namespace hoffman::isaiah {
	namespace game {
		/// <summary>Base class for game objects that are drawn on the screen.</summary>
		class GameObject : public graphics::IDrawable {
		public:
			GameObject(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res, const GameMap& game_map, graphics::shapes::ShapeTypes st,
				graphics::Color o_color, graphics::Color f_color, double cgx, double cgy, double gw, double gh);

			// Implements graphics::Drawable::draw()
			void draw(const graphics::Renderer2D& renderer) const noexcept override {
				this->sprite->draw(renderer);
			}

			bool checkHit(float sx, float sy) const noexcept {
				return this->sprite->checkHit(sx, sy);
			}
			bool intersects(const GameObject& other) const noexcept {
				return this->sprite->intersect(other.sprite.get());
			}

			// Setters and Changers
			void translate(double dgx, double dgy) {
				this->changeGameX(dgx);
				this->changeGameY(dgy);
				this->sprite->change_translate(static_cast<float>(dgx) * this->my_map.getGameSquareWidth<float>(), 0.f);
				this->sprite->change_translate(0.f, static_cast<float>(dgy) * this->my_map.getGameSquareHeight<float>());
			}
			void rotate(float new_theta) {
				this->sprite->change_rotation(new_theta);
			}

			// Getters
			/// <returns>Returns the game x-coordinate of the object.</returns>
			double getGameX() const noexcept {
				return this->gx;
			}
			/// <returns>Returns the game y-coordinate of the object.</returns>
			double getGameY() const noexcept {
				return this->gy;
			}
			/// <returns>Returns the screen x-coordinate of the object.</returns>
			double getScreenX() const noexcept {
				return this->getGameMap().convertToScreenX(this->getGameX());
			}
			/// <param name="my_map">Reference to the game map.</param>
			/// <returns>Returns the screen y-coordinate of the object.</returns>
			double getScreenY() const noexcept {
				return this->getGameMap().convertToScreenY(this->getGameY());
			}
			const GameMap& getGameMap() const noexcept {
				return this->my_map;
			}
		protected:
			// Setters and Changers
			// Note that these don't perform actual changes to the graphical representation.
			void changeGameX(double dgx) {
				this->gx += dgx;
			}
			void changeGameY(double dgy) {
				this->gy += dgy;
			}
		private:
			/// <summary>The graphical representation of the object.</summary>
			std::unique_ptr<graphics::shapes::Shape2DBase> sprite;
			/// <summary>Reference to the game map.</summary>
			const GameMap& my_map;
			/// <summary>The object's game x-coordinate.</summary>
			double gx;
			/// <summary>The object's game y-coordinate.</summary>
			double gy;
		};
	}
}