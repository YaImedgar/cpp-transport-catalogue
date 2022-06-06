#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std;

int main()
{
	tc::TransportCatalogue transp_catalogue;
	map_render::MapSettings map_settings;

	json_reader::JSONReader json_reader(transp_catalogue, map_settings);
	json_reader.ReadRequestFrom(std::cin);

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
