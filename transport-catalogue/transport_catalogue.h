#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>

using namespace std;

namespace transport {

	struct Stop {
		std::string stop_name;
		Coordinates coordinates;
	};

	struct PairStopHasher {
		size_t operator()(const  pair<Stop*, Stop*>& pair_ptr) const {
			std::hash<const void*> hasher;
			return hasher(pair_ptr.first) + hasher(pair_ptr.second);
		}
	};

	struct Bus {
		std::string bus_name;
		std::vector<Stop*> route;
	};

	namespace detail {
		//Вспомогательная структура для сбора инфы по маршруту
		struct BusInfo {
			size_t stops_qty = 0;
			size_t unique_stops_qty = 0;
			int route_length = 0;
			double curvature = 0.0;
		};
	}

	

	class Catalogue {
	public:
		void AddStop(Stop&& stop);

		Stop* FindStop(std::string_view stop_name) const;

		void AddBus(Bus&& bus);

		Bus* FindBus(std::string_view bus_name) const;

		detail::BusInfo GetBusInfo(Bus* bus) const;

		const std::set<string_view>& GetStopToBusesList(Stop* stop) const;

		void AddStopsDistance(Stop* lhs_stop, Stop* rhs_stop, int distance);

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

