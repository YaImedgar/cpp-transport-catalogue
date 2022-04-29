#pragma once

#include "transport_catalogue.h"

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

using std::cin;
using std::getline;
using std::cout;
using std::ostream;
using std::endl;

namespace tc
{
	class TransportCatalogue;
	namespace output
	{
		class Requests
		{
		public:
			explicit Requests(const TransportCatalogue& transp_cat)
				: transp_cat_(&transp_cat)
			{}

			std::vector<std::string> ReadRequests(std::istream& input);
			void HandleRequests(std::ostream& output, std::vector<std::string> queries);
		private:
			const TransportCatalogue* const transp_cat_;
		};
	}
}