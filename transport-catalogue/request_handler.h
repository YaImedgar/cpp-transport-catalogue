#pragma once
#include "domain.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include <unordered_map>

#include <vector>

namespace request_handler
{
    class RequestHandler final
    {
    public:
        RequestHandler() = delete;
        RequestHandler( tc::TransportCatalogue& transp_cat );
        RequestHandler( const RequestHandler& ) = delete;
        RequestHandler( RequestHandler&& ) = delete;
        RequestHandler& operator=( const RequestHandler& ) = delete;
        RequestHandler& operator=( RequestHandler&& other ) = delete;
        ~RequestHandler() = default;

        void AddStops( std::vector<domain::StopParams>& stops );
        const domain::Stop* AddStop( const std::string& stop_name );
        void AddBuses( std::vector<domain::BusParams>& buses );

        const domain::Stop* FindStop( std::string_view name );
        std::optional<tc::BusInfo> GetBusInfo( std::string bus_name ) const;
        std::optional<tc::StopInfo> GetStopInfo( std::string stop_name ) const;
        const std::deque<domain::Bus>& GetAllBuses( void ) const;
        const std::deque<domain::Stop>& GetAllStops( void ) const;
        std::optional<graph::DirectedWeightedGraph<double>> GetGraph() const;
        std::optional<graph::Router<double>> GetRouter() const;

        void CalculateGraph( double bus_wait_time, double bus_velocity );

    private:
        tc::TransportCatalogue& transp_cat_;
        std::optional<graph::DirectedWeightedGraph<double>> graph_;
        std::optional<graph::Router<double>> router_;
    };
}
