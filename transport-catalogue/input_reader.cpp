#include "input_reader.h"

namespace tc
{
	namespace input
	{
		void GetDataFromSS(TransportCatalogue& transpCat)
		{
			int num_of_queries;
			cin >> num_of_queries;

			{
				string blank; //TODO fix with getnumber
				getline(cin, blank);
			}

			vector<string> queries;

			string query;
 			while (num_of_queries--)
			{
				getline(cin, query);
				queries.push_back(query);
			}

			sort(queries.begin(), queries.end(), std::greater<string>());

			for (string& query_str : queries)
			{
				std::stringstream ss_query;

				ss_query << query_str;
				if (query_str.compare(0, 5, "Stop ") == 0)
				{
					StopParams stop = ParseStop(ss_query);
					transpCat.AddStop(move(stop));
				}
				else if (query_str.compare(0, 4, "Bus ") == 0)
				{
					bool is_cycle_route = query_str.find_first_of('-') != string::npos;
					BusParams bus_to_stops = ParseBus(ss_query, is_cycle_route);
					transpCat.AddBus(move(bus_to_stops));
				}
			}
		}

		StopParams ParseStop(std::stringstream& stop_query)
		{
			string stop_name;
			getline(stop_query, stop_name, ':');

			if (stop_name.compare(0, 5, "Stop ") == 0)
			{
				stop_name = stop_name.substr(5);
			}

			double latitude = detail::GetDblFromSS(stop_query);
			double longitude = detail::GetDblFromSS(stop_query);

			string distance;
			unordered_map<string, long> distance_to_other_stop;
			while (getline(stop_query, distance, ','))
			{
				long dist = stol(distance);
				size_t to = distance.find("to ") + 3;

				string stop_name_to_other = distance.substr(to, distance.length() - to - (distance.find_last_of('\n') != string::npos));

				distance_to_other_stop.emplace(stop_name_to_other, dist);
			}

			return { stop_name, Coordinates{latitude, longitude}, distance_to_other_stop };
		}

		BusParams ParseBus(std::stringstream& bus_query, bool is_cycle_route)
		{
			string bus_name;
			getline(bus_query, bus_name, ':');

			if (bus_name.compare(0, 4, "Bus ") == 0)
			{
				bus_name = bus_name.substr(4);
			}

			list<string> stops;

			string stop_name;
			while (getline(bus_query, stop_name, (is_cycle_route ? '-' : '>')))
			{
				size_t first_pos = stop_name.find_first_not_of(' ');
				stops.push_back(stop_name.substr(first_pos, stop_name.find_last_not_of(' ') - (stop_name.find_last_of('\n') != string::npos)));
			}

			if (is_cycle_route)
			{
				stops.insert(stops.end(), next(stops.rbegin()), stops.rend());
			}

			return { bus_name, stops };
		}
		namespace detail
		{
			double GetDblFromSS(std::stringstream& ss)
			{
				string str;
				getline(ss, str, ',');
				/*size_t dot_pos = str.find_first_of('.');
				if (dot_pos != string::npos)
				{
					str[dot_pos] = ',';
				}*/

				return stod(str);
			}
		}
	}
}