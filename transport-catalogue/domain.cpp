#include "domain.h"

namespace domain
{
    Bus::Bus( size_t id, std::string&& name, std::vector<domain::Stop*>&& stops, bool is_round )
        : id_(id)
        , name_( std::move( name ) )
        , stops_( std::move( stops ) )
        , total_stops_( stops_.size() )
        , total_unique_stops_( std::set<domain::Stop*>( stops_.begin(), stops_.end() ).size() )
        , is_round_route_( is_round )
    {
        route_length_straight_ = 0;
        route_length_normal_ = 0;
        for ( size_t i = 0; i < stops_.size() - 1; i++ )
        {
            route_length_straight_ += ComputeDistance( stops_[i]->GetCoordinates(),
                                                       stops_[i + 1]->GetCoordinates() );

            std::string cur_stop = std::string( stops_[i]->GetName() );
            std::string next_stop = std::string( stops_[i + 1]->GetName() );

            std::optional<double> dist_to = stops_[i]->GetDistToOtherStop( next_stop );
            if ( dist_to.has_value() )
            {
                route_length_normal_ += dist_to.value();
            }
            else
            {
                route_length_normal_ += stops_[i + 1]->GetDistToOtherStop( cur_stop ).value();
            }
        }
    }

    std::string_view Bus::GetName() const
    {
        return name_;
    }

    int Bus::GetTotalStops() const
    {
        return total_stops_;
    }

    int Bus::GetTotalUniqueStops() const
    {
        return total_unique_stops_;
    }

    bool Bus::IsRoundRoute() const
    {
        return is_round_route_;
    }

    double Bus::GetRouteLength() const
    {
        return route_length_normal_;
    }

    double Bus::GetRouteLengthStraight() const
    {
        return route_length_straight_;
    }

    double Bus::GetCurvature() const
    {
        return GetRouteLength() / GetRouteLengthStraight();
    }

    const std::vector<Stop*> Bus::GetStops() const
    {
        return stops_;
    }

    void Bus::AddStop( Stop* stop )
    {
        stops_.emplace_back( stop );
    }

    size_t Bus::GetId() const {
        return id_;
    }

    Stop::Stop( domain::StopParams&& stop )
        : name_( std::move( stop.name ) )
        , coordinates_( std::move( stop.coordinates ) )
        , distance_to_other_stops_( std::move( stop.dist_to_other_stop ) )
        , id_( stop.id )
    {}

    std::string_view Stop::GetName() const
    {
        return name_;
    }

    void Stop::AddBus( std::string bus )
    {
        buses_.insert( bus );
    }

    geo::Coordinates Stop::GetCoordinates() const
    {
        return coordinates_;
    }

    const std::set<std::string>& Stop::GetBuses() const
    {
        return buses_;
    }

    void Stop::AddDistToOtherStop( std::string_view stop, long distance )
    {
        distance_to_other_stops_[stop] = distance;
    }

    std::optional<double> Stop::GetDistToOtherStop( std::string_view other_stop ) const
    {
        if ( distance_to_other_stops_.count( other_stop ) )
        {
            return { distance_to_other_stops_.at( other_stop ) };
        }
        else
        {
            return std::nullopt;
        }
    }

    void Stop::SetCoordinates( geo::Coordinates&& coordinates )
    {
        coordinates_ = std::move( coordinates );
    }

    size_t Stop::GetId() const
    {
        return id_;
    }

    const std::unordered_map<std::string_view, int> &Stop::GetAllDistanceToOtherStops() const {
        return distance_to_other_stops_;
    }
}