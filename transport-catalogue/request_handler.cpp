#include "request_handler.h"

using namespace std;

namespace transport {

	RequestHandler::RequestHandler(const TransportCatalogue& transport_catalogue, const renderer::MapRenderer& renderer)
    :transport_catalogue_(transport_catalogue), map_renderer_(renderer) {
	}

	std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	    Bus* bus = transport_catalogue_.FindBus(bus_name);
		if (bus == nullptr) {
			return {};
		}

		BusStat bus_stat;
		vector<const Stop*> bus_full_route = GetBusFullRoute(bus);
		bus_stat.stops_qty = static_cast<int>(bus_full_route.size());

		unordered_set<string> unique_stops;
		double geographic_route_length = 0.0;
		for (int i = 0; i < bus_stat.stops_qty - 1; ++i) {
			unique_stops.insert(bus_full_route[i]->stop_name);
			geographic_route_length += ComputeDistance(bus_full_route[i]->coordinates, bus_full_route[i + 1]->coordinates);
			bus_stat.route_length += transport_catalogue_.GetDistanceBetweenStops(bus_full_route[i], bus_full_route[i + 1]);
		}
		unique_stops.insert(bus_full_route[bus_stat.stops_qty - 1]->stop_name);

		bus_stat.unique_stops_qty = static_cast<int>(unique_stops.size());
		bus_stat.curvature = bus_stat.route_length / geographic_route_length;

		return bus_stat;
	}

    svg::Document RequestHandler::RenderMap() const {
		vector<const Bus*> bus_list = transport_catalogue_.GetBusListSorted();
		vector<const Stop*> stop_list = transport_catalogue_.GetStopListSorted();
		return map_renderer_.RenderMap(bus_list, stop_list);
    }

} // namespace transport