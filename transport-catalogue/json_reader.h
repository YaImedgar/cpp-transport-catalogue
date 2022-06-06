#pragma once

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

namespace json_reader
{
	class JSONReader final
	{
	public:
		JSONReader() = delete;
		JSONReader(tc::TransportCatalogue& catalogue,
			map_render::MapSettings& settings)
			: map_settings_(settings)
			, tc_handler_(request_handler::RequestHandler{ catalogue })
		{}
		JSONReader(const JSONReader&) = delete;
		JSONReader(JSONReader&&) = delete;
		JSONReader& operator=(const JSONReader&) = delete;
		JSONReader& operator=(JSONReader&& other) = delete;
		~JSONReader() = default;

		void ReadRequestFrom(std::istream& input);
	private:
		void ParseBaseRequests(const json::Array& base_requests);
		void SetRenderSettings(const json::Dict& settings,
			map_render::MapSettings& general_settings);
		void ParseStatRequests(const json::Array& stat_requests,
			map_render::MapSettings& map_settings,
			std::ostream& output);

		tc::StopParams ParseStop(const json::Dict& stop);
		tc::BusParams ParseBus(const json::Dict& bus);

		svg::Point GetSVGPoint(const json::Array& pair);
		svg::Color GetColor(const json::Node& color_node);

		map_render::MapSettings& map_settings_;
		request_handler::RequestHandler tc_handler_;
	};
}