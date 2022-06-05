#include "request_handler.h"

namespace request_handler
{
    void TCHandler::AddStops(std::vector<tc::StopParams> &stops)
    {
        for ( auto& stop : stops )
        {
            transp_cat_.AddStop( std::move(stop) );
        }
    }

    void TCHandler::AddBuses(std::vector<tc::BusParams> &buses)
    {
        for ( auto& bus : buses )
        {
            transp_cat_.AddBus( std::move(bus) );
        }
    }

    const std::deque<domain::Bus> &TCHandler::GetAllBuses(void) const
    {
        return transp_cat_.GetAllBuses();
    }

    std::optional<tc::StopInfo> TCHandler::GetStopInfo(std::string stop_name) const
    {
        return transp_cat_.GetStopInfo(stop_name);
    }

    std::optional<tc::BusInfo> TCHandler::GetBusInfo(std::string bus_name) const
    {
        return transp_cat_.GetBusInfo(bus_name);
    }
}
