#pragma once
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

namespace hoffman::isaiah {
	namespace graphics::shapes {
		enum class ShapeTypes {
			Ellipse, Triangle, Rectangle, Diamond, Star
		};

		/// <summary>Abstract base class for all shapes.</summary>
		class Shape2DBase : public graphics::IDrawable {
		public:
			/// <summary>Virtual destructor that releases COM objects.</summary>
			virtual ~Shape2DBase() noexcept {
				SafeRelease(&this->path_geometry);
				SafeRelease(&this->transformed_geometry);
			}
			// (Note: Derived classes can override the default implementation
			// given in this class if necessary.)
			// Overrides IDrawable::draw
			void draw(const Renderer2D& renderer) const noexcept override;
			// Hit testing and similar stuff
			/// <summary>Determines if the given point is within the shape's interior.</summary>
			/// <param name="sx">The screen x-coordinate of the point to test.</param>
			/// <param name="sy">The screen y-coordinate of the point to test.</param>
			/// <returns>True if the given point is inside the geometry; otherwise, false.</returns>
			bool checkHit(float sx, float sy) const noexcept {
				BOOL result = false;
				HRESULT hr = this->transformed_geometry->FillContainsPoint(D2D1::Point2F(sx, sy),
					D2D1::Matrix3x2F::Identity(), &result);
				return SUCCEEDED(hr) && result;
			}
			/// <summary>Determines if two shapes have any points in common.</summary>
			/// <param name="other_shape">The shape to compare this shape to.</param>
			/// <returns>True if the two shapes are not disjoint.</returns>
			bool intersect(const Shape2DBase& other_shape) const noexcept;
			// Overload on pointer
			bool intersect(const Shape2DBase* other_shape) const noexcept {
				return this->intersect(*other_shape);
			}
			// Transformation Setters
			/// <param name="dsx">The change in the horizontal translation.</param>
			/// <param name="dsy">The change in the vertical translation.</param>
			/// <param name="new_theta">The new rotation angle in radians from the original position.</param>
			/// <param name="new_hscale">The new horizontal scale factor to apply (with 1.0 = original size).</param>
			/// <param name="new_vscale">The new vertical scale factor to apply (with 1.0 = original size).</param>
			void change_transform(float dsx, float dsy, float new_theta, float new_hscale, float new_vscale) {
				this->h_translate += dsx;
				this->v_translate += dsy;
				this->theta = new_theta;
				this->h_scale = new_hscale;
				this->v_scale = new_vscale;
				this->recreateGeometry();
			}
			/// <summary>Translates the figure in the x and y coordinates from the original position.</summary>
			/// <param name="dsx">The change in the horizontal translation.</param>
			/// <param name="dsy">The change in the vertical translation.</param>
			void change_translate(float dsx, float dsy) {
				this->change_transform(dsx, dsy, this->theta, this->h_scale, this->v_scale);
			}
			/// <summary>Changes the rotation of the shape.</summary>
			/// <param name="new_theta">The new rotation angle in radians. Note that this value is relative to the original.</param>
			void change_rotation(float new_theta) {
				this->change_transform(this->h_translate, this->v_translate, new_theta, this->h_scale, this->v_scale);
			}
			/// <summary>Changes the scale of the shape.</summary>
			/// <param name="new_hscale">The new horizontal scale factor to apply (with 1.0 = original size).</param>
			/// <param name="new_vscale">The new vertical scale factor to apply (with 1.0 = original size).</param>
			void change_scale(float new_hscale, float new_vscale) {
				this->change_transform(this->h_translate, this->v_translate, this->theta, new_hscale, new_vscale);
			}
		protected:
			Shape2DBase(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color, float csx, float csy) :
				device_resources {dev_res},
				outline_color {o_color},
				fill_color {f_color},
				center_sx {csx},
				center_sy {csy} {
				HRESULT hr = this->device_resources->getFactory()->CreatePathGeometry(&this->path_geometry);
				if (FAILED(hr)) {
					throw std::runtime_error {"Creation of shape geometry failed!"};
				}
			}
			/// <summary>Recreates the geometry using the current transformation values.</summary>
			void recreateGeometry();

			// (Really no reason to keep this all private.)
			/// <summary>Pointer to the device resources in use.</summary>
			std::shared_ptr<DX::DeviceResources2D> device_resources;
			/// <summary>The color to use to outline the shape.</summary>
			Color outline_color;
			/// <summary>The color to use to fill in the interior of the shape.</summary>
			Color fill_color;
			/// <summary>Pointer to the original path geometry.</summary>
			ID2D1PathGeometry* path_geometry {nullptr};
			/// <summary>Pointer to the geometry after applying transformations.</summary>
			ID2D1TransformedGeometry* transformed_geometry {nullptr};
			/// <summary>The screen x-coordinate of the approximate center of the geometry.</summary>
			float center_sx;
			/// <summary>The screen y-coordinate of the approximate center of the geometry.</summary>
			float center_sy;
			// Note: Transformation order is translate first, then scale, then rotate last.
			/// <summary>Amount by which to translate the geometry horizontally by.</summary>
			float h_translate {0.f};
			/// <summary>Amount by which to translate the geometry vertically by.</summary>
			float v_translate {0.f};
			/// <summary>Radian measure to rotate the geometry by (with 0 = no rotation).</summary>
			float theta {0.f};
			/// <summary>Horizontal scale factor (with 1.0 = original size).</summary>
			float h_scale {1.f};
			/// <summary>Vertical scale factor (with 1.0 = original size).</summary>
			float v_scale {1.f};
		};

		/// <summary>Shape that represents an ellipse.</summary>
		class Shape2DEllipse : public Shape2DBase {
		public:
			/// <param name="csx">The screen x-coordinate of the center of the ellipse.</param>
			/// <param name="csy">The screen y-coordinate of the center of the ellipse.</param>
			/// <param name="sw">The width in screen coordinates of the ellipse.</param>
			/// <param name="sh">The height in screen coordinates of the ellipse.</param>
			Shape2DEllipse(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
				float csx, float csy, float sw, float sh);
		};

		// Note that Triangle, Rectangle, Diamond, and Star
		// exist for convenience and could be also represented using
		// Polygon.
		/// <summary>Shape that represents a triangle.</summary>
		class Shape2DTriangle : public Shape2DBase {
		public:
			/// <param name="sx1">The screen x-coordinate of the first point.</param>
			/// <param name="sy1">The screen y-coordinate of the first point.</param>
			/// <param name="sx2">The screen x-coordinate of the second point.</param>
			/// <param name="sy2">The screen y-coordinate of the second point.</param>
			/// <param name="sx3">The screen x-coordinate of the third point.</param>
			/// <param name="sy3">The screen y-coordinate of the third point.</param>
			Shape2DTriangle(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
				float sx1, float sy1, float sx2, float sy2, float sx3, float sy3);
		};

		/// <summary>Shape that represents a rectangle.</summary>
		class Shape2DRectangle : public Shape2DBase {
		public:
			/// <param name="lsx">The screen x-coordinate of the left-most part of the rectangle.</param>
			/// <param name="tsy">The screen y-coordinate of the top-most part of the rectangle.</param>
			/// <param name="rsx">The screen x-coordinate of the right-most part of the rectangle.</param>
			/// <param name="bsy">The screen y-coordinate of the bottom-most part of the rectangle.</param>
			Shape2DRectangle(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
				float lsx, float tsy, float rsx, float bsy);
		};

		/// <summary>Shape that represents a diamond.</summary>
		class Shape2DDiamond : public Shape2DBase {
		public:
			/// <param name="lsx">The screen x-coordinate of the center of the diamond.</param>
			/// <param name="tsy">The screen y-coordinate of the center of the diamond.</param>
			/// <param name="rsx">The width in screen coordinates of the diamond.</param>
			/// <param name="bsy">The height in screen coordinates of the diamond.</param>
			Shape2DDiamond(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
				float csx, float csy, float sw, float sh);
		};
		// Represents a N-sided polygon
		// N --> The number of sides of the polygon
		template <int N>
		class Shape2DPolygon : public Shape2DBase {
		public:
			/// <param name="points">An array of points containing the screen x and screen y coordinates of
			/// each point in the polygon.</param>
			/// <param name="csx">The screen x-coordinate of the polygon's center. Note that invalid values
			/// could result in transformations not working properly.</param>
			/// <param name="csy">The screen y-coordinate of the polygon's center. Note that invalid values
			/// could result in transformations not working properly.</param>
			Shape2DPolygon(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
				std::array<std::array<float, 2>, N> points, float csx, float csy);
		};

		// Templates must be defined in the same file as they are declared...
		template <int N>
		Shape2DPolygon<N>::Shape2DPolygon(std::shared_ptr<DX::DeviceResources2D> dev_res, Color o_color, Color f_color,
			std::array<std::array<float, 2>, N> points, float csx, float csy) :
			Shape2DBase {dev_res, o_color, f_color, csx, csy} {
			ID2D1GeometrySink* geom_sink {nullptr};
			HRESULT hr = this->path_geometry->Open(&geom_sink);
			if (FAILED(hr)) {
				throw std::runtime_error {"Could not open geometry sink to define shape."};
			}
			geom_sink->BeginFigure(D2D1::Point2F(points[0][0], points[0][1]), D2D1_FIGURE_BEGIN_FILLED);
			for (unsigned int i = 1; i < points.size(); ++i) {
				geom_sink->AddLine(D2D1::Point2F(points[i][0], points[i][1]));
			}
			geom_sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = geom_sink->Close();
			SafeRelease(&geom_sink);
			this->recreateGeometry();
		}
	}
}