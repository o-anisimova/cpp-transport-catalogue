#include "request_handler.h"

using namespace std;

namespace transport {

	RequestHandler::RequestHandler(TransportCatalogue& transport_catalogue)
    :transport_catalogue_(transport_catalogue),transport_router_(transport_catalogue_){
	}

	void RequestHandler::AddStop(const Stop& stop) {
		transport_catalogue_.AddStop(stop);
	}

	void RequestHandler::AddBus(const Bus& bus) {
		transport_catalogue_.AddBus(bus);
	}

	void  RequestHandler::SetStopsDistance(Stop* lhs_stop, Stop* rhs_stop, int distance) {
		transport_catalogue_.SetStopsDistance(lhs_stop, rhs_stop, distance);
	}

	Stop* RequestHandler::FindStop(std::string_view stop_name) const {
		return transport_catalogue_.FindStop(stop_name);
	}

	const std::set<std::string_view>& RequestHandler::GetStopToBusesList(Stop* stop) const {
		return transport_catalogue_.GetStopToBusesList(stop);
	}

	std::optional<BusStat> RequestHandler::GetBusStat(const string_view& bus_name) const {
		return transport_catalogue_.GetBusStat(bus_name);
	}

	void RequestHandler::SetRenderSettings(const renderer::RenderSettings& render_settings) {
		map_renderer_.SetRenderSettings(render_settings);
	}

    svg::Document RequestHandler::RenderMap() const {
		vector<const Bus*> bus_list = transport_catalogue_.GetBusListSorted();
		vector<const Stop*> stop_list = transport_catalogue_.GetStopListSorted();
		return map_renderer_.RenderMap(bus_list, stop_list);
    }

	void RequestHandler::SetRoutingSettings(const RoutingSettings routing_settings) {
		transport_router_.SetRoutingSettings(routing_settings);
	}

	void RequestHandler::BuildGraph() {
		transport_router_.BuildGraph();
	}

	std::optional<Route> RequestHandler::BuildRoute(std::string_view from, std::string_view to) const {
		return transport_router_.BuildRoute(from, to);
	}

	void RequestHandler::Serialize(std::ofstream& output, const renderer::RenderSettings& settings, const RoutingSettings& routing_settings) const {
		SerializeCatalogue(output, transport_catalogue_, settings, routing_settings);
	}

	void RequestHandler::Deserialize(std::ifstream& input) {
		renderer::RenderSettings render_settings;
		RoutingSettings routing_settings;
		DeserializeCatalogue(input, transport_catalogue_, render_settings, routing_settings);
		map_renderer_.SetRenderSettings(render_settings);
		transport_router_.SetRoutingSettings(routing_settings);
		transport_router_.BuildGraph();
	}

} // namespace transport