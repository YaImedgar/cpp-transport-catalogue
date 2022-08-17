#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>
#include <string_view>

using namespace std::literals;
using namespace std;

void PrintUsage(std::ostream& stream = std::cout) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    tc::TransportCatalogue tc;
    json_reader::JSONReader json_reader(tc);
    const std::string_view mode(argv[1]);
    if (mode == "make_base"sv)
    {
        if (!json_reader.MakeBase(std::cin))
        {
            cerr << "Failed to Save base" << endl;
        }
    }
    else if (mode == "process_requests"sv)
    {
        if (!json_reader.ProcessRequests(std::cin))
        {
            cerr << "Failed to Load base" << endl;
        }
    }
    else
    {
        PrintUsage();
        return 1;
    }
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
