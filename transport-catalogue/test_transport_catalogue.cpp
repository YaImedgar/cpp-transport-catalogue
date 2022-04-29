#include "test_transport_catalogue.h"

using namespace std::string_literals;
/*
namespace tc
{
	namespace test
	{
		void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
			const std::string& hint)
		{
			if (!value)
			{
				std::cerr << file << "("s << line << "s): "s << func << ": "s;
				std::cerr << "ASSERT("s << expr_str << "s) failed."s;
				if (!hint.empty())
				{
					std::cerr << "s Hint: "s << hint;
				}
				std::cerr << std::endl;
 				abort();
			}
		}

		namespace input
		{
			void ParseStop()
			{
				{
					stringstream ss;
					ss << "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s << endl;
					StopParams stop = tc::input::ParseStop(ss);
					StopParams test_tuple = { "Tolstopaltsevo"s, { 55.611087 , 37.20829 }, { { "Marushkino", 3900 } } };
					ASSERT(stop == test_tuple);
				}

				{
					stringstream ss;
					ss << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"s << endl;
					StopParams stop = tc::input::ParseStop(ss);
					StopParams test_tuple = { "Biryulyovo Zapadnoye"s, { 55.574371 , 37.6517 }, { { "Rossoshanskaya ulitsa", 7500}, { "Biryusinka", 1800}, { "Universam", 2400}  } };
					ASSERT(stop == test_tuple);
				}

				{
					stringstream ss;
					ss << "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757"s << endl;
					StopParams stop = tc::input::ParseStop(ss);
					StopParams test_tuple = { "Rossoshanskaya ulitsa"s, { 55.595579 , 37.605757 }, { } };
					ASSERT(stop == test_tuple);
				}
			}

			void ParseBus()
			{
				{
					stringstream ss;
					ss << "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl;
					bool is_cycle_route = false;
					BusParams stop = tc::input::ParseBus(ss, is_cycle_route);
					BusParams test_tuple = { "828"s, {"Biryulyovo Zapadnoye"s, "Universam"s, "Rossoshanskaya ulitsa"s, "Biryulyovo Zapadnoye"s } };
					ASSERT(stop == test_tuple);
				}

				{
					stringstream ss;
					ss << "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka"s << endl;
					bool is_cycle_route = true;
					BusParams stop = tc::input::ParseBus(ss, is_cycle_route);
					BusParams test_tuple = { "750"s, {"Tolstopaltsevo"s, "Marushkino"s, "Marushkino"s, "Rasskazovka"s, "Marushkino"s, "Marushkino"s, "Tolstopaltsevo"s }};
					ASSERT(stop == test_tuple);
				}
			} 

			void TestAll()
			{
				RUN_TEST(ParseStop);
				RUN_TEST(ParseBus);
			}
		}

		namespace core
		{

			void GetTestStops(stringstream& ss)
			{
				ss << "13"s << endl;
				ss << "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl;
				ss << "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka"s << endl;
				ss << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << endl;
				ss << "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s << endl;
				ss << "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s << endl;
				ss << "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s << endl;
				ss << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"s << endl;
				ss << "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam"s << endl;
				ss << "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya"s << endl;
				ss << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya"s << endl;
				ss << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye"s << endl;
				ss << "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757"s << endl;
				ss << "Stop Prazhskaya: 55.611678, 37.603831"s << endl;
			}

			void AddStop()
			{
				tc::TransportCatalogue tc;

				stringstream ss;
				GetTestStops(ss);

				tc::input::ReadTransportCatalogue(tc, ss);
				{
					domain::Stop* stop = tc.FindStop("Tolstopaltsevo"s);
					ASSERT(stop->GetName() == "Tolstopaltsevo"s);
					ASSERT(stop->GetCoordinates() == Coordinates({ 55.611087, 37.20829 }) );
					ASSERT(stop->GetBuses() == std::set<std::string>{"750"s});
				}

				{
					domain::Stop* stop = tc.FindStop("FakeStop"s);
					ASSERT(stop == nullptr);
				}
			}

			void AddBus()
			{
				tc::TransportCatalogue tc;

				stringstream ss;
				GetTestStops(ss);

				tc::input::ReadTransportCatalogue(tc, ss);
				{
					domain::Bus* bus = tc.FindBus("256"s);
					ASSERT(bus->GetName() == "256"s);
					ASSERT(bus->GetTotalStops() == 6);
					ASSERT(bus->GetTotalUniqueStopst() == 5);
					ASSERT(bus->GetRouteLength() == 5950.0);
					ASSERT(bus->GetCurvature() == double(1.3612391910001966));
				}

				{
					domain::Bus* bus = tc.FindBus("750"s);
					ASSERT(bus->GetName() == "750"s);
					ASSERT(bus->GetTotalStops() == 7);
					ASSERT(bus->GetTotalUniqueStopst() == 3);
					ASSERT(bus->GetRouteLength() == 27400.0);
					ASSERT(bus->GetCurvature() == double(1.3085327817704286));
				}

				{
					domain::Bus* bus = tc.FindBus("751"s);
					ASSERT(bus == nullptr);
				}
			}

			void TestAll()
			{
				RUN_TEST(AddStop);
				RUN_TEST(AddBus);
			}
		}

		namespace output
		{
			void TestAll()
			{
			}
		}

		void TransportCatalogue()
		{
			RUN_TEST(input::TestAll);
			RUN_TEST(core::TestAll);
			RUN_TEST(output::TestAll);
		}
	}
}*/