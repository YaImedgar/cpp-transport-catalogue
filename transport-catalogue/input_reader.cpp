#include "input_reader.h"
/*
namespace tc
{
	namespace input
	{
		void ReadTransportCatalogue(TransportCatalogue& transp_cat, std::istream& input)
		{
			int num_of_queries;
			input >> num_of_queries;

			{
				std::string blank; //TODO fix with getnumber
				getline(input, blank);
			}

			std::vector<std::string> queries;

			std::string query;
			while (num_of_queries--)
			{
				getline(input, query);
				queries.push_back(query);
			}

			sort(queries.begin(), queries.end(), std::greater<std::string>());

			for (std::string& query_str : queries)
			{
				std::stringstream ss_query;

				ss_query << query_str;
				if (query_str.compare(0, 5, "Stop ") == 0)
				{
					StopParams stop = ParseStop(ss_query);
					transp_cat.AddStop(move(stop));
				}
				else if (query_str.compare(0, 4, "Bus ") == 0)
				{
					bool is_cycle_route = query_str.find_first_of('-') != std::string::npos;
					BusParams bus_to_stops = ParseBus(ss_query, is_cycle_route);
					transp_cat.AddBus(move(bus_to_stops));
				}
			}
		}

		StopParams ParseStop(std::stringstream& stop_query)
		{
			std::string stop_name;
			getline(stop_query, stop_name, ':');

			if (stop_name.compare(0, 5, "Stop ") == 0)
			{
				stop_name = stop_name.substr(5);
			}

			double latitude = detail::ParseDouble(stop_query);
			double longitude = detail::ParseDouble(stop_query);

			std::string distance;
			std::unordered_map<std::string, long> distance_to_other_stop;
			while (getline(stop_query, distance, ','))
			{
				long dist = stol(distance);
				size_t to = distance.find("to ") + 3;

				std::string stop_name_to_other = distance.substr(to, distance.length() - to - (distance.find_last_of('\n') != std::string::npos));

				distance_to_other_stop.emplace(stop_name_to_other, dist);
			}

			return { stop_name, Coordinates{latitude, longitude}, distance_to_other_stop };
		}

		BusParams ParseBus(std::stringstream& bus_query, bool is_cycle_route)
		{
			std::string bus_name;
			getline(bus_query, bus_name, ':');

			if (bus_name.compare(0, 4, "Bus ") == 0)
			{
				bus_name = bus_name.substr(4);
			}

			list<std::string> stops;

			std::string stop_name;
			while (getline(bus_query, stop_name, (is_cycle_route ? '-' : '>')))
			{
				size_t first_pos = stop_name.find_first_not_of(' ');
				stops.push_back(stop_name.substr(first_pos, stop_name.find_last_not_of(' ') - (stop_name.find_last_of('\n') != std::string::npos)));
			}

			if (is_cycle_route)
			{
				stops.insert(stops.end(), next(stops.rbegin()), stops.rend());
			}

			return { bus_name, stops };
		}
		namespace detail
		{
			double ParseDouble(std::stringstream& ss)
			{
				std::string str;
				getline(ss, str, ',');

				size_t dot_pos = str.find_first_of('.');
				if (dot_pos != string::npos)
				{
					str[dot_pos] = ',';
				}

				return stod(str);
			}
		}
	}
}*/