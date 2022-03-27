#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

using std::cin;
using std::getline;
using std::cout;
using std::ostream;
using std::endl;

#include "transport_catalogue.h"

namespace tc
{
	class TransportCatalogue;
	namespace output
	{
		void ReceiveRequest(const TransportCatalogue& transpCat);
	}
}