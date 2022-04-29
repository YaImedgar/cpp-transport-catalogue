	#include "transport_catalogue.h"

	namespace tc
	{
		void TransportCatalogue::AddStop(StopParams&& stop)
		{
			_stops.emplace_back(move(stop));
			domain::Stop& stop_ = _stops.back();
			_stopname_to_stop.emplace(stop_.GetName(), &stop_);
		}

		domain::Stop* TransportCatalogue::FindStop(std::string stop_name) const
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
			std::list<std::string> stops = std::move(std::get<1>(bus));
			std::vector<domain::Stop*> stop_vec;
			std::string bus_name = std::move(std::get<0>(bus));

			for_each(stops.begin(), stops.end(), [&bus_name, &stop_vec, this](std::string& stop_)
				{
					domain::Stop* stop = _stopname_to_stop.at(stop_);
					stop->AddBus(bus_name);
					stop_vec.push_back(stop);
				});

			_buses.emplace_back(move(bus_name), std::move(stop_vec), std::get<2>(bus));

			domain::Bus& bus_ = _buses.back();
			_busname_to_bus.emplace(bus_.GetName(), &bus_);
		}

		domain::Bus* TransportCatalogue::FindBus(std::string bus_name) const
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

		std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string bus_name) const
		{
			domain::Bus* bus_ptr = FindBus(bus_name);
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

		std::optional<StopInfo> TransportCatalogue::GetStopInfo(std::string stop_name) const
		{
			domain::Stop* stop_ptr = FindStop(stop_name);
			if (stop_ptr != nullptr)
			{
				std::set<std::string> names_of_buses = stop_ptr->GetBuses();
				return StopInfo{ names_of_buses };
			}
			else
			{
				return std::nullopt;
			}
		}
		const std::deque<domain::Bus>& TransportCatalogue::GetAllBuses( void ) const
		{
			return _buses;
		}
		const std::deque<domain::Stop>& TransportCatalogue::GetAllStops( void ) const
		{
			return _stops;
		}
	}