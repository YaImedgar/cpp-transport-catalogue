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
        size_t id;
	};

	struct StopInfo
	{
		const std::set<std::string>& buses;
        size_t id;
	};

	class TransportCatalogue
	{
	public:
		const domain::Stop* AddStop(domain::StopParams&& stop);
        domain::Stop* FindStop(std::string_view stop_name) const;
        void AddBus(domain::BusParams&& bus);
		domain::Bus* FindBus(std::string_view bus_name) const;

		std::optional<BusInfo> GetBusInfo(std::string_view bus_name) const;
		std::optional<StopInfo> GetStopInfo(std::string_view stop_name) const;
		const std::deque<domain::Bus>& GetAllBuses(void) const;
		const std::deque<domain::Stop>& GetAllStops(void) const;

        std::string_view GetStopNameById(int id) const;
        std::string_view GetBusNameById(int id) const;
    private:
		std::deque<domain::Bus> buses_;
		std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;
        std::unordered_map<size_t, domain::Bus*> id_to_bus_;

		std::deque<domain::Stop> stops_;
        std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_;
        std::unordered_map<size_t, domain::Stop*> id_to_stop_;
    };
}