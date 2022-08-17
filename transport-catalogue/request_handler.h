#pragma once
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "graph.h"
#include "router.h"
#include "geo.h"
#include "svg.h"
#include "serialization.h"

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
        std::optional<tc::BusInfo> GetBusInfo( std::string_view bus_name ) const;
        std::optional<tc::StopInfo> GetStopInfo( std::string_view stop_name ) const;
        const std::deque<domain::Bus>& GetAllBuses( void ) const;
        const std::deque<domain::Stop>& GetAllStops( void ) const;

        std::string_view GetStopNameById(int id) const;
        std::string_view GetBusNameById(int id) const;

        std::optional<graph::DirectedWeightedGraph<double>> GetGraph() const;
        std::optional<graph::Router<double>> GetRouter() const;

        bool SaveTo(std::string&& filename);
        bool ParseFrom(std::string&& filename);

        const map_render::MapSettings& GetRenderSettings() const;
        void SetRenderSettings(map_render::MapSettings&& settings);

        void CalculateGraph();
        void SetBusWaitTime(double time);
        void SetBusVelocity(double velocity);
    private:
        tc::TransportCatalogue& transp_cat_;
        map_render::MapSettings map_settings_;
        std::optional<graph::DirectedWeightedGraph<double>> graph_;
        std::optional<graph::Router<double>> router_;

        double bus_wait_time;
        double bus_velocity;
    };
}
