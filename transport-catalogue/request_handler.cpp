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

    void RequestHandler::CalculateGraph( double bus_wait_time, double bus_velocity )
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
            auto& bus_stops = bus.GetBusesStops();

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

    std::optional<tc::StopInfo> RequestHandler::GetStopInfo( std::string stop_name ) const
    {
        return transp_cat_.GetStopInfo( stop_name );
    }

    const domain::Stop* RequestHandler::FindStop( std::string_view name )
    {
        return transp_cat_.FindStop( name );
    }

    std::optional<tc::BusInfo> RequestHandler::GetBusInfo( std::string bus_name ) const
    {
        return transp_cat_.GetBusInfo( bus_name );
    }
}
