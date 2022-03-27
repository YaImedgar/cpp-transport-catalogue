#include "transport_catalogue.h"

namespace tc
{
	void TransportCatalogue::AddStop(StopParams&& stop)
	{
		_stops.emplace_back(move(stop));
		Stop& stop_ = _stops.back();
		_stopname_to_stop.emplace(stop_.GetName(), &stop_);
	}

	Stop* TransportCatalogue::FindStop(string stop_name) const
	{
		if (_stopname_to_stop.count(stop_name) > 0)
		{
			return _stopname_to_stop.at(stop_name);
		}
		else
		{
			return nullptr;
		}
	}

	void TransportCatalogue::AddBus(BusParams&& bus)
	{
		list<string> stops = move(get<1>(bus));
		vector<Stop*> stop_vec;
		string bus_name = move(get<0>(bus));

		for_each(stops.begin(), stops.end(), [&bus_name, &stop_vec, this](string& stop_)
			{
				Stop* stop = _stopname_to_stop.at(stop_);
				stop->AddBus(bus_name);
				stop_vec.push_back(stop);
			});

		_buses.emplace_back(move(bus_name), move(stop_vec));

		Bus& bus_ = _buses.back();
		_busname_to_bus.emplace(bus_.GetName(), &bus_);
	}

	Bus* TransportCatalogue::FindBus(string bus_name) const
	{
		if (_busname_to_bus.count(bus_name) > 0)
		{
			return _busname_to_bus.at(bus_name);
		}
		else
		{
			return nullptr;
		}
	}

	std::optional<BusInfo> TransportCatalogue::GetBusInfo(string bus_name) const
	{
		Bus* bus_ptr = FindBus(bus_name);
		if (bus_ptr != nullptr)
		{
			return BusInfo{ bus_ptr->GetTotalStops(),
							bus_ptr->GetTotalUniqueStopst(),
							bus_ptr->GetRouteLength(),
							bus_ptr->GetCurvature() };
		}
		else
		{
			return std::nullopt;
		}
	}

	std::optional<StopInfo> TransportCatalogue::GetStopInfo(string stop_name) const
	{
		Stop* stop_ptr = FindStop(stop_name);
		if (stop_ptr != nullptr)
		{
			set<string> names_of_buses = stop_ptr->GetBuses();
			return StopInfo{ names_of_buses };
		}
		else
		{
			return std::nullopt;
		}
	}

	Bus::Bus(string name, vector<Stop*>&& stops)
		: _bus_name(move(name)), _stops(move(stops)), _total_stops(_stops.size())
	{
		_total_unique_stops = set<Stop*>(_stops.begin(), _stops.end()).size();
		_route_length_straight = 0;
		_route_length_normal = 0;
		for (size_t i = 0; i < _stops.size() - 1; i++)
		{
			_route_length_straight += ComputeDistance(_stops[i]->GetCoordinates(),
				_stops[i + 1]->GetCoordinates());

			string_view cur_stop = _stops[i]->GetName();
			string_view next_stop = _stops[i + 1]->GetName();

			std::optional<double> dist_to = _stops[i]->GetDistanceToOtherStop(static_cast<string>(next_stop));
			if (dist_to.has_value())
			{
				_route_length_normal += dist_to.value();
			}
			else
			{
				_route_length_normal += _stops[i + 1]->GetDistanceToOtherStop(static_cast<string>(cur_stop)).value();
			}
		}
	}

	std::string_view Bus::GetName()
	{
		return _bus_name;
	}

	size_t Bus::GetTotalStops()
	{
		return _total_stops;
	}

	size_t Bus::GetTotalUniqueStopst()
	{
		return _total_unique_stops;
	}

	double Bus::GetRouteLength()
	{
		return _route_length_normal;
	}

	double Bus::GetRouteLengthStraight()
	{
		return _route_length_straight;
	}

	double Bus::GetCurvature()
	{
		return GetRouteLength() / GetRouteLengthStraight();
	}

	Stop::Stop(StopParams&& stop)
		: _stop_name(move(get<0>(stop))),
		_coordinates(move(get<1>(stop))),
		_distance_to_other_stop(move(get<2>(stop)))
	{
	}

	std::string_view Stop::GetName()
	{
		return _stop_name;
	}

	void Stop::AddBus(string bus_name)
	{
		_buses.insert(bus_name);
	}

	Coordinates Stop::GetCoordinates()
	{
		return _coordinates;
	}

	set<string> Stop::GetBuses()
	{
		return _buses;
	}

	void Stop::AddDistToOthStop(string stop, long distance)
	{
		_distance_to_other_stop[stop] = distance;
	}

	std::optional<double> Stop::GetDistanceToOtherStop(std::string other_stop)
	{
		if (_distance_to_other_stop.count(other_stop))
		{
			return { _distance_to_other_stop[other_stop] };
		}
		else
		{
			return std::nullopt;
		}
	}
}