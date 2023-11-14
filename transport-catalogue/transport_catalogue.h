#pragma once

#include "domain.h"

#include <string_view>
#include <deque>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace transport {

	class TransportCatalogue {
	public:
		using StopsDistances = std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopHasher>;

		void AddStop(const Stop& stop);

		Stop* FindStop(std::string_view stop_name) const;
	    const Stop* FindStopByPos(size_t pos) const;

		void AddBus(const Bus& bus);

		Bus* FindBus(std::string_view bus_name) const;

		std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

		const std::set<std::string_view>& GetStopToBusesList(Stop* stop) const;

		void SetStopsDistance(const Stop* lhs_stop, const Stop* rhs_stop, int distance);
		
		int GetDistanceBetweenStops(const Stop* lhs_stop, const Stop* rhs_stop) const;

		std::vector<const Bus*> GetBusListSorted() const;
		
		std::vector<const Stop*> GetStopListSorted() const;

		const std::deque<Stop>& GetStopList() const;

		const std::deque<Bus>& GetBusList() const;

		size_t GetStopsCnt() const;

		StopsDistances GetStopsDistances() const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_bus_names_;
		StopsDistances stops_distances_;
	};

}  // namespace transport

