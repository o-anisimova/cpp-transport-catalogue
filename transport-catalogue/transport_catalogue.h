#pragma once

#include "domain.h"

#include <string_view>
#include <deque>
#include <unordered_map>
#include <set>

using namespace std;

namespace transport {

	class Catalogue {
	public:
		void AddStop(const Stop& stop);

		Stop* FindStop(std::string_view stop_name) const;

		void AddBus(const Bus& bus);

		Bus* FindBus(std::string_view bus_name) const;

		detail::BusInfo GetBusInfo(Bus* bus) const;

		const std::set<string_view>& GetStopToBusesList(Stop* stop) const;

		void SetStopsDistance(Stop* lhs_stop, Stop* rhs_stop, int distance);

		int GetDistanceBetweenStops(Stop* lhs_stop, Stop* rhs_stop) const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<Stop*, std::set<string_view>> stop_to_bus_names_;
		std::unordered_map<std::pair<Stop*, Stop*>, int, PairStopHasher> stops_distances_;
	};

}

