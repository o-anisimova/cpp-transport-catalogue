#include "transport_catalogue.h"

#include <stdexcept>
#include <unordered_set>

using namespace std;

namespace transport {

	void Catalogue::AddStop(Stop&& stop) {
		stops_.push_back(move(stop));
		stopname_to_stop_.insert({ stops_.back().stop_name, &stops_.back() });
	}

	Stop* Catalogue::FindStop(string_view stop_name) const{
		if (stopname_to_stop_.count(stop_name) == 0) {
			return nullptr;
		}
		return stopname_to_stop_.at(stop_name);
	}

	void Catalogue::AddBus(Bus&& bus) {
		buses_.push_back(move(bus));
		Bus* added_bus = &buses_.back();
		busname_to_bus_.insert({ added_bus->bus_name, added_bus });

		for (Stop* stop : added_bus->route) {
			stop_to_bus_names_[stop].insert(added_bus->bus_name);
		}
	}

	Bus* Catalogue::FindBus(std::string_view bus_name) const {
		if (busname_to_bus_.count(bus_name) == 0) {
			return nullptr;
		}
		return busname_to_bus_.at(bus_name);
	}

	detail::BusInfo Catalogue::GetBusInfo(Bus* bus) const {
		if (bus == nullptr) {
			throw invalid_argument("Bus not found"s);
		}

		detail::BusInfo bus_info;
		bus_info.stops_qty = bus->route.size();

		unordered_set<string> unique_stops;
		double geographic_route_length = 0.0;
		for (int i = 0; i < bus_info.stops_qty - 1; ++i) {
			unique_stops.insert(bus->route[i]->stop_name);
			geographic_route_length += ComputeDistance(bus->route[i]->coordinates, bus->route[i + 1]->coordinates);
			bus_info.route_length += GetDistanceBetweenStops(bus->route[i], bus->route[i+1]);
		}
		unique_stops.insert(bus->route[bus_info.stops_qty - 1]->stop_name);
	
		bus_info.unique_stops_qty = unique_stops.size();
		bus_info.curvature = bus_info.route_length / geographic_route_length;

		return bus_info;
	}

	const set<string_view>& Catalogue::GetStopToBusesList(Stop* stop) const {
		if (stop == nullptr) {
			throw invalid_argument("Stop not found"s);
		}
		else if (stop_to_bus_names_.count(stop) == 0) {
			static set<string_view> empty_set;
			return empty_set;
		}

		return stop_to_bus_names_.at(stop);
	}

	void Catalogue::AddStopsDistance(Stop* lhs_stop, Stop* rhs_stop, int distance) {
		stops_distances_[std::pair<Stop*, Stop*>(lhs_stop, rhs_stop)] = distance;
	}

	int Catalogue::GetDistanceBetweenStops(Stop* lhs_stop, Stop* rhs_stop) const {
		if (stops_distances_.count({ lhs_stop, rhs_stop }) > 0) {
			return stops_distances_.at({ lhs_stop, rhs_stop });
		}
		else if (stops_distances_.count({ rhs_stop , lhs_stop }) > 0) {
			return stops_distances_.at({ rhs_stop , lhs_stop });
		}
		throw invalid_argument("Distance not found");
	}

}

