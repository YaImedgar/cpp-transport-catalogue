#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <tuple>
#include <list>
#include <unordered_map>
#include <sstream>

using std::cin;
using std::getline;
using std::stod;
using std::stol;
using std::string;
using std::list;
using std::tuple;

using namespace geo;

/*namespace tc
{
	namespace input
	{
		namespace detail
		{
			double ParseDouble(std::stringstream& ss);
		}

		void ReadTransportCatalogue(tc::TransportCatalogue& transp_cat, std::istream& input);
		StopParams ParseStop(std::stringstream& stop_query);
		BusParams ParseBus(std::stringstream& bus_query, bool is_cycle_route);
	}
}*/