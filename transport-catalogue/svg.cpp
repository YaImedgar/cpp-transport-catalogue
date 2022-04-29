#include "svg.h"

namespace svg
{
	using namespace std::literals;

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}

	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\" "sv;

		RenderAttrs(context.out);
		out << "/>"sv;
	}

	// ---------- Document ------------------

	void Document::AddPtr(std::unique_ptr<Object>&& obj)
	{
		_objects.emplace_back(std::move(obj));
		_ptr_objects.emplace_back(_objects.back().get());
	}

	void Document::Render(std::ostream& out) const
	{
		RenderContext rendCont(out, 0, 4);
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl
			<< "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
		for (const Object* obj : _ptr_objects)
		{
			obj->Render(rendCont);
		}
		out << "</svg>"sv << std::endl;
	}

	// ---------- Text ------------------

	Text& Text::SetPosition(Point pos)
	{
		_position = pos;
		return *this;
	}

	Text& Text::SetOffset(Point offset)
	{
		_offset = offset;
		return *this;
	}

	Text& Text::SetFontSize(uint32_t size)
	{
		_size = size;
		return *this;
	}

	Text& Text::SetFontFamily(std::string font_family)
	{
		_font_family = font_family;
		return *this;
	}

	Text& Text::SetFontWeight(std::string font_weight)
	{
		_font_weight = font_weight;
		return *this;
	}

	Text& Text::SetData(std::string data)
	{
		_data = data;
		return *this;
	}

	void Text::DataTextToXML(std::ostream& out) const
	{
		if (!_data.empty())
		{
			for (char ch : _data)
			{
				switch (ch)
				{
				case '"':
					out << "&quot;";
					break;
				case '\'':
					out << "&apos;";
					break;
				case '<':
					out << "&lt;";
					break;
				case '>':
					out << "&gt;";
					break;
				case '&':
					out << "&amp;";
					break;
				default:
					out << ch;
					break;
				}
			}
		}
	}

	void Text::RenderObject(const RenderContext& context) const
	{
		auto& out = context.out;
		out << "<text"sv;
		out << " x=\"" << _position.x << "\" y=\"" << _position.y << "\"";
		out << " dx=\"" << _offset.x << "\" dy=\"" << _offset.y << "\"";
		out << " font-size=\"" << _size << "\"";
		if (!_font_family.empty())
			out << " font-family=\"" << _font_family << "\"";
		if (!_font_weight.empty())
			out << " font-weight=\"" << _font_weight << "\"";

		RenderAttrs(context.out);

		out << ">";
		DataTextToXML(out);
		out << "</text>";
	}

	// ---------- Polyline ------------------

	Polyline& Polyline::AddPoint(Point point)
	{
		points_.push_back(point);
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const
	{
		auto& out = context.out;

		bool isFirst = true;

		out << "<polyline points=\""sv;
		for (Point p : points_)
		{
			if (!isFirst)
			{
				out << " "sv;
			}
			else
			{
				isFirst = false;
			}

			out << p.x << ","sv << p.y;
		}
		out << "\"";

		RenderAttrs(context.out);

		out << "/>"sv;
	}

	std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap)
	{
		switch (line_cap)
		{
		case svg::StrokeLineCap::BUTT:
			out << "butt"sv;
			break;
		case svg::StrokeLineCap::ROUND:
			out << "round"sv;
			break;
		case svg::StrokeLineCap::SQUARE:
			out << "square"sv;
			break;
		}
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join)
	{
		switch (line_join)
		{
		case svg::StrokeLineJoin::ARCS:
			out << "arcs"sv;
			break;
		case svg::StrokeLineJoin::BEVEL:
			out << "bevel"sv;
			break;
		case svg::StrokeLineJoin::MITER:
			out << "miter"sv;
			break;
		case svg::StrokeLineJoin::MITER_CLIP:
			out << "miter-clip"sv;
			break;
		case svg::StrokeLineJoin::ROUND:
			out << "round"sv;
			break;
		}
		return out;
	}

}  // namespace svg

namespace shapes
{
	svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
		using namespace svg;
		Polyline polyline;
		for (int i = 0; i <= num_rays; ++i) {
			double angle = 2 * M_PI * (i % num_rays) / num_rays;
			polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
			if (i == num_rays) {
				break;
			}
			angle += M_PI / num_rays;
			polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
		}
		return polyline;
	}


	void Triangle::Draw(svg::ObjectContainer& container) const
	{
		container.Add(svg::Polyline().AddPoint(_p1).AddPoint(_p2).AddPoint(_p3).AddPoint(_p1));
	}

	void Snowman::Draw(svg::ObjectContainer& container) const
	{
		const double r = _head_radius;
		const double h_y = _head_center.y;
		for (char i = 2; i >= 0; i--)
		{
			container.Add(svg::Circle().
				SetRadius(r + r * 0.5 * i).
				SetCenter(
					svg::Point{ _head_center.x, h_y + static_cast<int>((i) * 2.5) * r }).
				SetFillColor("rgb(240,240,240)"s).
				SetStrokeColor("black"s));
		}
	}

	void Star::Draw(svg::ObjectContainer& container) const
	{
		container.Add(CreateStar(_center, _outer_radius, _inner_radius, _num_rays).
			SetFillColor("red"s).
			SetStrokeColor("black"s));
	}
}

