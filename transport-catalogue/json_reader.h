#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>

namespace json_reader
{
    void ReadRequests( tc::TransportCatalogue& transp_cat,
                       map_render::MapSettings& map_settings,
                       std::istream& input );

    void InsertData( tc::TransportCatalogue& transp_cat, const json::Array& nodes );
    void GetData( tc::TransportCatalogue& transp_cat,
                  const json::Array& nodes,
                  map_render::MapSettings& map_settings );

    [[nodiscard]] json::Node GetStopData( std::optional<tc::StopInfo> stop_info, int request_id );
    [[nodiscard]] json::Node GetBusData( std::optional<tc::BusInfo> bus_info, int request_id );

    tc::StopParams ParseStop( const json::Dict& stop );
    tc::BusParams ParseBus( const json::Dict& bus );
    void RenderSettings( const json::Dict& nodes,
                         map_render::MapSettings& map_settings );
}