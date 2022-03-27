#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"
#include "geo.h"

using namespace std;

int main()
{
	tc::TransportCatalogue transpCat;

	tc::input::GetDataFromSS(transpCat);
	tc::output::ReceiveRequest(transpCat);

	return 0;
}

//#include "test_transport_catalogue.h"
//
//using namespace std;
//
//int main()
//{
//    setlocale(LC_ALL, "Russian");
//    try
//    {
//        tc::test::TransportCatalogue();
//    }
//    catch (const std::exception& e)
//    {
//        std::cout << e.what() << std::endl;
//    }
//    // Если вы видите эту строку, значит все тесты прошли успешно
//    std::cout << "Transport catalogue testing finished"s << std::endl;
//}
