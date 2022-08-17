#include "request_handler.h"

namespace request_handler
{
    RequestHandler::RequestHandler( tc::TransportCatalogue& transp_cat )
        : transp_cat_( transp_cat )
    {}

    void RequestHandler::AddStops( std::vector<domain::StopParams>& stops )
    {
        for ( auto& stop : stops )
        {
            transp_cat_.AddStop( std::move( stop ) );
        }
    }

    const domain::Stop* RequestHandler::AddStop( const std::string& stop_name )
    {
        domain::StopParams params;
        params.name = stop_name;
        return transp_cat_.AddStop( std::move( params ) );
    }

    void RequestHandler::AddBuses( std::vector<domain::BusParams>& buses )
    {
        for ( auto& bus : buses )
        {
            transp_cat_.AddBus( std::move( bus ) );
        }
    }

    const std::deque<domain::Bus>& RequestHandler::GetAllBuses( void ) const
    {
        return transp_cat_.GetAllBuses();
    }

    const std::deque<domain::Stop>& RequestHandler::GetAllStops( void ) const
    {
        return transp_cat_.GetAllStops();
    }

    std::optional<graph::DirectedWeightedGraph<double>> RequestHandler::GetGraph() const
    {
        return graph_;
    }

    std::optional<graph::Router<double>> RequestHandler::GetRouter() const
    {
        return router_;
    }

    void RequestHandler::CalculateGraph()
    {
        const auto& stops = GetAllStops();
        const auto& buses = GetAllBuses();

        auto stops_count = stops.size();
        graph::DirectedWeightedGraph<double> graph( stops_count * 2 );

        for ( const auto& stop : stops )
        {
            size_t stop_id = stop.GetId();
            graph.AddEdge( { stop_id, stop_id + stops_count, bus_wait_time, 0, stop.GetName() } );
        }

        for ( const auto& bus : buses )
        {
            auto& bus_stops = bus.GetStops();

            for ( auto start_iter = bus_stops.begin(); start_iter != bus_stops.end(); ++start_iter )
            {
                auto start_stop = *start_iter;
                size_t span_count{};
                double distance{};
                for ( auto next_iter = std::next( start_iter ); next_iter != bus_stops.end(); ++next_iter )
                {
                    auto next_stop = *next_iter;
                    auto small_distance = std::prev( next_iter ).operator*()->GetDistToOtherStop( next_stop->GetName() );
                    if ( small_distance )
                    {
                        distance += small_distance.value();
                    }
                    else
                    {
                        distance += next_stop->GetDistToOtherStop( std::prev( next_iter ).operator*()->GetName() ).value();
                    }

                    graph.AddEdge( { start_stop->GetId() + stops_count,
                                   next_stop->GetId(),
                                   distance / bus_velocity,
                                   ++span_count,
                                   bus.GetName() } );
                }
            }
        }

        graph_ = std::move( graph );
        router_.emplace( std::move( graph::Router{ graph_.value() } ) );
    }

    std::optional<tc::StopInfo> RequestHandler::GetStopInfo( std::string_view stop_name ) const
    {
        return transp_cat_.GetStopInfo( stop_name );
    }

    const domain::Stop* RequestHandler::FindStop( std::string_view name )
    {
        return transp_cat_.FindStop( name );
    }

    std::optional<tc::BusInfo> RequestHandler::GetBusInfo( std::string_view bus_name ) const
    {
        return transp_cat_.GetBusInfo( bus_name );
    }

    bool RequestHandler::ParseFrom(std::string&& filename)
    {
        auto serialize_tc = tc_serialize::Serialize::ParseFrom(std::move(filename));
        if (!serialize_tc.has_value())
        {
            return false;
        }

        // Filling basic stops with id and name for next string_view using
        auto stops =  serialize_tc->stops();
        for (auto stop : stops)
        {
            domain::StopParams stop_params;
            stop_params.id = stop.id();
            stop_params.name = stop.name();
            transp_cat_.AddStop(std::move(stop_params));
        }

        // Main filling stops
        for (auto stop : stops)
        {
            domain::StopParams stop_params;
            stop_params.name = stop.name();
            stop_params.coordinates = geo::Coordinates{stop.lat(), stop.lng()};
            auto dist_to_stops = stop.distance_to_other_stops();
            for (auto dist : dist_to_stops)
            {
                stop_params.dist_to_other_stop_ids[dist.stop_id()] = dist.distance();
            }
            transp_cat_.AddStop(std::move(stop_params));
        }

        auto buses = serialize_tc->buses();
        for (auto bus: buses)
        {
            domain::BusParams bus_params;

            bus_params.id = bus.id();
            bus_params.name = bus.name();
            bus_params.is_route = bus.is_round_route();
            std::list<std::string> stop_list;
            for (auto stop_id: bus.stops_ids())
            {
                stop_list.emplace_back(std::move(std::string(transp_cat_.GetStopNameById(stop_id))));
            }

            bus_params.stops_list = std::move(stop_list);

            transp_cat_.AddBus(std::move(bus_params));
        }

        map_settings_ = std::move(tc_serialize::Serialize::ParseMapFromSerialized(*serialize_tc->mutable_render_settings()));

        if (serialize_tc->has_routing_settings())
        {
            auto rout_settings = serialize_tc->routing_settings();

            graph_.emplace(std::move(tc_serialize::Serialize::ParseGraph(*this, rout_settings.graph())));
            auto routes_internal_data = tc_serialize::Serialize::ParseRouter(*this,  rout_settings);
            router_.emplace(std::move(graph::Router(graph_.value(), std::move(routes_internal_data))));
        }

        return true;
    }

    bool RequestHandler::SaveTo(std::string&& filename)
    {
        tc_serialize::TransportCatalogue serialize_tc;

        {// buses
            auto serialize_buses = tc_serialize::Serialize::BusesToSerialize(transp_cat_);
            *serialize_tc.mutable_buses() = {serialize_buses.begin(), serialize_buses.end()};
        }

        {// stops
            auto serialize_stops = tc_serialize::Serialize::StopsToSerialize(transp_cat_);
            *serialize_tc.mutable_stops() = {serialize_stops.begin(), serialize_stops.end()};
        }

        {// render settings
            *serialize_tc.mutable_render_settings() = std::move(tc_serialize::Serialize::RenderSettingsToSerialize(map_settings_));
        }

        if (router_.has_value())
        {// routing settings
            *serialize_tc.mutable_routing_settings() = std::move(tc_serialize::Serialize::SerializeRoutingSettings(*this, graph_.value(), router_.value()));
        }

        return tc_serialize::Serialize::SaveTo(std::move(filename), std::move(serialize_tc));
    }

    void RequestHandler::SetBusWaitTime(double time)
    {
        bus_wait_time = time;
    }

    void RequestHandler::SetBusVelocity(double velocity)
    {
        bus_velocity = velocity;
    }

    void RequestHandler::SetRenderSettings(map_render::MapSettings&& settings)
    {
        map_settings_ = std::move(settings);
    }

    const map_render::MapSettings &RequestHandler::GetRenderSettings() const
    {
        return map_settings_;
    }

    std::string_view RequestHandler::GetStopNameById(int id) const
    {
        return transp_cat_.GetStopNameById(id);
    }

    std::string_view RequestHandler::GetBusNameById(int id) const
    {
        return transp_cat_.GetBusNameById(id);
    }
}
