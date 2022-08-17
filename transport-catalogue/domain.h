#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <tuple>
#include <optional>	
#include <set>	
#include <vector>	
#include <list>	
#include <unordered_map>	

namespace domain
{
    class Stop;
    class Bus;

    struct StopParams
    {
        std::string name;
        geo::Coordinates coordinates{};
        std::unordered_map<std::string_view, int> dist_to_other_stop{};
        std::unordered_map<int, int> dist_to_other_stop_ids{};
        size_t id{};
    };

    struct BusParams
    {
        std::string name;
        std::list<std::string> stops_list;
        bool is_route = false;
        size_t id{};
    };

    class Stop
    {
    public:
        explicit Stop( StopParams&& stop );

        Stop() = delete;
        Stop( const Stop& other ) = delete;
        Stop& operator=( const Stop& other ) = delete;

        Stop( Stop&& other ) = delete;
        Stop& operator=( Stop&& other ) = delete;

        std::string_view GetName() const;
        geo::Coordinates GetCoordinates() const;
        std::optional<double> GetDistToOtherStop( std::string_view other_stop ) const;
        const std::unordered_map<std::string_view, int>& GetAllDistanceToOtherStops() const;
        const std::set<std::string>& GetBuses() const;
        size_t GetId() const;

        void AddBus( std::string bus );
        void AddDistToOtherStop( std::string_view stop, long distance );
        void SetCoordinates( geo::Coordinates&& coordinates );

    private:
        size_t id_;
        std::string name_;
        geo::Coordinates coordinates_{};
        std::unordered_map<std::string_view, int> distance_to_other_stops_{};
        std::set<std::string> buses_{};
    };

    class Bus
    {
    public:
        explicit Bus( size_t id,
                      std::string&& name,
                      std::vector<Stop*>&& stops,
                      bool is_round );

        Bus() = delete;
        Bus( const Bus& other ) = delete;
        Bus& operator=( const Bus& other ) = delete;

        Bus( Bus&& other ) = delete;
        Bus& operator=( Bus&& other ) = delete;

        size_t GetId() const;
        std::string_view GetName() const;
        int GetTotalStops() const;
        int GetTotalUniqueStops() const;
        bool IsRoundRoute() const;
        double GetRouteLength() const;
        double GetRouteLengthStraight() const;
        double GetCurvature() const;
        const std::vector<Stop*> GetStops() const;
        void AddStop( Stop* stop );

    private:
        size_t id_;
        std::string name_;
        std::vector<Stop*> stops_;
        int total_stops_;
        int total_unique_stops_;
        double route_length_normal_;
        double route_length_straight_;
        bool is_round_route_ = false;
    };
} // end domain