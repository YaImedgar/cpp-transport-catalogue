#include "json_reader.h"

namespace json_reader
{
    JSONReader::JSONReader( tc::TransportCatalogue& catalogue )
        : tc_handler_( request_handler::RequestHandler{ catalogue } )
    {}

    void JSONReader::ParseBaseRequests( const json::Array& base_requests )
    {
        std::vector<domain::StopParams> stops;
        std::vector<domain::BusParams> buses;

        stops.reserve( base_requests.size() );
        buses.reserve( base_requests.size() );

        for ( const auto& request : base_requests )
        {
            const json::Dict values = request.AsDict();
            const std::string& type = values.at( "type" ).AsString();
            if ( "Stop" == type )
            {
                stops.push_back( ParseStop( values ) );
            }
            else if ( "Bus" == type )
            {
                buses.push_back( ParseBus( values ) );
            }
        }

        tc_handler_.AddStops( stops );
        tc_handler_.AddBuses( buses );
    }

    svg::Point JSONReader::GetSVGPoint( const json::Array& pair )
    {
        return svg::Point{ pair.front().AsDouble(), pair.back().AsDouble() };
    }

    svg::Color JSONReader::GetColor( const json::Node& color_node )
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

    map_render::MapSettings JSONReader::ParseRenderSettings( const json::Dict& settings )
    {
        map_render::MapSettings map_settings;

        if ( settings.count( "width" ) )
            map_settings.width = settings.at( "width" ).AsDouble();

        if ( settings.count( "height" ) )
            map_settings.height = settings.at( "height" ).AsDouble();

        if ( settings.count( "padding" ) )
            map_settings.padding = settings.at( "padding" ).AsDouble();

        if ( settings.count( "line_width" ) )
            map_settings.line_width = settings.at( "line_width" ).AsDouble();

        if ( settings.count( "stop_radius" ) )
            map_settings.stop_radius = settings.at( "stop_radius" ).AsDouble();

        if ( settings.count( "bus_label_font_size" ) )
            map_settings.bus_label_font_size = settings.at( "bus_label_font_size" ).AsInt();

        if ( settings.count( "bus_label_offset" ) )
            map_settings.bus_label_offset = GetSVGPoint( settings.at( "bus_label_offset" ).AsArray() );

        if ( settings.count( "stop_label_font_size" ) )
            map_settings.stop_label_font_size = settings.at( "stop_label_font_size" ).AsInt();

        if ( settings.count( "stop_label_offset" ) )
            map_settings.stop_label_offset = GetSVGPoint( settings.at( "stop_label_offset" ).AsArray() );

        if ( settings.count( "underlayer_color" ) )
            map_settings.underlayer_color = GetColor( settings.at( "underlayer_color" ) );

        if ( settings.count( "underlayer_width" ) )
            map_settings.underlayer_width = settings.at( "underlayer_width" ).AsDouble();

        if ( settings.count( "color_palette" ) )
        {
            map_settings.color_palette.clear();

            for ( const json::Node& color : settings.at( "color_palette" ).AsArray() )
            {
                map_settings.color_palette.push_back( GetColor( color ) );
            }
        }

        return std::move( map_settings );
    }

    void JSONReader::ParseStatRequests( const json::Array& stat_requests,
                                        const map_render::MapSettings& map_settings,
                                        std::ostream& output )
    {
        json::Array request_result;
        request_result.reserve( stat_requests.size() + 1 );

        const auto router = tc_handler_.GetRouter();
        const auto graph = tc_handler_.GetGraph();

        for ( const auto& request : stat_requests )
        {
            const auto& map = request.AsDict();
            std::string_view request_type = map.at( "type" ).AsString();
            int request_id = map.at( "id" ).AsInt();
            bool found = false;

            if ( "Map" == request_type )
            {
                std::stringstream ss = map_render::MapRender::Render( map_settings, tc_handler_.GetAllBuses() );
                request_result.emplace_back( json::Builder{}.
                                             StartDict().
                                             Key( "request_id" ).Value( request_id ).
                                             Key( "map" ).Value( ss.str() ).
                                             EndDict().
                                             Build().AsDict() );
                continue;
            }

            if ( "Route" == request_type )
            {
                const std::string& stop_from = map.at( "from" ).AsString();
                const std::string& stop_to = map.at( "to" ).AsString();

                auto route = router->BuildRoute( tc_handler_.FindStop( stop_from )->GetId(),
                                                 tc_handler_.FindStop( stop_to )->GetId() );

                if ( route.has_value() )
                {
                    found = true;
                    json::Array items;
                    items.reserve( route.value().edges.size() );

                    for ( auto edge : route.value().edges )
                    {
                        json::Dict items_dict;
                        std::string type{};
                        auto item = graph.value().GetEdge( edge );
                        if ( item.span_count == 0 )
                        {
                            type = "Wait";
                            items_dict["stop_name"] = std::string( item.name );
                        }
                        else
                        {
                            type = "Bus";
                            items_dict["bus"] = std::string( item.name );
                            items_dict["span_count"] = static_cast< int >( item.span_count );
                        }

                        items_dict["type"] = type;
                        items_dict["time"] = item.weight;
                        items.emplace_back( std::move( items_dict ) );
                    }

                    json::Dict route_dict;
                    route_dict["request_id"] = request_id;
                    route_dict["total_time"] = route.value().weight;
                    route_dict["items"] = std::move( items );
                    request_result.emplace_back( std::move( route_dict ) );
                }
            }
            else
            {
                std::string name = map.at( "name" ).AsString();
                if ( "Stop" == request_type )
                {
                    std::optional<tc::StopInfo> stop_info = tc_handler_.GetStopInfo( name );

                    if ( stop_info.has_value() )
                    {
                        found = true;

                        json::Array buses;

                        const std::set<std::string>& buses_list = stop_info.value().buses;
                        buses.reserve( buses_list.size() );
                        for ( const std::string& bus : buses_list )
                        {
                            buses.emplace_back( bus );
                        }

                        json::Dict stop_dict;
                        stop_dict["request_id"] = request_id;
                        stop_dict["buses"] = std::move( buses );
                        request_result.emplace_back( std::move( stop_dict ) );
                    }
                }
                else if ( "Bus" == request_type )
                {
                    std::optional<tc::BusInfo> bus_info = tc_handler_.GetBusInfo( name );
                    if ( bus_info.has_value() )
                    {
                        found = true;

                        tc::BusInfo b_i = bus_info.value();
                        request_result.emplace_back( std::move( json::Builder{}.
                                                                StartDict().
                                                                Key( "request_id" ).Value( request_id ).
                                                                Key( "stop_count" ).Value( b_i.total_stops ).
                                                                Key( "unique_stop_count" ).Value( b_i.total_unique_stops ).
                                                                Key( "route_length" ).Value( b_i.route_length ).
                                                                Key( "curvature" ).Value( b_i.curvature ).
                                                                EndDict().
                                                                Build().AsDict() ) );

                    }
                }
            }

            if ( !found )
            {
                request_result.emplace_back( std::move( json::Builder{}.
                                                        StartDict().
                                                        Key( "request_id"s ).Value( request_id ).
                                                        Key( "error_message"s ).Value( "not found"s ).
                                                        EndDict().
                                                        Build().AsDict() ) );
            }
        }

        json::Document doc( std::move( request_result ) );
        json::Print( std::move( doc ), output );
    }

    domain::StopParams JSONReader::ParseStop( const json::Dict& stop )
    {
        std::string name = stop.at( "name" ).AsString();

        double latitude = stop.at( "latitude" ).AsDouble();
        double longitude = stop.at( "longitude" ).AsDouble();

        std::unordered_map<std::string_view, int> distance_to_other_stop;
        for ( const auto& [stop_name, distance] : stop.at( "road_distances" ).AsDict() )
        {
            auto stop_ptr = tc_handler_.AddStop( stop_name );
            distance_to_other_stop[stop_ptr->GetName()] = distance.AsInt();
        }

        return { name, Coordinates{latitude, longitude}, distance_to_other_stop };
    }

    domain::BusParams JSONReader::ParseBus( const json::Dict& bus )
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

        return std::move( domain::BusParams{ bus_name, stops_list, is_cycle_route } );
    }

    bool JSONReader::MakeBase(std::istream &input)
    {
        const json::Dict& requests = GetRequests(input);

        if (!requests.count("serialization_settings"))
        {
            return false;
        }

        if ( requests.count( "base_requests" ) )
        {
            auto base_requests = requests.at( "base_requests" ).AsArray();
            ParseBaseRequests( std::move( base_requests ) );
        }

        if ( requests.count( "render_settings" ) )
        {
            auto render_settings = requests.at( "render_settings" ).AsDict();
            tc_handler_.SetRenderSettings(std::move(ParseRenderSettings( std::move( render_settings ) )));
        }

        if ( requests.count( "routing_settings" ) )
        {
            auto route_settings = requests.at( "routing_settings" ).AsDict();
            double bus_wait_time = route_settings.at( "bus_wait_time" ).AsDouble();
            double bus_velocity = route_settings.at( "bus_velocity" ).AsDouble() * 1000 / 60; // meters in hour

            tc_handler_.SetBusVelocity(bus_velocity);
            tc_handler_.SetBusWaitTime(bus_wait_time);
            tc_handler_.CalculateGraph();
        }

        std::string filename = GetFileName(requests);
        return tc_handler_.SaveTo(std::move(filename));
    }

    bool JSONReader::ProcessRequests(std::istream &input)
    {
        const json::Dict& requests = GetRequests(input);

        if (requests.count("serialization_settings"))
        {
            std::string filename = GetFileName(requests);
            if (!tc_handler_.ParseFrom(std::move(filename)))
            {
                return false;
            }
        }

        if ( requests.count( "stat_requests" ) )
        {
            auto stat_requests = requests.at( "stat_requests" ).AsArray();
            ParseStatRequests( std::move( stat_requests ), tc_handler_.GetRenderSettings(), std::cout );
        }

        return true;
    }

    json::Dict JSONReader::GetRequests(std::istream& input) {
        return json::Load( input ).GetRoot().AsDict();
    }

    std::string JSONReader::GetFileName(const json::Dict& requests) const
    {
        return std::move(requests.at("serialization_settings").AsDict().at("file").AsString());
    }
}