#pragma once

#include "transport_catalogue.h"

#include <vector>


namespace request_handler
{
	class RequestHandler final
	{
	public:
		RequestHandler() = delete;
		RequestHandler(tc::TransportCatalogue& transp_cat)
			: transp_cat_(transp_cat)
		{}
		RequestHandler(const RequestHandler&) = delete;
		RequestHandler(RequestHandler&&) = delete;
		RequestHandler& operator=(const RequestHandler&) = delete;
		RequestHandler& operator=(RequestHandler&& other) = delete;
		~RequestHandler() = default;

		void AddStops(std::vector<tc::StopParams>& stops);
		void AddBuses(std::vector<tc::BusParams>& buses);

		std::optional<tc::BusInfo> GetBusInfo(std::string bus_name) const;
		std::optional<tc::StopInfo> GetStopInfo(std::string stop_name) const;
		const std::deque<domain::Bus>& GetAllBuses(void) const;
	private:
		tc::TransportCatalogue& transp_cat_;
	};
}
