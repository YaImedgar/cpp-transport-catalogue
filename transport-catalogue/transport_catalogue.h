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

using namespace geo;

namespace tc
{
	typedef std::tuple<std::string/*Stop Name*/, geo::Coordinates, std::unordered_map<std::string, int>/*Distance to other bus stop*/> StopParams;
	typedef std::tuple<std::string/*Bus Name*/, std::list<std::string>/*Stops names*/, bool> BusParams;

	struct BusInfo
	{
		int total_stops;
		int total_unique_stops;
		double route_length;
		double curvature;
	};

	struct StopInfo
	{
		std::set<std::string> buses;
	};

	class TransportCatalogue
	{
	public:
		void AddStop(StopParams&& stop);
		domain::Stop* FindStop(std::string stop_name) const;
		void AddBus(BusParams&& bus);
		domain::Bus* FindBus(std::string bus_name) const;

		std::optional<BusInfo> GetBusInfo(std::string bus_name) const;
		std::optional<StopInfo> GetStopInfo(std::string stop_name) const;
		const std::deque<domain::Bus>& GetAllBuses(void) const;
		const std::deque<domain::Stop>& GetAllStops(void) const;

	private:
		std::deque<domain::Bus> _buses;
		std::unordered_map<std::string_view, domain::Bus*> _busname_to_bus;

		std::deque<domain::Stop> _stops;
		std::unordered_map<std::string_view, domain::Stop*> _stopname_to_stop;
	};
}