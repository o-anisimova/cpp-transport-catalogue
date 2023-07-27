#include "geo.h"

#include <string>
#include <vector>

namespace transport {
	struct Stop {
		std::string stop_name;
		geo::Coordinates coordinates;
	};

	struct PairStopHasher {
		size_t operator()(const  std::pair<Stop*, Stop*>& pair_ptr) const {
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
			int stops_qty = 0;
			int unique_stops_qty = 0;
			int route_length = 0;
			double curvature = 0.0;
		};
	}
}