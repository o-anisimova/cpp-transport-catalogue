#include "request_handler.h"

using namespace std;

namespace transport {

	RequestHandler::RequestHandler(const TransportCatalogue& transport_catalogue, const renderer::MapRenderer& renderer)
    :transport_catalogue_(transport_catalogue), map_renderer_(renderer) {
	}

	std::optional<BusStat> RequestHandler::GetBusStat(const string_view& bus_name) const {
		return transport_catalogue_.GetBusStat(bus_name);
	}

    svg::Document RequestHandler::RenderMap() const {
		vector<const Bus*> bus_list = transport_catalogue_.GetBusListSorted();
		vector<const Stop*> stop_list = transport_catalogue_.GetStopListSorted();
		return map_renderer_.RenderMap(bus_list, stop_list);
    }

} // namespace transport