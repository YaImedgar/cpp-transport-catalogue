#include "serialization.h"

namespace tc_serialize
{
    bool Serialize::SaveTo(std::string&& path, tc_serialize::TransportCatalogue&& serialize_tc)
    {
        std::ofstream output(path, std::ios::binary);
        return serialize_tc.SerializeToOstream(&output);
    }

    std::optional<tc_serialize::TransportCatalogue> Serialize::ParseFrom(std::string&& path)
    {
        tc_serialize::TransportCatalogue serialize_tc;

        std::ifstream input(path, std::ios::binary);

        if (!serialize_tc.ParseFromIstream(&input))
        {
            return std::nullopt;
        }

        return serialize_tc;
    }

    std::vector<tc_serialize::Bus> Serialize::BusesToSerialize(const tc::TransportCatalogue &tc)
    {
        const auto& tc_buses = tc.GetAllBuses();
        std::vector<tc_serialize::Bus> buses(tc_buses.size());

        int i = 0;
        for (const auto& bus : tc_buses)
        {
            auto& ser_bus = buses[i++];

            {
                std::vector<domain::Stop *> stops = bus.GetStops();
                std::vector<uint32_t> stop_ids(stops.size());

                int j = 0;
                for (auto stop: stops)
                {
                    stop_ids[j++] = stop->GetId();
                }
                *ser_bus.mutable_stops_ids() = {stop_ids.begin(), stop_ids.end()};
            }

            ser_bus.set_id(bus.GetId());
            ser_bus.set_name(std::string(bus.GetName()));
            ser_bus.set_is_round_route(bus.IsRoundRoute());
        }

        return std::move(buses);
    }

    std::vector<tc_serialize::Stop> Serialize::StopsToSerialize(const tc::TransportCatalogue &tc)
    {
        const auto& tc_stops = tc.GetAllStops();
        std::vector<tc_serialize::Stop> stops(tc_stops.size());

        int stop_count = 0;
        for (const auto& stop : tc_stops)
        {
            auto& ser_stop = stops[stop_count++];

            ser_stop.set_id(stop.GetId());
            ser_stop.set_name(std::string(stop.GetName()));
            auto coordinates = stop.GetCoordinates();
            ser_stop.set_lat(coordinates.lat);
            ser_stop.set_lng(coordinates.lng);

            {
                const auto& distance_to_other_stopes = stop.GetAllDistanceToOtherStops();
                std::vector<tc_serialize::DistanseToOtherStop> distances(distance_to_other_stopes.size());

                int stop_count = 0;
                for (auto& stop_dist : distance_to_other_stopes)
                {
                    auto& cur_distance = distances[stop_count++];
                    cur_distance.set_stop_id(tc.GetStopInfo(std::string(stop_dist.first))->id);
                    cur_distance.set_distance(stop_dist.second);
                }

                *ser_stop.mutable_distance_to_other_stops() = {distances.begin(), distances.end()};
            }

            {
                const std::set<std::string>& buses = stop.GetBuses();
                std::vector<uint32_t> buses_ids(buses.size());

                int bus_count = 0;
                for (auto bus : buses)
                {
                    auto bus_info = tc.GetBusInfo(bus);
                    buses_ids[bus_count++] = bus_info->id;
                }
                *ser_stop.mutable_buses_ids() = {buses_ids.begin(), buses_ids.end()};
            }
        }

        return std::move(stops);
    }

    tc_serialize::Point Serialize::GetPoint(svg::Point point)
    {
        tc_serialize::Point p;
        auto [x, y] = point;
        p.set_x(x);
        p.set_y(y);
        return std::move(p);
    }

    tc_serialize::Color Serialize::SetColor(svg::Color color)
    {
        tc_serialize::Color ser_color;
        uint32_t type_and_rgb{};
        if (std::holds_alternative<std::monostate>(color)) {
            type_and_rgb = 0x1;
        } else if (std::holds_alternative<std::string>(color)) {
            type_and_rgb = 0x2;
            *ser_color.mutable_color_name() = std::move(std::get<std::string>(color));
        }
        if (std::holds_alternative<svg::Rgb>(color)) {
            svg::Rgb rgb = std::move(std::get<svg::Rgb>(color));
            type_and_rgb = 0x3 | rgb.red << 8 | rgb.green << 16 | rgb.blue << 24;
        } else if (std::holds_alternative<svg::Rgba>(color)) {
            svg::Rgba rgba = std::move(std::get<svg::Rgba>(color));
            type_and_rgb = 0x4 | rgba.red << 8 | rgba.green << 16 | rgba.blue << 24;
            ser_color.set_opacity(rgba.opacity);
        }
        ser_color.set_type_and_rgb(type_and_rgb);
        return std::move(ser_color);
    }

    tc_serialize::RenderSettings Serialize::RenderSettingsToSerialize(const map_render::MapSettings& settings)
    {
        tc_serialize::RenderSettings ser_map;

        ser_map.set_width(settings.width);
        ser_map.set_height(settings.height);
        ser_map.set_padding(settings.padding);
        ser_map.set_line_width(settings.line_width);
        ser_map.set_stop_radius(settings.stop_radius);
        ser_map.set_bus_label_font_size(settings.bus_label_font_size);
        *ser_map.mutable_bus_label_offset() = std::move(GetPoint(settings.bus_label_offset));
        ser_map.set_stop_label_font_size(settings.stop_label_font_size);
        *ser_map.mutable_stop_label_offset() = std::move(GetPoint(settings.stop_label_offset));

        *ser_map.mutable_underlayer_color() = std::move(SetColor(settings.underlayer_color));
        ser_map.set_underlayer_width(settings.underlayer_width);
        const auto& color_pallete = settings.color_palette;
        std::vector<tc_serialize::Color> colors(color_pallete.size());
        int color_num = 0;
        for (const auto& color : color_pallete)
        {
            colors[color_num++] = std::move(SetColor(color));
        }
        *ser_map.mutable_color_palette() = {colors.begin(), colors.end()};

        return std::move(ser_map);
    }

    svg::Point Serialize::GetPoint(tc_serialize::Point point)
    {
        svg::Point p;
        p.x = point.x();
        p.y = point.y();
        return std::move(p);
    }

    svg::Color Serialize::GetColor(const tc_serialize::Color& color)
    {
        uint32_t rgb = color.type_and_rgb() >> 8;
        uint8_t type = color.type_and_rgb() & 0xff;
        if (type == 1) // monostate
        {
            return {};
        }
        if (type == 2) // string color
        {
            return svg::Color(color.color_name());
        }
        uint8_t r, g, b;
        r = rgb & 0xff;
        g = rgb >> 8 & 0xff;
        b = rgb >> 16;

        if (type == 3) // rgb
        {
            return {svg::Rgb(r, g, b)};
        }
        // rgba

        return {svg::Rgba{r, g, b, color.opacity()}};
    }

    std::vector<svg::Color>
    Serialize::GetColors(const google::protobuf::RepeatedPtrField<::tc_serialize::Color> &ser_colors)
    {
        std::vector<svg::Color> colors;
        colors.reserve(ser_colors.size());

        for (const auto& ser_color : ser_colors)
        {
            colors.push_back(std::move(GetColor(ser_color)));
        }

        return std::move(colors);
    }

    map_render::MapSettings Serialize::ParseMapFromSerialized(const tc_serialize::RenderSettings& serialized)
    {
        map_render::MapSettings map;

        map.width = serialized.width();
        map.height = serialized.height();
        map.padding = serialized.padding();
        map.line_width = serialized.line_width();
        map.stop_radius = serialized.stop_radius();
        map.bus_label_font_size = serialized.bus_label_font_size();
        map.bus_label_offset = GetPoint(serialized.bus_label_offset());
        map.stop_label_font_size = serialized.stop_label_font_size();
        map.stop_label_offset = GetPoint(serialized.stop_label_offset());

        map.underlayer_color = GetColor(serialized.underlayer_color());
        map.underlayer_width = serialized.underlayer_width();
        map.color_palette = GetColors(serialized.color_palette());

        return std::move(map);
    }

    tc_serialize::Graph Serialize::SerializeGraph(const request_handler::RequestHandler& tc_handler,
                                                  const graph::DirectedWeightedGraph<double>& graph)
    {
        tc_serialize::Graph ser_graph;
        {
            std::vector<tc_serialize::Edge> edges;
            auto graph_edges = graph.GetAllEdges();
            edges.reserve(graph_edges.size());

            for (const auto& edge : graph_edges)
            {
                edges.push_back(std::move(SerializeEdge(edge, tc_handler)));
            }

            *ser_graph.mutable_edges() = {edges.begin(), edges.end()};
        }
        {
            std::vector<tc_serialize::IncidenceList> incidence_list;
            auto graph_incidence_lists = graph.GetAllIncidenceLists();
            incidence_list.reserve(graph_incidence_lists.size());

            for (const auto& inner : graph_incidence_lists)
            {
                tc_serialize::IncidenceList list;
                *list.mutable_edge_ids() = {inner.begin(), inner.end()};
                incidence_list.push_back(std::move(list));
            }

            *ser_graph.mutable_incidence_list() = {incidence_list.begin(), incidence_list.end()};
        }
        return std::move(ser_graph);
    }

    google::protobuf::RepeatedPtrField<tc_serialize::RoutesInternalData> Serialize::SerializeRouter(const request_handler::RequestHandler& tc_handler,
                                                                                                    const graph::Router<double>& router)
    {
        auto routes_internal_data = router.GetRoutesInternalData();
        std::vector<tc_serialize::RoutesInternalData> serialize_data;
        serialize_data.reserve(routes_internal_data.size());

        for (const auto& internal_data : routes_internal_data)
        {
            std::vector<tc_serialize::RouteInternalData> route_serialize_data(internal_data.size());
            int inner = 0;
            for (const auto& final_data : internal_data)
            {
                tc_serialize::RouteInternalData data;
                if (final_data.has_value())
                {
                    data.set_weight(final_data->weight);
                    if (final_data->prev_edge.has_value())
                    {
                        data.set_prev_edge(final_data->prev_edge.value());
                    }
                    else
                    {
                        data.set_prev_edge(-1);
                    }
                }
                else
                {
                    data.set_is_empty(true);
                }
                route_serialize_data[inner++] = std::move(data);
            }

            tc_serialize::RoutesInternalData serialize_inner;
            *serialize_inner.mutable_route_internal_data_() = {route_serialize_data.begin(), route_serialize_data.end()};
            serialize_data.push_back(std::move(serialize_inner));
        }

        return {serialize_data.begin(), serialize_data.end()};
    }

    tc_serialize::Edge Serialize::SerializeEdge(const graph::Edge<double> &edge, const request_handler::RequestHandler& tc_handler)
    {
        tc_serialize::Edge ser_edge;
        auto [from, to, weight, span_count, name] = edge;
        ser_edge.set_from(from);
        ser_edge.set_to(to);
        ser_edge.set_weigth(weight);
        ser_edge.set_span_count(span_count);
        size_t id;
        if (span_count == 0)
        {
            auto stop_info = tc_handler.GetStopInfo(name);
            id = stop_info->id;
        }
        else
        {
            auto bus_info = tc_handler.GetBusInfo(name);
            id = bus_info->id;
        }
        ser_edge.set_stop_or_bus_id(id);

        return std::move(ser_edge);
    }

    graph::DirectedWeightedGraph<double> Serialize::ParseGraph(const request_handler::RequestHandler& tc_handler,
                                                               const tc_serialize::Graph& serialized_graph)
    {
        auto ser_edges = serialized_graph.edges();
        std::vector<graph::Edge<double>> edges(ser_edges.size());

        int graph_count = 0;
        for(const auto& ser_edge : ser_edges)
        {
            auto& edge = edges[graph_count++];
            edge.from = ser_edge.from();
            edge.to = ser_edge.to();
            edge.weight = ser_edge.weigth();
            edge.span_count = ser_edge.span_count();
            if (edge.span_count == 0)
            {
                edge.name = tc_handler.GetStopNameById(ser_edge.stop_or_bus_id());
            }
            else
            {
                edge.name = tc_handler.GetBusNameById(ser_edge.stop_or_bus_id());
            }
        }

        const auto& ser_incidence_list = serialized_graph.incidence_list();
        std::vector<graph::DirectedWeightedGraph<double>::IncidenceList> incidence_list(ser_incidence_list.size());
        int list_count = 0;
        for (const auto& ser_list : ser_incidence_list)
        {
            const auto& edges_ids = ser_list.edge_ids();
            incidence_list[list_count++] = {edges_ids.begin(), edges_ids.end()};
        }

        return {edges, incidence_list};
    }

    graph::Router<double>::RoutesInternalData
    Serialize::ParseRouter(const request_handler::RequestHandler& tc_handler,
                           const ::tc_serialize::RoutingSettings& routing_settings)
    {
        auto serialized_routes_internal_data = routing_settings.routes_internal_data();
        graph::Router<double>::RoutesInternalData routes_internal_data(serialized_routes_internal_data.size());

        int outer = 0;
        for (const auto& ser_outher_data : serialized_routes_internal_data)
        {
            auto& internal_data = routes_internal_data[outer++];
            const auto& ser_inner_data = ser_outher_data.route_internal_data_();
            internal_data.reserve(ser_inner_data.size());
            for (const auto& ser_data : ser_inner_data)
            {
                std::optional<graph::Router<double>::RouteInternalData> data;
                if (ser_data.is_empty())
                {
                    data = std::nullopt;
                }
                else
                {
                    graph::Router<double>::RouteInternalData pure_data;

                    pure_data.weight = ser_data.weight();
                    if (ser_data.prev_edge() != -1)
                    {
                        pure_data.prev_edge.emplace(ser_data.prev_edge());
                    }
                    else
                    {
                        pure_data.prev_edge = std::nullopt;
                    }
                    data.emplace(std::move(pure_data));
                }
                internal_data.push_back(std::move(data));
            }
        }

        return routes_internal_data;
    }

    tc_serialize::RoutingSettings Serialize::SerializeRoutingSettings(const request_handler::RequestHandler& tc_handler,
                                                                      const graph::DirectedWeightedGraph<double>& graph,
                                                                      const graph::Router<double>& router)
    {

        tc_serialize::RoutingSettings routing_settings;
        *routing_settings.mutable_graph() = SerializeGraph(tc_handler, graph);
        *routing_settings.mutable_routes_internal_data() = SerializeRouter(tc_handler, router);
        return std::move(routing_settings);
    }
}