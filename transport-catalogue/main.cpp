#include "json_reader.h"
#include "map_renderer.h"

int main()
{
    transport::TransportCatalogue transport_catalogue;
	transport::json_reader::JsonReader json_reader(transport_catalogue);
	json_reader.FillCatalogue(std::cin);
	json_reader.OutputData(std::cout);
}