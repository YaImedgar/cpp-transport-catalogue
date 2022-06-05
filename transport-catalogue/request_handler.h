#pragma once

#include "transport_catalogue.h"

#include <vector>


namespace request_handler
{
    class TCHandler final
    {
    public:
        TCHandler() = delete;
        TCHandler(tc::TransportCatalogue& transp_cat)
                  : transp_cat_(transp_cat)
        {}

        TCHandler( const TCHandler& ) = delete;
        TCHandler( TCHandler&& ) = default;

        TCHandler& operator=(const TCHandler& ) = delete;
        TCHandler& operator=( TCHandler&& other ) = default;

        ~TCHandler() = default;

        void AddStops(std::vector<tc::StopParams>& stops);
        void AddBuses(std::vector<tc::BusParams>& buses);

        std::optional<tc::BusInfo> GetBusInfo( std::string bus_name ) const;
        std::optional<tc::StopInfo> GetStopInfo( std::string stop_name ) const;
        const std::deque<domain::Bus>& GetAllBuses( void ) const;
    private:
        tc::TransportCatalogue& transp_cat_;
    };
}
