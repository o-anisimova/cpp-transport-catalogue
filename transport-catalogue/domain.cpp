#include "domain.h"

namespace transport {
	
	std::vector<const Stop*> GetBusFullRoute(const Bus* bus) {
		std::vector<const Stop*> bus_full_route;
		bus_full_route.reserve(bus->route.size() * 2 - 1);

		for (const Stop* stop : bus->route) {
			bus_full_route.push_back(stop);
		}
		if (!bus->is_roundtrip) {
			for (size_t i = bus->route.size() - 1; i > 0; --i) {
				bus_full_route.push_back(bus->route[i - 1]);
			}
		}
		return bus_full_route;
	}

} // namespace transport