// File Author: Isaiah Hoffman
// File Created: May 15, 2018
#include <memory>
#include <array>
#include <cmath>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./game_object.hpp"

namespace hoffman::isaiah {
	namespace game {
		GameObject::GameObject(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res, graphics::shapes::ShapeTypes st,
			graphics::Color o_color, graphics::Color f_color, double cgx, double cgy, double gw, double gh) :
			sprite {nullptr},
			gx {cgx},
			gy {cgy} {
			const float csx = static_cast<float>(graphics::convertToScreenX(cgx));
			const float csy = static_cast<float>(graphics::convertToScreenY(cgy));
			float sw = static_cast<float>(gw) * graphics::getGameSquareWidth<float>();
			float sh = static_cast<float>(gh) * graphics::getGameSquareHeight<float>();
			if (st == graphics::shapes::ShapeTypes::Star) {
				// Swap sw and sh due to the 90 degree rotation performed
				std::swap(sw, sh);
			}

			switch (st) {
			case graphics::shapes::ShapeTypes::Star:
			{
				const std::array<std::array<float, 2>, 10> points {{
					{csx + sw / 2.f, csy},
					{csx + sw / 4.f * std::cos(0.2f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(0.2f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(0.4f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(0.4f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(0.6f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(0.6f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(0.8f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(0.8f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(1.0f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(1.0f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(1.2f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(1.2f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(1.4f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(1.4f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(1.6f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(1.6f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(1.8f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(1.8f * math::calculate_pi<float>())}
				}};
				this->sprite = std::make_unique<graphics::shapes::Shape2DPolygon<10>>(dev_res, o_color, f_color,
					points, csx, csy);
				this->rotate(math::calculate_pi<float>() * -0.5f);
				break;
			}
			case graphics::shapes::ShapeTypes::Diamond:
				this->sprite = std::make_unique<graphics::shapes::Shape2DDiamond>(dev_res, o_color, f_color,
					csx, csy, sw, sh);
				break;
			case graphics::shapes::ShapeTypes::Rectangle:
				this->sprite = std::make_unique<graphics::shapes::Shape2DRectangle>(dev_res, o_color, f_color,
					csx - sw / 2.f, csy - sh / 2.f, csx + sw / 2.f, csy + sh / 2.f);
				break;
			case graphics::shapes::ShapeTypes::Triangle:
			{
				this->sprite = std::make_unique<graphics::shapes::Shape2DTriangle>(dev_res, o_color, f_color,
					csx - sw / 2.f, csy - sh / 2.f, csx, csy + sh / 2.f, csx + sw / 2.f, csy - sh / 2.f);
				break;
			}
			case graphics::shapes::ShapeTypes::Ellipse:
			default:
				this->sprite = std::make_unique<graphics::shapes::Shape2DEllipse>(dev_res, o_color, f_color,
					csx, csy, sw, sh);
				break;
			}
		}
	}
}