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

using namespace tc::geo;

namespace tc
{
	class TransportCatalogue;
	typedef std::tuple<std::string/*Stop Name*/, tc::geo::Coordinates, std::unordered_map<std::string, long>/*Distance to other bus stop*/> StopParams;
	typedef std::tuple<std::string/*Bus Name*/, std::list<std::string>/*Stops names*/> BusParams;

	namespace input
	{
		namespace detail
		{
			double GetDblFromSS(std::stringstream& ss);
		}

		void GetDataFromSS(TransportCatalogue& transpCat);
		StopParams ParseStop(std::stringstream& stop_query);
		BusParams ParseBus(std::stringstream& bus_query, bool is_cycle_route);
	}
}