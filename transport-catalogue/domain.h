#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace transport {

	struct Stop {
		std::string stop_name;
		geo::Coordinates coordinates;
	};

	struct PairStopHasher {
		size_t operator()(const  std::pair<const Stop*, const Stop*>& pair_ptr) const {
			std::hash<const void*> hasher;
			return hasher(pair_ptr.first) + hasher(pair_ptr.second);
		}
	};

	struct Bus {
		std::string bus_name;
		std::vector<Stop*> route;
		bool is_roundtrip;
	};

	std::vector<const Stop*> GetBusFullRoute(const Bus* bus);

}  // namespace transport