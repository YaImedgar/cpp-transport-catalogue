#include "map_renderer.h"

namespace map_render
{
    std::stringstream MapRender::Render( const MapSettings& ms,
                                   const std::deque<domain::Bus>& buses )
    {
        svg::Document doc;
        std::map<std::string_view, const domain::Bus*> sorted_buses;
        std::map<std::string_view, const domain::Stop*> all_stops;
        std::vector<geo::Coordinates> coordinates;

        for ( const domain::Bus& bus : buses )
        {
            if ( bus.GetTotalStops() > 0 )
            {
                sorted_buses.emplace( bus.GetName(), &bus );
                const auto& stops = bus.GetBusesStops();
                for ( const auto& stop : bus.GetBusesStops() )
                {
                    all_stops.emplace( stop->GetName(), stop );
                    coordinates.push_back( stop->GetCoordinates() );
                }
            }
        }

        SphereProjector sphere_proj( coordinates.begin(), coordinates.end(), ms.width, ms.height, ms.padding );

        RenderRouteLines( ms, doc, sorted_buses, sphere_proj );
        RenderRouteNames( ms, doc, sorted_buses, sphere_proj );
        RenderCircles( ms, doc, all_stops, sphere_proj );
        RenderStopNames( ms, doc, all_stops, sphere_proj );

        std::stringstream ss;
        doc.Render( ss );
        return ss;
    }
    void MapRender::RenderRouteLines( const MapSettings& ms,
                                      svg::Document& doc,
                                      const std::map<std::string_view, const domain::Bus*>& sorted_buses,
                                      const SphereProjector& sphere_proj )
    {
        const auto bus_colors_ptr = ms.color_palette.begin();
        int i = 0;

        for ( const auto& [name, bus] : sorted_buses )
        {
            svg::Polyline bus_svg;

            if ( i >= ms.color_palette.size() )
            {
                i = 0;
            }
            bus_svg.SetStrokeColor( bus_colors_ptr[i] );
            ++i;

            bus_svg.SetStrokeWidth( ms.line_width );
            bus_svg.SetFillColor( svg::NoneColor );
            bus_svg.SetStrokeLineJoin( svg::StrokeLineJoin::ROUND );
            bus_svg.SetStrokeLineCap( svg::StrokeLineCap::ROUND );
            for ( const auto& stop : bus->GetBusesStops() )
            {
                bus_svg.AddPoint( sphere_proj( stop->GetCoordinates() ) );
            }

            doc.Add( bus_svg );
        }
    }

    void MapRender::RenderRouteNames( const MapSettings& ms, svg::Document& doc, const std::map<std::string_view, const domain::Bus*>& sorted_buses, const SphereProjector& sphere_proj )
    {
        const auto bus_colors_ptr = ms.color_palette.begin();
        int i = 0;

        for ( const auto& [name, bus] : sorted_buses )
        {
            if ( i >= ms.color_palette.size() )
            {
                i = 0;
            }

            svg::Color bus_color = bus_colors_ptr[i];
            ++i;

            geo::Coordinates stop_coord_first = bus->GetBusesStops().front()->GetCoordinates();
            svg::Point location = sphere_proj( stop_coord_first );

            AddRouteName( ms, doc, name, location, bus_color );

            if ( !bus->IsRoundRoute() )
            {
                const auto& stops = bus->GetBusesStops();
                geo::Coordinates stop_coord = stops[stops.size() / 2]->GetCoordinates();
                if ( stop_coord_first != stop_coord )
                {
                    svg::Point location = sphere_proj( stop_coord );

                    AddRouteName( ms, doc, name, location, bus_color );
                }
            }
        }
    }

    void MapRender::RenderCircles( const MapSettings& ms, svg::Document& doc, const std::map<std::string_view, const domain::Stop*>& all_stops, const SphereProjector& sphere_proj )
    {
        for ( const auto& [_, stop] : all_stops )
        {
            svg::Circle circle;

            circle.SetCenter( sphere_proj( stop->GetCoordinates() ) );
            circle.SetRadius( ms.stop_radius );
            circle.SetFillColor( "white"s );

            doc.Add( circle );
        }
    }

    void MapRender::RenderStopNames( const MapSettings& ms, svg::Document& doc, const std::map<std::string_view, const domain::Stop*>& all_stops, const SphereProjector& sphere_proj )
    {
        for ( const auto& [name, stop] : all_stops )
        {
            svg::Text stop_name;
            svg::Text background;

            svg::Point location = sphere_proj( stop->GetCoordinates() );

            SetCommonParams( stop_name, ms, name, location, false );
            SetCommonParams( background, ms, name, location, false );

            {// route name additional params
                stop_name.SetFillColor( "black"s );
            }

            {// background additional params
                background.SetFillColor( ms.underlayer_color );
                background.SetStrokeColor( ms.underlayer_color );
                background.SetStrokeWidth( ms.underlayer_width );
                background.SetStrokeLineCap( svg::StrokeLineCap::ROUND );
                background.SetStrokeLineJoin( svg::StrokeLineJoin::ROUND );
            }

            doc.Add( background );
            doc.Add( stop_name );
        }
    }

    void MapRender::SetCommonParams( svg::Text& text, const MapSettings& ms, std::string_view name, svg::Point location, bool is_route )
    {
        text.SetPosition( location );

        if ( is_route )
        {
            text.SetOffset( ms.bus_label_offset );
            text.SetFontSize( ms.bus_label_font_size );
            text.SetFontWeight( "bold" );
        }
        else
        {
            text.SetOffset( ms.stop_label_offset );
            text.SetFontSize( ms.stop_label_font_size );
        }

        text.SetFontFamily( "Verdana" );
        text.SetData( std::string( name ) );
    }

    void MapRender::AddRouteName( const MapSettings& ms, svg::Document& doc, std::string_view name, svg::Point location, svg::Color bus_color )
    {
        svg::Text route_name;
        svg::Text background;

        SetCommonParams( route_name, ms, name, location, true );
        SetCommonParams( background, ms, name, location, true );

        {// route name additional params
            route_name.SetFillColor( bus_color );
        }

        {// background additional params
            background.SetFillColor( ms.underlayer_color );
            background.SetStrokeColor( ms.underlayer_color );
            background.SetStrokeWidth( ms.underlayer_width );
            background.SetStrokeLineCap( svg::StrokeLineCap::ROUND );
            background.SetStrokeLineJoin( svg::StrokeLineJoin::ROUND );
        }

        doc.Add( background );
        doc.Add( route_name );
    }
}
