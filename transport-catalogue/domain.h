#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <tuple>
#include <optional>	
#include <set>	
#include <vector>	
#include <list>	
#include <unordered_map>	

namespace domain
{
	typedef std::tuple<std::string/*Stop Name*/, geo::Coordinates, std::unordered_map<std::string, int>/*Distance to other bus stop*/> StopParams;
	typedef std::tuple<std::string/*Bus Name*/, std::list<std::string>/*Stops names*/> BusParams;

	class Stop
	{
	public:
		explicit Stop(StopParams&& stop);

		Stop() = delete;
		Stop(const Stop& other) = delete;
		Stop& operator=(const Stop& other) = delete;

		Stop(Stop&& other) = delete;
		Stop& operator=(Stop&& other) = delete;

		std::string_view GetName() const;
		void AddBus(std::string bus_name);
		geo::Coordinates GetCoordinates() const;
		std::set<std::string> GetBuses() const;
		void AddDistToOthStop(std::string stop, long distance);
		std::optional<double> GetDistanceToOtherStop(std::string other_stop);

	private:
		std::string _stop_name;
		geo::Coordinates _coordinates;
		std::unordered_map<std::string, int> _distance_to_other_stop;
		std::set<std::string> _buses;
	};

	class Bus
	{
	public:
		explicit Bus(std::string name, std::vector<Stop*>&& bus, bool is_round);

		Bus() = delete;
		Bus(const Bus& other) = delete;
		Bus& operator=(const Bus& other) = delete;

		Bus(Bus&& other) = delete;
		Bus& operator=(Bus&& other) = delete;

		std::string_view GetName() const;
		int GetTotalStops() const;
		int GetTotalUniqueStopst() const;
		bool IsRoundRoute() const;
		double GetRouteLength() const;
		double GetRouteLengthStraight() const;
		double GetCurvature() const;
		const std::vector<Stop*> GetBusesStops() const;

	private:
		std::string _bus_name;
		std::vector<Stop*> _stops;
		int _total_stops;
		int _total_unique_stops;
		double _route_length_normal;
		double _route_length_straight;
		bool _is_round_route = false;
	};
} // end domain