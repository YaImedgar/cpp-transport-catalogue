#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "graph.h"
#include "router.h"
#include "request_handler.h"

#include <transport_catalogue.pb.h>
#include <list>
#include <iostream>
#include <string>
#include <fstream>
#include <ostream>
#include <optional>

namespace request_handler
{
    class RequestHandler;
}

namespace tc_serialize
{
    class Serialize final
    {
    public:
        static bool SaveTo(std::string&& path, tc_serialize::TransportCatalogue&& serialize_tc);
        static std::optional<tc_serialize::TransportCatalogue> ParseFrom(std::string&& path);
        static std::vector<tc_serialize::Bus> BusesToSerialize(const tc::TransportCatalogue& tc);
        static std::vector<tc_serialize::Stop> StopsToSerialize(const tc::TransportCatalogue& tc);
        static tc_serialize::RenderSettings RenderSettingsToSerialize(const map_render::MapSettings& settings);
        static tc_serialize::Point GetPoint(svg::Point point);
        static tc_serialize::Color SetColor(svg::Color color);
        static map_render::MapSettings ParseMapFromSerialized(const tc_serialize::RenderSettings& serialized);
        static svg::Point GetPoint(tc_serialize::Point point);
        static svg::Color GetColor(const tc_serialize::Color& color);
        static std::vector<svg::Color> GetColors(const google::protobuf::RepeatedPtrField<::tc_serialize::Color> &ser_colors);
        static tc_serialize::Graph SerializeGraph(const request_handler::RequestHandler& tc_handler,
                                                  const graph::DirectedWeightedGraph<double>& graph);
        static tc_serialize::Edge SerializeEdge(const graph::Edge<double> &edge, const request_handler::RequestHandler& tc_handler);
        static graph::DirectedWeightedGraph<double> ParseGraph(const request_handler::RequestHandler& tc_handler,
                                                               const tc_serialize::Graph& serialized_graph);
        static graph::Router<double>::RoutesInternalData ParseRouter(const request_handler::RequestHandler& tc_handler,
                                                                      const ::tc_serialize::RoutingSettings& routing_settings);
        static google::protobuf::RepeatedPtrField<tc_serialize::RoutesInternalData>
        SerializeRouter(const request_handler::RequestHandler& tc_handler, const graph::Router<double>& router);
        static tc_serialize::RoutingSettings SerializeRoutingSettings(const request_handler::RequestHandler& tc_handler,
                                                                      const graph::DirectedWeightedGraph<double>& graph,
                                                                      const graph::Router<double>& router);
    private:
    };
}