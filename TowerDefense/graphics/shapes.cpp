// File Author: Isaiah Hoffman
// File Created: March 17, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <d2d1.h>
#include <memory>
#include <array>
#include "./../globals.hpp"
#include "./../ih_math.hpp"
#include "./graphics_DX.hpp"
#include "./graphics.hpp"
#include "./shapes.hpp"

namespace hoffman::isaiah {
	namespace graphics::shapes {
		void Shape2DBase::draw(const Renderer2D& renderer) const noexcept {
			renderer.drawGeometry(this->transformed_geometry, this->outline_color);
			renderer.fillGeometry(this->transformed_geometry, this->fill_color);
		}

		void Shape2DBase::recreateGeometry() {
			const auto my_translate = D2D1::Matrix3x2F::Translation({this->h_translate, this->v_translate});
			const auto my_rotate = D2D1::Matrix3x2F::Rotation(math::convert_to_degrees<float>(this->theta),
				D2D1::Point2F(this->center_sx, this->center_sy));
			const auto my_scale = D2D1::Matrix3x2F::Scale({this->h_scale, this->v_scale});
			const auto my_transform = my_rotate * my_translate * my_scale;
			// Release old geometry
			graphics::SafeRelease(&this->transformed_geometry);
			// Create new geometry
			this->device_resources->getFactory()->CreateTransformedGeometry(this->path_geometry,
				my_transform, &this->transformed_geometry);
		}

		bool Shape2DBase::intersect(const Shape2DBase& other_shape) const noexcept {
			D2D1_GEOMETRY_RELATION relationship {};
			HRESULT hr = this->transformed_geometry->CompareWithGeometry(
				other_shape.transformed_geometry, D2D1::Matrix3x2F::Identity(), &relationship);
			return SUCCEEDED(hr) && relationship > D2D1_GEOMETRY_RELATION::D2D1_GEOMETRY_RELATION_DISJOINT;
		}

		Shape2DEllipse::Shape2DEllipse(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
			float csx, float csy, float sw, float sh) :
			Shape2DBase {dev_res, o_color, f_color, csx, csy} {
			ID2D1GeometrySink* geom_sink {nullptr};
			HRESULT hr = this->path_geometry->Open(&geom_sink);
			if (FAILED(hr)) {
				throw std::runtime_error {"Could not open geometry sink to define shape."};
			}
			geom_sink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);
			geom_sink->BeginFigure(D2D1::Point2F(csx + sw / 2.f, csy), D2D1_FIGURE_BEGIN_FILLED);
			geom_sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(csx - sw / 2.f, csy), D2D1::SizeF(sw / 2.f, sh / 2.f),
				180.f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			geom_sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(csx + sw / 2.f, csy), D2D1::SizeF(sw / 2.f, sh / 2.f),
				180.f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			geom_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = geom_sink->Close();
			SafeRelease(&geom_sink);
			this->recreateGeometry();
		}

		Shape2DTriangle::Shape2DTriangle(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
			float sx1, float sy1, float sx2, float sy2, float sx3, float sy3) :
			Shape2DBase {dev_res, o_color, f_color,
				math::get_avg(math::get_min(sx1, sx2, sx3), math::get_max(sx1, sx2, sx3)),
				math::get_avg(math::get_min(sy1, sy2, sy3), math::get_max(sy1, sy2, sy3))} {
			ID2D1GeometrySink* geom_sink {nullptr};
			HRESULT hr = this->path_geometry->Open(&geom_sink);
			if (FAILED(hr)) {
				throw std::runtime_error {"Could not open geometry sink to define shape."};
			}
			geom_sink->BeginFigure(D2D1::Point2F(sx1, sy1), D2D1_FIGURE_BEGIN_FILLED);
			geom_sink->AddLine(D2D1::Point2F(sx2, sy2));
			geom_sink->AddLine(D2D1::Point2F(sx3, sy3));
			geom_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = geom_sink->Close();
			SafeRelease(&geom_sink);
			this->recreateGeometry();
		}

		Shape2DRectangle::Shape2DRectangle(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
			float lsx, float tsy, float rsx, float bsy) :
			Shape2DBase {dev_res, o_color, f_color, (lsx + rsx) / 2.f, (tsy + bsy) / 2.f} {
			ID2D1GeometrySink* geom_sink {nullptr};
			HRESULT hr = this->path_geometry->Open(&geom_sink);
			if (FAILED(hr)) {
				throw std::runtime_error {"Could not open geometry sink to define shape."};
			}
			geom_sink->BeginFigure(D2D1::Point2F(lsx, tsy), D2D1_FIGURE_BEGIN_FILLED);
			geom_sink->AddLine(D2D1::Point2F(lsx, bsy));
			geom_sink->AddLine(D2D1::Point2F(rsx, bsy));
			geom_sink->AddLine(D2D1::Point2F(rsx, tsy));
			geom_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = geom_sink->Close();
			SafeRelease(&geom_sink);
			this->recreateGeometry();
		}


		Shape2DDiamond::Shape2DDiamond(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
			float csx, float csy, float sw, float sh) :
			Shape2DBase {dev_res, o_color, f_color, csx, csy} {
			ID2D1GeometrySink* geom_sink {nullptr};
			HRESULT hr = this->path_geometry->Open(&geom_sink);
			if (FAILED(hr)) {
				throw std::runtime_error {"Could not open geometry sink to define shape."};
			}
			geom_sink->BeginFigure(D2D1::Point2F(csx + sw / 2.f, csy), D2D1_FIGURE_BEGIN_FILLED);
			geom_sink->AddLine(D2D1::Point2F(csx, csy - sh / 2.f));
			geom_sink->AddLine(D2D1::Point2F(csx - sw / 2.f, csy));
			geom_sink->AddLine(D2D1::Point2F(csx, csy + sh / 2.f));
			geom_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = geom_sink->Close();
			SafeRelease(&geom_sink);
			this->recreateGeometry();
		}
	}
}