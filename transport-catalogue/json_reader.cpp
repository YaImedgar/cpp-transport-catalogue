#include "json_reader.h"

namespace json_reader
{
	void JSONReader::ReadRequestFrom(std::istream& input)
	{
		const json::Document json_doc = json::Load(input);
		const json::Node& node = json_doc.GetRoot();
		const json::Dict& requests = node.AsDict();

		if (requests.count("base_requests"))
		{
			auto base_requests = requests.at("base_requests").AsArray();
			ParseBaseRequests(std::move(base_requests));
		}

		if (requests.count("render_settings"))
		{
			auto render_settings = requests.at("render_settings").AsDict();
			SetRenderSettings(std::move(render_settings), map_settings_);
		}

		if (requests.count("stat_requests"))
		{
			auto stat_requests = requests.at("stat_requests").AsArray();
			ParseStatRequests(std::move(stat_requests), map_settings_, std::cout);
		}
	}

	void JSONReader::ParseBaseRequests(const json::Array& base_requests)
	{
		std::vector<tc::StopParams> stops;
		std::vector<tc::BusParams> buses;

		for (const auto& request : base_requests)
		{
			const json::Dict values = request.AsDict();
			std::string_view type = values.at("type").AsString();
			if ("Stop" == type)
			{
				stops.push_back(ParseStop(values));
			}
			else if ("Bus" == type)
			{
				buses.push_back(ParseBus(values));
			}
		}

		tc_handler_.AddStops(stops);
		tc_handler_.AddBuses(buses);
	}

	svg::Point JSONReader::GetSVGPoint(const json::Array& pair)
	{
		return svg::Point{ pair.front().AsDouble(), pair.back().AsDouble() };
	}

	svg::Color JSONReader::GetColor(const json::Node& color_node)
	{
		if (color_node.IsString())
		{
			return svg::Color(color_node.AsString());
		}
		else if (color_node.IsArray())
		{
			uint8_t rgb[3];
			const json::Array color = color_node.AsArray();
			for (size_t i = 0; i < 3; i++)
			{
				rgb[i] = static_cast<uint8_t>(color[i].AsInt());
			}
			if (color.size() == 3)
			{
				return svg::Rgb(rgb[0], rgb[1], rgb[2]);
			}
			else if (color.size() == 4)
			{
				return svg::Rgba(rgb[0], rgb[1], rgb[2], color[3].AsDouble());
			}
		}
		return {};
	}

	void JSONReader::SetRenderSettings(const json::Dict& settings, map_render::MapSettings& general_settings)
	{
		map_render::MapSettings map_settings;

		if (settings.count("width"))
			map_settings.width = settings.at("width").AsDouble();

		if (settings.count("height"))
			map_settings.height = settings.at("height").AsDouble();

		if (settings.count("padding"))
			map_settings.padding = settings.at("padding").AsDouble();

		if (settings.count("line_width"))
			map_settings.line_width = settings.at("line_width").AsDouble();

		if (settings.count("stop_radius"))
			map_settings.stop_radius = settings.at("stop_radius").AsDouble();

		if (settings.count("bus_label_font_size"))
			map_settings.bus_label_font_size = settings.at("bus_label_font_size").AsInt();

		if (settings.count("bus_label_offset"))
			map_settings.bus_label_offset = GetSVGPoint(settings.at("bus_label_offset").AsArray());

		if (settings.count("stop_label_font_size"))
			map_settings.stop_label_font_size = settings.at("stop_label_font_size").AsInt();

		if (settings.count("stop_label_offset"))
			map_settings.stop_label_offset = GetSVGPoint(settings.at("stop_label_offset").AsArray());

		if (settings.count("underlayer_color"))
			map_settings.underlayer_color = GetColor(settings.at("underlayer_color"));

		if (settings.count("underlayer_width"))
			map_settings.underlayer_width = settings.at("underlayer_width").AsDouble();

		if (settings.count("color_palette"))
		{
			map_settings.color_palette.clear();

			for (const json::Node& color : settings.at("color_palette").AsArray())
			{
				map_settings.color_palette.push_back(GetColor(color));
			}
		}
		general_settings = std::move(map_settings);
	}

	void JSONReader::ParseStatRequests(const json::Array& stat_requests,
		map_render::MapSettings& map_settings,
		std::ostream& output)
	{
		json::Array request_result;

		for (const auto& request : stat_requests)
		{
			const auto& map = request.AsDict();
			std::string_view request_type = map.at("type").AsString();
			int request_id = map.at("id").AsInt();


			if ("Map" == request_type)
			{
				std::stringstream ss = map_render::MapRender::Render(map_settings, tc_handler_.GetAllBuses());
				request_result.emplace_back(json::Builder{}.
					StartDict().
					Key("request_id").Value(request_id).
					Key("map").Value(ss.str()).
					EndDict().
					Build().AsDict());
			}
			else
			{
				bool isFound = false;

				std::string name = map.at("name").AsString();
				if ("Stop" == request_type)
				{
					std::optional<tc::StopInfo> stop_info = tc_handler_.GetStopInfo(name);

					if (stop_info.has_value())
					{
						isFound = true;

						json::Array buses;

						std::set<std::string> buses_list = stop_info.value().buses;
						for (const std::string& bus : buses_list)
						{
							buses.emplace_back(bus);
						}

						request_result.emplace_back(json::Builder{}.
							StartDict().
							Key("request_id").Value(request_id).
							Key("buses").Value(buses).
							EndDict().
							Build().AsDict());
					}
				}
				else if ("Bus" == request_type)
				{
					std::optional<tc::BusInfo> bus_info = tc_handler_.GetBusInfo(name);
					if (bus_info.has_value())
					{
						isFound = true;

						tc::BusInfo b_i = bus_info.value();
						request_result.emplace_back(json::Builder{}.
							StartDict().
							Key("request_id").Value(request_id).
							Key("stop_count").Value(b_i.total_stops).
							Key("unique_stop_count").Value(b_i.total_unique_stops).
							Key("route_length").Value(b_i.route_length).
							Key("curvature").Value(b_i.curvature).
							EndDict().
							Build().AsDict());

					}
				}

				if (!isFound)
				{
					request_result.emplace_back(json::Builder{}.
						StartDict().
						Key("request_id").Value(request_id).
						Key("error_message").Value("not found").
						EndDict().
						Build().AsDict());
				}
			}
		}

		json::Document doc(request_result);
		json::Print(doc, output);
	}

	tc::StopParams JSONReader::ParseStop(const json::Dict& stop)
	{
		std::string stop_name = stop.at("name").AsString();

		double latitude = stop.at("latitude").AsDouble();
		double longitude = stop.at("longitude").AsDouble();

		std::unordered_map<std::string, int> distance_to_other_stop;
		for (const auto& [stop_name, distance] : stop.at("road_distances").AsDict())
		{
			distance_to_other_stop.emplace(stop_name, distance.AsInt());
		}

		return { stop_name, Coordinates{latitude, longitude}, distance_to_other_stop };
	}

	tc::BusParams JSONReader::ParseBus(const json::Dict& bus)
	{
		std::string bus_name = bus.at("name").AsString();

		std::list<std::string> stops_list;
		bool is_cycle_route = bus.at("is_roundtrip").AsBool();

		for (const auto& stop : bus.at("stops").AsArray())
		{
			stops_list.emplace_back(stop.AsString());
		}

		if (!is_cycle_route)
		{
			stops_list.insert(stops_list.end(), std::next(stops_list.rbegin()), stops_list.rend());
		}

		return { bus_name, stops_list, is_cycle_route };
	}
}