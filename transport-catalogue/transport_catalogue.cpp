#include "transport_catalogue.h"

namespace tc
{
    const domain::Stop* TransportCatalogue::AddStop( domain::StopParams&& stop_params )
    {
        if ( stopname_to_stop_.count( stop_params.name ) > 0 )
        {
            domain::Stop* stop_ptr = stopname_to_stop_.at( stop_params.name );
            if ( stop_ptr->GetCoordinates() != geo::Coordinates{} )
            {
                return stop_ptr;
            }

            stop_ptr->SetCoordinates( std::move( stop_params.coordinates ) );
            for ( auto& [stop, distance] : stop_params.dist_to_other_stop )
            {
                stop_ptr->AddDistToOtherStop( stop, distance );
            }

            for (auto& [id, distance] : stop_params.dist_to_other_stop_ids)
            {
                stop_ptr->AddDistToOtherStop(GetStopNameById(id), distance);
            }

            return stop_ptr;
        }
        if (stop_params.id == 0)
        {
            stop_params.id = stops_.size();
        }
        stops_.emplace_back( std::move( stop_params ) );
        domain::Stop& stop_ = stops_.back();
        stopname_to_stop_.emplace( stop_.GetName(), &stop_ );
        id_to_stop_.emplace(stop_.GetId(), &stop_);
        return &stop_;
    }

    domain::Stop* TransportCatalogue::FindStop( std::string_view stop_name ) const
    {
        if ( stopname_to_stop_.count( stop_name ) > 0 )
        {
            return stopname_to_stop_.at( stop_name );
        }

        return nullptr;
    }

    void TransportCatalogue::AddBus( domain::BusParams&& bus_params )
    {
        auto& [name, stops, is_route, id] = bus_params;
        std::vector<domain::Stop*> stops_vec;
        std::string bus_name = name;

        for_each( stops.begin(),
                  stops.end(),
                  [&stops_vec, &bus_name, this] ( std::string_view stop_name )
                  {
                      domain::Stop* stop = stopname_to_stop_.at( stop_name );
                      stop->AddBus( bus_name );
                      stops_vec.push_back( stop );
                  } );
        if (bus_params.id == 0)
        {
            id = buses_.size();
        }
        buses_.emplace_back( id, move( bus_name ), std::move( stops_vec ), is_route );

        domain::Bus& bus = buses_.back();
        busname_to_bus_.emplace( bus.GetName(), &bus );
        id_to_bus_.emplace(bus.GetId(), &bus);
    }

    domain::Bus* TransportCatalogue::FindBus( std::string_view bus_name ) const
    {
        if ( busname_to_bus_.count( bus_name ) > 0 )
        {
            return busname_to_bus_.at( bus_name );
        }
        else
        {
            return nullptr;
        }
    }

    std::optional<BusInfo> TransportCatalogue::GetBusInfo( std::string_view bus_name ) const
    {
        domain::Bus* bus_ptr = FindBus( bus_name );
        if ( bus_ptr != nullptr )
        {
            return BusInfo{ bus_ptr->GetTotalStops(),
                            bus_ptr->GetTotalUniqueStops(),
                            bus_ptr->GetRouteLength(),
                            bus_ptr->GetCurvature(),
                            bus_ptr->GetId() };
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<StopInfo> TransportCatalogue::GetStopInfo( std::string_view stop_name ) const
    {
        domain::Stop* stop_ptr = FindStop( stop_name );
        if ( stop_ptr != nullptr )
        {
            const std::set<std::string>& names_of_buses = stop_ptr->GetBuses();
            return std::move( StopInfo{ names_of_buses, stop_ptr->GetId() } );
        }
        else
        {
            return std::nullopt;
        }
    }

    const std::deque<domain::Bus>& TransportCatalogue::GetAllBuses( void ) const
    {
        return buses_;
    }

    const std::deque<domain::Stop>& TransportCatalogue::GetAllStops( void ) const
    {
        return stops_;
    }

    std::string_view TransportCatalogue::GetStopNameById(int id) const
    {
        return id_to_stop_.at(id)->GetName();
    }

    std::string_view TransportCatalogue::GetBusNameById(int id) const
    {
        return id_to_bus_.at(id)->GetName();
    }
}