#include "json_reader.h"
#include "map_renderer.h"

int main()
{
    transport::TransportCatalogue transport_catalogue;
	transport::renderer::MapRenderer map_renderer;
	transport::json_reader::JsonReader json_reader(transport_catalogue, map_renderer);
	json_reader.FillCatalogue(std::cin);
	json_reader.OutputData(std::cout);
}