#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <deque>
#include <vector>
#include <string>
#include <optional>
#include <variant>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

using std::literals::string_view_literals::operator""sv;
using std::literals::string_literals::operator""s;

namespace svg
{
	class Rgb {
	public:
		Rgb() = default;

		Rgb(uint8_t red, uint8_t green, uint8_t blue)
			: red(red)
			, green(green)
			, blue(blue)
		{}

		uint8_t red = 0, green = 0, blue = 0;
	};

	class Rgba {
	public:
		Rgba() = default;

		Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
			: red(red)
			, green(green)
			, blue(blue)
			, opacity(opacity)
		{}

		uint8_t red = 0, green = 0, blue = 0;
		double opacity = 1.0;
	};

	using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

	// ������� � ������������ ����� ��������� �� �������������� inline,
	// �� ������� ���, ��� ��� ����� ����� �� ��� ������� ����������,
	// ������� ���������� ���� ���������.
	// � ��������� ������ ������ ������� ���������� ����� ������������ ���� ����� ���� ���������
	inline const Color NoneColor{ "none"s };

	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
	};

	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};

	std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);
	std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

	struct OstreamColorPrinter {
		std::ostream& out;

		int ToInt(uint8_t col) const
		{
			return static_cast<int>(col);
		}

		void operator()(std::monostate) const {
			out << "none"sv;
		}

		void operator()(std::string color) const {
			out << color;
		}

		void operator()(svg::Rgb rgb) const {
			out << "rgb("sv
				<< ToInt(rgb.red) << ","sv
				<< ToInt(rgb.green) << ","sv
				<< ToInt(rgb.blue) << ")"sv;
		}

		void operator()(svg::Rgba rgba) const {
			out << "rgba("sv
				<< ToInt(rgba.red) << ","sv
				<< ToInt(rgba.green) << ","sv
				<< ToInt(rgba.blue) << ","sv
				<< rgba.opacity << ")"sv;
		}
	};

	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(Color color) {
			fill_color_ = std::move(color);
			return AsOwner();
		}

		Owner& SetStrokeColor(Color color) {
			stroke_color_ = std::move(color);
			return AsOwner();
		}

		Owner& SetStrokeWidth(double stroke_width) {
			stroke_width_ = std::move(stroke_width);
			return AsOwner();
		}

		Owner& SetStrokeLineCap(StrokeLineCap stroke_line_cap) {
			stroke_line_cap_ = std::move(stroke_line_cap);
			return AsOwner();
		}

		Owner& SetStrokeLineJoin(StrokeLineJoin stroke_line_join) {
			stroke_line_join_ = std::move(stroke_line_join);
			return AsOwner();
		}

		virtual ~PathProps() = default;

	protected:
		void RenderAttrs(std::ostream& out) const {
			using namespace std::literals;

			if (fill_color_) {
				out << " fill=\""sv;
				std::visit(OstreamColorPrinter{ out }, fill_color_.value());
				out << "\""sv;
			}

			if (stroke_color_) {
				out << " stroke=\""sv;
				std::visit(OstreamColorPrinter{ out }, stroke_color_.value());
				out << "\""sv;
			}

			if (stroke_width_) {
				out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
			}

			if (stroke_line_cap_) {
				out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
			}

			if (stroke_line_join_) {
				out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
			}
		}

	private:
		Owner& AsOwner() {
			// static_cast ��������� ����������� *this � Owner&,
			// ���� ����� Owner � ��������� PathProps
			return static_cast<Owner&>(*this);
		}

		std::optional<Color> fill_color_;
		std::optional<Color> stroke_color_;
		std::optional<double> stroke_width_;
		std::optional<StrokeLineCap> stroke_line_cap_;
		std::optional<StrokeLineJoin> stroke_line_join_;
	};

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};

	/*
	 * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
	 * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
	 */
	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};

	/*
	 * ����������� ������� ����� Object ������ ��� ���������������� ��������
	 * ���������� ����� SVG-���������
	 * ��������� ������� "��������� �����" ��� ������ ����������� ����
	 */
	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;
	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	class ObjectContainer
	{
	public:
		/*
		 ����� Add ��������� � svg-�������� ����� ������-��������� svg::Object.
		 ������ �������������:
		 Document doc;
		 doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
		*/
		template <typename Obj>
		void Add(Obj obj)
		{
			_objects.emplace_back(std::make_unique<Obj>(std::move(obj)));
			_ptr_objects.emplace_back(_objects.back().get());
		}

		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

		virtual ~ObjectContainer() = default;

	protected:
		std::vector<const Object*> _ptr_objects;
		std::deque<std::unique_ptr<Object>> _objects;
	};

	class Document final : public ObjectContainer {
	public:
		// ��������� � svg-�������� ������-��������� svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj) override;

		// ������� � ostream svg-������������� ���������
		void Render(std::ostream& out) const;
	private:
	};

	// ��������� Drawable ����� �������, ������� ����� ���������� � ������� Graphics
	class Drawable {
	public:
		virtual void Draw(ObjectContainer& container) const = 0;
		virtual ~Drawable() = default;
	};

	/*
	 * ����� Circle ���������� ������� <circle> ��� ����������� �����
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/*
	 * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline final : public Object, public PathProps<Polyline> {
	public:
		// ��������� ��������� ������� � ������� �����
		Polyline& AddPoint(Point point);

	private:
		void RenderObject(const RenderContext& context) const override;

		std::vector<Point> points_;
	};

	/*
	 * ����� Text ���������� ������� <text> ��� ����������� ������
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text final : public Object, public PathProps<Text> {
	public:
		// ����� ���������� ������� ����� (�������� x � y)
		Text& SetPosition(Point pos);

		// ����� �������� ������������ ������� ����� (�������� dx, dy)
		Text& SetOffset(Point offset);

		// ����� ������� ������ (������� font-size)
		Text& SetFontSize(uint32_t size);

		// ����� �������� ������ (������� font-family)
		Text& SetFontFamily(std::string font_family);

		// ����� ������� ������ (������� font-weight)
		Text& SetFontWeight(std::string font_weight);

		// ����� ��������� ���������� ������� (������������ ������ ���� text)
		Text& SetData(std::string data);

	private:
		void DataTextToXML(std::ostream& out) const;

		void RenderObject(const RenderContext& context) const override;
		Point _position = { 0.0, 0.0 };
		Point _offset = { 0.0, 0.0 };
		uint32_t _size = 1;
		std::string _font_family;
		std::string _font_weight;
		std::string _data = ""s;
	};
}  // namespace svg

namespace shapes
{
	using std::literals::string_literals::operator""s;

	class Star final : public svg::Drawable {
	public:
		Star(svg::Point center,
			double outer_radius,
			double inner_radius,
			int num_rays)
			: _center(center)
			, _outer_radius(outer_radius)
			, _inner_radius(inner_radius)
			, _num_rays(num_rays)
		{}

		void Draw(svg::ObjectContainer& container) const override;

	private:
		svg::Point _center = { 0.0, 0.0 };
		double _outer_radius = 0.0;
		double _inner_radius = 0.0;
		int _num_rays = 0;
	};

	class Snowman final : public svg::Drawable {
	public:
		explicit Snowman(svg::Point head_center, double head_radius)
			: _head_center(head_center), _head_radius(head_radius)
		{
		}

		void Draw(svg::ObjectContainer& container) const override;

	private:
		svg::Point _head_center = { 0.0, 0.0 };
		double _head_radius = 0.0;
	};

	class Triangle : public svg::Drawable {
	public:
		Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
			: _p1(p1)
			, _p2(p2)
			, _p3(p3)
		{}

		void Draw(svg::ObjectContainer& container) const override;

	private:
		svg::Point _p1, _p2, _p3;
	};
}	// namespace shapes