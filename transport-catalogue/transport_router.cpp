#include "transport_router.h"

namespace transport {

	bool operator>(const EdgeParams& lhs, const EdgeParams& rhs) {
		return lhs.weight > rhs.weight;
	}

	bool operator<(const EdgeParams& lhs, const EdgeParams& rhs) {
		return lhs.weight < rhs.weight;
	}

	EdgeParams operator+(const EdgeParams& lhs, const EdgeParams& rhs) {
		EdgeParams edge_params;
		edge_params.weight = lhs.weight + rhs.weight;
		return edge_params;
	}

	TransportRouter::TransportRouter(const TransportCatalogue& transport_catalogue)
		:transport_catalogue_(transport_catalogue), settings_() {
	}

	void TransportRouter::SetRoutingSettings(RoutingSettings routing_settings) {
		settings_ = routing_settings;
	}

	void TransportRouter::AddWaitGraphEdges() {
		for (const Stop& stop : transport_catalogue_.GetStopList()) {
			graph::Edge<EdgeParams> edge;
			edge.from = stop.id * 2; // Вершина id * 2 - начало ожидания
			edge.to = stop.id * 2 + 1; // Вершина id * 2 + 1 - окончание ожидания
			edge.weight.weight = settings_.bus_wait_time;
			graph_ptr_->AddEdge(edge);
		}
	}

	void TransportRouter::AddBusTripEdges() {
		for (const Bus& bus : transport_catalogue_.GetBusList()) {
			const auto& bus_full_route = GetBusFullRoute(&bus);
			for (int i = 0; i < bus_full_route.size(); ++i) {
				double total_distance = 0;
				for (size_t j = i + 1; j < bus_full_route.size(); ++j) {
					double local_distance = transport_catalogue_.GetDistanceBetweenStops(bus_full_route[j - 1], bus_full_route[j]);
					total_distance += local_distance;
					graph::Edge<EdgeParams> edge;
					const Stop* from = bus_full_route[i];
					const Stop* to = bus_full_route[j];
					edge.from = from->id * 2 + 1;
					edge.to = to->id * 2;
					edge.weight.bus_name = bus.bus_name;
					edge.weight.span_count = j - i;
					edge.weight.weight = (total_distance / (1000 * settings_.bus_velocity)) * 60;
					graph_ptr_->AddEdge(edge);
				}
			}
		}
	}

	void TransportRouter::BuildGraph() {
		//Создаем граф и передаем в конструктор количество остановок * 2
		graph_ptr_ = std::make_unique<Graph>(Graph(transport_catalogue_.GetStopsCnt() * 2));
		//Добавляем ребра для учета времени ожидания
		AddWaitGraphEdges();
		//Теперь добавляем маршруты автобусов в граф
		AddBusTripEdges();
		router_ptr_ = std::make_unique<Router>(Router(*graph_ptr_));
	}

	std::optional<Route> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
		//Получили лучший маршрут
		std::optional<graph::Router<EdgeParams>::RouteInfo> route_info = router_ptr_->BuildRoute(transport_catalogue_.FindStop(from)->id * 2,
			transport_catalogue_.FindStop(to)->id * 2);

		//Приводим в читабельный вид
		std::optional<Route> result;
		if (route_info.has_value()) {
			Route route;
			route.total_weight = route_info->weight.weight;
			route.events.reserve(route.events.size());
			for (const auto edge_id : route_info->edges) {
				const auto& edge = graph_ptr_->GetEdge(edge_id);
				Event event;
				event.weight = edge.weight.weight;
				//Если не указан автобус, считаем, что это Wait
				if (edge.weight.bus_name.empty()) {
					event.event_type = EventType::WAIT;
					event.stop_name = transport_catalogue_.GetStopNameByPos(edge.from / 2);
				}
				//Если указан автобус, то это BusTrip
				else {
					event.event_type = EventType::BUS_TRIP;
					event.bus_name = edge.weight.bus_name;
					event.span_count = edge.weight.span_count;
				}
				route.events.push_back(std::move(event));
			}
			result = route;
		}
		return result;
	}
}