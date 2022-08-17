#pragma once
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "graph.h"
#include "router.h"

#include <iostream>

namespace json_reader
{
    class JSONReader final
    {
    public:
        JSONReader() = delete;
        JSONReader( tc::TransportCatalogue& catalogue );
        JSONReader( const JSONReader& ) = delete;
        JSONReader( JSONReader&& ) = delete;
        JSONReader& operator=( const JSONReader& ) = delete;
        JSONReader& operator=( JSONReader&& other ) = delete;
        ~JSONReader() = default;

        bool MakeBase( std::istream& input );
        bool ProcessRequests( std::istream& input );
    private:
        json::Dict GetRequests(std::istream& input);
        std::string GetFileName(const json::Dict& requests) const;
        void ParseBaseRequests( const json::Array& base_requests );
        map_render::MapSettings ParseRenderSettings( const json::Dict& settings );
        void ParseStatRequests( const json::Array& stat_requests,
                                const map_render::MapSettings& map_settings,
                                std::ostream& output );

        domain::StopParams ParseStop( const json::Dict& stop );
        domain::BusParams ParseBus( const json::Dict& bus );

        svg::Point GetSVGPoint( const json::Array& pair );
        svg::Color GetColor( const json::Node& color_node );

        request_handler::RequestHandler tc_handler_;
    };
}