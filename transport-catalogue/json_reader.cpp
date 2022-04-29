#include "json_reader.h"

namespace json_reader
{
    void ReadRequests( tc::TransportCatalogue& transp_cat,
                       map_render::MapSettings& map_settings,
                       std::istream& input )
    {
        json::Document json_doc = json::Load( input );
        const json::Node& node = json_doc.GetRoot();
        const json::Dict& requests = node.AsMap();

        if ( requests.count( "base_requests" ) )
        {
            InsertData( transp_cat, requests.at( "base_requests" ).AsArray() );
        }

        if ( requests.count( "render_settings" ) )
        {
            RenderSettings( requests.at( "render_settings" ).AsMap(), map_settings );
        }
        if ( requests.count( "stat_requests" ) )
        {
            GetData( transp_cat, requests.at( "stat_requests" ).AsArray(), map_settings );
        }
    }

    svg::Point GetSVGPoint( const json::Array& pair )
    {
        return svg::Point{ pair.front().AsDouble(), pair.back().AsDouble() };
    }

    svg::Color GetColor( const json::Node& color_node )
    {
        if ( color_node.IsString() )
        {
            return svg::Color( color_node.AsString() );
        }
        else if ( color_node.IsArray() )
        {
            uint8_t rgb[3];
            const json::Array color = color_node.AsArray();
            for ( size_t i = 0; i < 3; i++ )
            {
                rgb[i] = static_cast< uint8_t >( color[i].AsInt() );
            }
            if ( color.size() == 3 )
            {
                return svg::Rgb( rgb[0], rgb[1], rgb[2] );
            }
            else if ( color.size() == 4 )
            {
                return svg::Rgba( rgb[0], rgb[1], rgb[2], color[3].AsDouble() );
            }
        }
        return {};
    }

    void RenderSettings( const json::Dict& nodes,
                         map_render::MapSettings& map_settings )
    {
        if ( nodes.count( "width" ) )
            map_settings.width = nodes.at( "width" ).AsDouble();

        if ( nodes.count( "height" ) )
            map_settings.height = nodes.at( "height" ).AsDouble();

        if ( nodes.count( "padding" ) )
            map_settings.padding = nodes.at( "padding" ).AsDouble();

        if ( nodes.count( "line_width" ) )
            map_settings.line_width = nodes.at( "line_width" ).AsDouble();

        if ( nodes.count( "stop_radius" ) )
            map_settings.stop_radius = nodes.at( "stop_radius" ).AsDouble();

        if ( nodes.count( "bus_label_font_size" ) )
            map_settings.bus_label_font_size = nodes.at( "bus_label_font_size" ).AsInt();

        if ( nodes.count( "bus_label_offset" ) )
            map_settings.bus_label_offset = GetSVGPoint( nodes.at( "bus_label_offset" ).AsArray() );

        if ( nodes.count( "stop_label_font_size" ) )
            map_settings.stop_label_font_size = nodes.at( "stop_label_font_size" ).AsInt();

        if ( nodes.count( "stop_label_offset" ) )
            map_settings.stop_label_offset = GetSVGPoint( nodes.at( "stop_label_offset" ).AsArray() );

        if ( nodes.count( "underlayer_color" ) )
            map_settings.underlayer_color = GetColor( nodes.at( "underlayer_color" ) );

        if ( nodes.count( "underlayer_width" ) )
            map_settings.underlayer_width = nodes.at( "underlayer_width" ).AsDouble();

        if ( nodes.count( "color_palette" ) )
        {
            map_settings.color_palette.clear();

            for ( const json::Node& color : nodes.at( "color_palette" ).AsArray() )
            {
                map_settings.color_palette.push_back( GetColor( color ) );
            }
        }
    }

    void InsertData( tc::TransportCatalogue& transp_cat, const json::Array& nodes )
    {
        std::vector<json::Node> buses;
        std::vector<json::Node> stops;

        for ( const auto& node : nodes )
        {
            const json::Dict values = node.AsMap();
            std::string_view type = values.at( "type" ).AsString();
            if ( type.compare( "Stop" ) == 0 )
            {
                stops.push_back( values );
            }
            else if ( type.compare( "Bus" ) == 0 )
            {
                buses.push_back( values );
            }
        }

        for ( const auto& stop : stops )
        {
            transp_cat.AddStop( std::move( ParseStop( stop.AsMap() ) ) );
        }
        for ( const auto& bus : buses )
        {
            transp_cat.AddBus( std::move( ParseBus( bus.AsMap() ) ) );
        }
    }

    void GetData( tc::TransportCatalogue& transp_cat,
                  const json::Array& nodes,
                  map_render::MapSettings& map_settings )
    {
        json::Array request_handler;
        for ( const auto& query : nodes )
        {
            const auto& map = query.AsMap();
            std::string_view query_str = map.at( "type" ).AsString();
            int request_id = map.at( "id" ).AsInt();

            if ( query_str.compare( "Map" ) == 0 )
            {
                json::Dict map_dict;
                map_dict.emplace( "request_id", request_id );
                std::stringstream ss = map_render::MapRender::Render( map_settings,
                                                                      transp_cat.GetAllBuses() );

                map_dict.emplace( "map", ss.str() );

                request_handler.emplace_back( std::move( map_dict ) );
            }
            else
            {
                std::string name = map.at( "name" ).AsString();
                if ( query_str.compare( "Stop" ) == 0 )
                {
                    std::optional<tc::StopInfo> stop_info = transp_cat.GetStopInfo( name );
                    request_handler.emplace_back( std::move( GetStopData( stop_info, request_id ) ) );
                }
                else if ( query_str.compare( "Bus" ) == 0 )
                {
                    std::optional<tc::BusInfo> bus_info = transp_cat.GetBusInfo( name );
                    request_handler.emplace_back( std::move( GetBusData( bus_info, request_id ) ) );
                }
            }
        }
        json::Document doc( request_handler );
        json::Print( doc, std::cout );
    }

    json::Node GetStopData( std::optional<tc::StopInfo> stop_info, int request_id )
    {
        json::Dict stop_dict;
        stop_dict.emplace( "request_id", request_id );
        json::Array arr;

        if ( stop_info.has_value() )
        {
            std::set<std::string> buses = stop_info.value().buses;
            if ( !buses.empty() )
            {
                for ( const std::string& bus : buses )
                {
                    arr.push_back( bus );
                }
            }
            stop_dict.emplace( "buses", arr );
        }
        else
        {
            stop_dict.emplace( "error_message", "not found" );
        }
        return stop_dict;
    }

    json::Node GetBusData( std::optional<tc::BusInfo> bus_info, int request_id )
    {
        json::Dict bus_dict;
        bus_dict.emplace( "request_id", request_id );
        if ( bus_info.has_value() )
        {
            tc::BusInfo b_i = bus_info.value();
            bus_dict.emplace( "stop_count", b_i.total_stops );
            bus_dict.emplace( "unique_stop_count", b_i.total_unique_stops );
            bus_dict.emplace( "route_length", b_i.route_length );
            bus_dict.emplace( "curvature", b_i.curvature );
        }
        else
        {
            bus_dict.emplace( "error_message", "not found" );
        }
        return bus_dict;
    }

    tc::StopParams ParseStop( const json::Dict& stop )
    {
        std::string stop_name = stop.at( "name" ).AsString();

        double latitude = stop.at( "latitude" ).AsDouble();
        double longitude = stop.at( "longitude" ).AsDouble();

        std::unordered_map<std::string, int> distance_to_other_stop;

        for ( const auto& [stop_name, distance] : stop.at( "road_distances" ).AsMap() )
        {
            distance_to_other_stop.emplace( stop_name, distance.AsInt() );
        }

        return { stop_name, Coordinates{latitude, longitude}, distance_to_other_stop };
    }

    tc::BusParams ParseBus( const json::Dict& bus )
    {
        std::string bus_name = bus.at( "name" ).AsString();

        std::list<std::string> stops_list;
        bool is_cycle_route = bus.at( "is_roundtrip" ).AsBool();

        for ( const auto& stop : bus.at( "stops" ).AsArray() )
        {
            stops_list.emplace_back( stop.AsString() );
        }

        if ( !is_cycle_route )
        {
            stops_list.insert( stops_list.end(), std::next( stops_list.rbegin() ), stops_list.rend() );
        }

        return { bus_name, stops_list, is_cycle_route };
    }
}