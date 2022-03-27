#include "stat_reader.h"

using namespace std::string_literals;

namespace tc
{
	namespace output
	{
		void ReceiveRequest(const tc::TransportCatalogue& transpCat)
		{
			int num_of_queries;
			cin >> num_of_queries;

			string blank; //TODO fix with getnumber
			getline(cin, blank);

			vector<string> queries;
			queries.reserve(num_of_queries);

			string query;
			while (num_of_queries--)
			{
				getline(cin, query);
				queries.push_back(query);
			}

			for (string& query_str : queries)
			{
				if (query_str.compare(0, 5, "Stop ") == 0)
				{
					std::optional<tc::StopInfo> stop_info = transpCat.GetStopInfo(&query_str[5]);

					cout << query_str << ": ";
					if (stop_info.has_value())
					{
						set<string> buses = stop_info.value().buses;
						if (!buses.empty())
						{
							cout << "buses";
							for (const string& bus : buses)
							{
								cout << " " << bus;
							}
							cout << endl;
						}
						else
						{
							cout << "no buses" << endl;
						}
					}
					else
					{
						cout << "not found" << endl;
					}
				}
				else if (query_str.compare(0, 4, "Bus ") == 0)
				{
					std::optional<tc::BusInfo> bus_info = transpCat.GetBusInfo(&query_str[4]);

					cout << query_str << ": ";
					if (bus_info.has_value())
					{
						tc::BusInfo b_i = bus_info.value();
						cout << b_i.total_stops << " stops on route, "s <<
							b_i.total_unique_stops << " unique stops, "s << std::setprecision(6) <<
							b_i.route_length << " route length, "s <<
							b_i.curvature << " curvature"s << endl;
					}
					else
					{
						cout << "not found" << endl;
					}
				}
			}
		}
	}
}
