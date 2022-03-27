#pragma once

#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"

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

using std::string;
using std::string_view;
using std::vector;
using std::deque;
using std::unordered_map;
using std::hash;
using std::get;
using std::list;
using std::tuple;
using std::set;
using std::for_each;
using std::move;
using std::optional;

using namespace tc::geo;

namespace tc
{
	typedef std::tuple<std::string/*Stop Name*/, tc::geo::Coordinates, std::unordered_map<std::string, long>/*Distance to other bus stop*/> StopParams;
	typedef std::tuple<std::string/*Bus Name*/, std::list<std::string>/*Stops names*/> BusParams;

	struct BusInfo
	{
		size_t total_stops;
		size_t total_unique_stops;
		double route_length;
		double curvature;
	};

	struct StopInfo
	{
		set<string> buses;
	};

	class Stop
	{
	public:
		explicit Stop(StopParams&& stop);

		Stop() = delete;
		Stop(const Stop& other) = delete;
		Stop& operator=(const Stop& other) = delete;

		Stop(Stop&& other) = delete;
		Stop& operator=(Stop&& other) = delete;

		std::string_view GetName();
		void AddBus(string bus_name);
		Coordinates GetCoordinates();
		set<string> GetBuses();
		void AddDistToOthStop(string stop, long distance);
		std::optional<double> GetDistanceToOtherStop(std::string other_stop);

	private:
		string _stop_name;
		Coordinates _coordinates;
		unordered_map<string, long> _distance_to_other_stop;
		set<string> _buses;
	};

	class Bus
	{
	public:
		explicit Bus(string name, vector<Stop*>&& bus);

		Bus() = delete;
		Bus(const Bus& other) = delete;
		Bus& operator=(const Bus& other) = delete;

		Bus(Bus&& other) = delete;
		Bus& operator=(Bus&& other) = delete;

		std::string_view GetName();
		size_t GetTotalStops();
		size_t GetTotalUniqueStopst();
		double GetRouteLength();
		double GetRouteLengthStraight();
		double GetCurvature();

	private:
		string _bus_name;
		vector<Stop*> _stops;
		size_t _total_stops;
		size_t _total_unique_stops;
		double _route_length_normal;
		double _route_length_straight;
	};

	class TransportCatalogue
	{
		typedef std::tuple<std::string, Coordinates, std::unordered_map<std::string, long>> StopParams;
	public:
		void AddStop(StopParams&& stop);
		Stop* FindStop(string stop_name) const;
		void AddBus(BusParams&& bus);
		Bus* FindBus(string bus_name) const;
		std::optional<BusInfo> GetBusInfo(string bus_name) const;
		std::optional<StopInfo> GetStopInfo(string stop_name) const;

	private:
		deque<Bus> _buses;
		unordered_map<std::string_view, Bus*> _busname_to_bus;

		deque<Stop> _stops;
		unordered_map<std::string_view, Stop*> _stopname_to_stop;
	};
}