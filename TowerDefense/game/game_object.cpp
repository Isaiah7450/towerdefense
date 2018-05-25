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

			switch (st) {
			case graphics::shapes::ShapeTypes::Star:
			{
				const std::array<std::array<float, 2>, 10> points {{
					{csx, csy + sh / 2.f},
					{csx + sw / 4.f * std::cos(0.7f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(0.7f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(0.9f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(0.9f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(1.1f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(1.1f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(1.3f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(1.3f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(1.5f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(1.5f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(1.7f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(1.7f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(1.9f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(1.9f * math::calculate_pi<float>())},
					{csx + sw / 2.f * std::cos(2.1f * math::calculate_pi<float>()),
					csy + sh / 2.f * std::sin(2.1f * math::calculate_pi<float>())},
					{csx + sw / 4.f * std::cos(2.3f * math::calculate_pi<float>()),
					csy + sh / 4.f * std::sin(2.3f * math::calculate_pi<float>())}
				}};
				this->sprite = std::make_unique<graphics::shapes::Shape2DPolygon<10>>(dev_res, o_color, f_color,
					points, csx, csy);
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