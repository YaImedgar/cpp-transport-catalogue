#pragma once

#include "geo.h"
#include "domain.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <sstream>
#include <set>
#include <list>
#include <tuple>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <cmath>
#include <optional>

using namespace geo;

namespace tc
{
	struct BusInfo
	{
		int total_stops;
		int total_unique_stops;
		double route_length;
		double curvature;
	};

	struct StopInfo
	{
		const std::set<std::string>& buses;
	};

	class TransportCatalogue
	{
	public:
		const domain::Stop* AddStop(domain::StopParams&& stop);
		domain::Stop* FindStop(std::string_view stop_name) const;
		void AddBus(domain::BusParams&& bus);
		domain::Bus* FindBus(std::string bus_name) const;

		std::optional<BusInfo> GetBusInfo(std::string bus_name) const;
		std::optional<StopInfo> GetStopInfo(std::string stop_name) const;
		const std::deque<domain::Bus>& GetAllBuses(void) const;
		const std::deque<domain::Stop>& GetAllStops(void) const;
	private:
		std::deque<domain::Bus> buses_;
		std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;

		std::deque<domain::Stop> stops_;
		std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_;
	};
}