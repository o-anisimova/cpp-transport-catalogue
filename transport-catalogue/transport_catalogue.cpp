#include "transport_catalogue.h"

#include <stdexcept>
#include <algorithm>

using namespace std;

namespace transport {

	void TransportCatalogue::AddStop(const Stop& stop) {
		stops_.push_back(stop);
		stopname_to_stop_.insert({ stops_.back().stop_name, &stops_.back() });
	}

	Stop* TransportCatalogue::FindStop(string_view stop_name) const{
		if (stopname_to_stop_.count(stop_name) == 0) {
			return nullptr;
		}
		return stopname_to_stop_.at(stop_name);
	}

	void TransportCatalogue::AddBus(const Bus& bus) {
		buses_.push_back(bus);
		Bus* added_bus = &buses_.back();
		busname_to_bus_.insert({ added_bus->bus_name, added_bus });

		for (Stop* stop : added_bus->route) {
			stop_to_bus_names_[stop].insert(added_bus->bus_name);
		}
	}

	Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
		if (busname_to_bus_.count(bus_name) == 0) {
			return nullptr;
		}
		return busname_to_bus_.at(bus_name);
	}

	const set<string_view>& TransportCatalogue::GetStopToBusesList(Stop* stop) const {
		if (stop == nullptr) {
			throw invalid_argument("Stop not found"s);
		}
		else if (stop_to_bus_names_.count(stop) == 0) {
			static set<string_view> empty_set;
			return empty_set;
		}

		return stop_to_bus_names_.at(stop);
	}

	void TransportCatalogue::SetStopsDistance(Stop* lhs_stop, Stop* rhs_stop, int distance) {
		stops_distances_[std::pair<Stop*, Stop*>(lhs_stop, rhs_stop)] = distance;
	}

	int TransportCatalogue::GetDistanceBetweenStops(const Stop* lhs_stop, const Stop* rhs_stop) const {
		if (stops_distances_.count({ lhs_stop, rhs_stop }) > 0) {
			return stops_distances_.at({ lhs_stop, rhs_stop });
		}
		else if (stops_distances_.count({ rhs_stop , lhs_stop }) > 0) {
			return stops_distances_.at({ rhs_stop , lhs_stop });
		}
		throw invalid_argument("Distance not found");
	}

	vector<const Bus*> TransportCatalogue::GetBusListSorted() const{
		vector<const Bus*> bus_list;
		for (size_t i = 0; i < buses_.size(); ++i) {
			bus_list.push_back(&buses_[i]);
		}
		sort(bus_list.begin(), bus_list.end(), [](const Bus* lhs, const Bus* rhs) {
			return std::lexicographical_compare(lhs->bus_name.begin(), lhs->bus_name.end(), rhs->bus_name.begin(), rhs->bus_name.end());
			});
		return bus_list;
	}

	vector<const Stop*> TransportCatalogue::GetStopListSorted() const {
		vector<const Stop*> stop_list;
		for (const auto& stop : stop_to_bus_names_) {
			if (!stop.second.empty()) {
				stop_list.push_back(stop.first);
			}
		}
		sort(stop_list.begin(), stop_list.end(), [](const Stop* lhs, const Stop* rhs) {
			return std::lexicographical_compare(lhs->stop_name.begin(), lhs->stop_name.end(), rhs->stop_name.begin(), rhs->stop_name.end());
			});
		return stop_list;
	}

	std::optional<BusStat> TransportCatalogue::GetBusStat(const std::string_view& bus_name) const {
		Bus* bus = FindBus(bus_name);
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
			bus_stat.route_length += GetDistanceBetweenStops(bus_full_route[i], bus_full_route[i + 1]);
		}
		unique_stops.insert(bus_full_route[bus_stat.stops_qty - 1]->stop_name);

		bus_stat.unique_stops_qty = static_cast<int>(unique_stops.size());
		bus_stat.curvature = bus_stat.route_length / geographic_route_length;

		return bus_stat;
	}

} // namespace transport



