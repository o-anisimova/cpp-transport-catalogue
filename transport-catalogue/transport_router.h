#pragma once
#include "transport_catalogue.h"
#include "router.h"

#include <memory>

namespace transport {

	struct RoutingSettings {
		double bus_wait_time;
		double bus_velocity;
	};

	struct EdgeParams {
		double weight = 0;
		size_t span_count = 0;
		std::string_view bus_name; 
	};

	bool operator>(const EdgeParams& lhs, const EdgeParams& rhs);
	bool operator<(const EdgeParams& lhs, const EdgeParams& rhs);
	EdgeParams operator+(const EdgeParams& lhs, const EdgeParams& rhs);

	enum EventType {
		WAIT,
		BUS_TRIP
	};

	struct Event {
		EventType event_type;
		double weight = 0;
		size_t span_count = 0;
		std::string_view stop_name;
		std::string_view bus_name;
	};

	struct Route {
		double total_weight = 0;
		std::vector<Event> events;
	};

	class TransportRouter {
	public:
		using Graph = graph::DirectedWeightedGraph<EdgeParams>;
		using Router = graph::Router<EdgeParams>;

		TransportRouter(const TransportCatalogue& transport_catalogue);

		void SetRoutingSettings(RoutingSettings routing_settings);

		void BuildGraph();

		std::optional<Route> BuildRoute(std::string_view from, std::string_view to) const;

	private:
		const TransportCatalogue& transport_catalogue_;
		RoutingSettings settings_;
		std::unique_ptr<Graph> graph_ptr_;
		std::unique_ptr<Router> router_ptr_;

		void AddWaitGraphEdges();

		void AddBusTripEdges();
	};

} //namespace transport
