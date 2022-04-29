#include "stat_reader.h"

using namespace std::string_literals;

namespace tc
{
	namespace output
	{
		std::vector<std::string> Requests::ReadRequests(std::istream& input)
		{
			int num_of_queries;
			input >> num_of_queries;

			std::string blank; //TODO fix with getnumber
			getline(input, blank);

			std::vector<std::string> queries;
			queries.reserve(num_of_queries);

			std::string query;
			while (num_of_queries--)
			{
				getline(input, query);
				queries.push_back(query);
			}
			return queries;
		}

		void Requests::HandleRequests(std::ostream& output, std::vector<std::string> queries)
		{
			for (std::string& query_str : queries)
			{
				if (query_str.compare(0, 5, "Stop ") == 0)
				{
					std::optional<tc::StopInfo> stop_info = transp_cat_->GetStopInfo(&query_str[5]);

					output << query_str << ": ";
					if (stop_info.has_value())
					{
						std::set<std::string> buses = stop_info.value().buses;
						if (!buses.empty())
						{
							output << "buses";
							for (const std::string& bus : buses)
							{
								output << " " << bus;
							}
							output << endl;
						}
						else
						{
							output << "no buses" << endl;
						}
					}
					else
					{
						output << "not found" << endl;
					}
				}
				else if (query_str.compare(0, 4, "Bus ") == 0)
				{
					std::optional<tc::BusInfo> bus_info = transp_cat_->GetBusInfo(&query_str[4]);

					output << query_str << ": ";
					if (bus_info.has_value())
					{
						tc::BusInfo b_i = bus_info.value();
						output << b_i.total_stops << " stops on route, "s <<
							b_i.total_unique_stops << " unique stops, "s << std::setprecision(6) <<
							b_i.route_length << " route length, "s <<
							b_i.curvature << " curvature"s << endl;
					}
					else
					{
						output << "not found" << endl;
					}
				}
			}
		}
	}
}
