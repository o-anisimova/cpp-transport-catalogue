#include "json_reader.h"

#include <cassert>

using namespace std;
using namespace json;

namespace transport {

	void JsonReader::FillCatalogue(istream& in) {
		Document document = Load(in);
		const Dict& root_dict = document.GetRoot().AsMap();
		assert(root_dict.count("base_requests"s) > 0);

		const Array& base_requests = root_dict.at("base_requests"s).AsArray();

		//¬ первый проход заполн€ем только остановки
		for (const Node& request_node : base_requests) {
			const Dict& base_request = request_node.AsMap();

			assert(base_request.count("type"s) > 0);
			if (base_request.at("type"s) == "Stop"s) {
				ProcessStop(base_request);
			}
		}

		//¬о второй проход устанавливаем рассто€ни€ между остановками и маршруты
		for (const Node& request_node : base_requests) {
			const Dict& base_request = request_node.AsMap();
			//–ассто€ние между остановками может быть не указано
			if (base_request.at("type"s) == "Stop"s && base_request.count("road_distances"s) > 0) {
				ProcessDistances(base_request);
			}
			else if (base_request.at("type"s) == "Bus"s) {
				ProcessBus(base_request);
			}
		}

		//—охран€ем запросы к транспортному каталогу в вектор
		if (root_dict.count("stat_requests"s) > 0) {
			for (const auto& request_node : root_dict.at("stat_requests"s).AsArray()) {
				const Dict& stat_request_dict = request_node.AsMap();
				FillStatRequestQueue(stat_request_dict);
			}
		}
	}

	void JsonReader::ProcessStop(const Dict& stop_request) {
		assert(stop_request.count("name"s) > 0);
		assert(stop_request.count("latitude"s) > 0);
		assert(stop_request.count("longitude"s) > 0);

		Stop stop;
		stop.stop_name = stop_request.at("name"s).AsString();
		stop.coordinates.lat = stop_request.at("latitude"s).AsDouble();
		stop.coordinates.lng = stop_request.at("longitude"s).AsDouble();
		transport_catalogue_.AddStop(stop);
	}

	void JsonReader::ProcessDistances(const Dict& stop_request) {
		for (const auto& dist : stop_request.at("road_distances"s).AsMap()) {
			Stop* stop1 = transport_catalogue_.FindStop(stop_request.at("name"s).AsString());
			Stop* stop2 = transport_catalogue_.FindStop(dist.first);
			int distance = dist.second.AsInt();

			transport_catalogue_.SetStopsDistance(stop1, stop2, distance);
		}
	}

	void JsonReader::ProcessBus(const json::Dict& bus_request) {
		assert(bus_request.count("name"s) > 0);

		Bus bus;
		bus.bus_name = bus_request.at("name").AsString();
		if (bus_request.count("stops"s) > 0) {
			for (const Node& request_node : bus_request.at("stops"s).AsArray()) {
				Stop* stop = transport_catalogue_.FindStop(request_node.AsString());
				bus.route.push_back(move(stop));
			}
			if (bus_request.count("is_roundtrip"s) > 0) {
				if (!bus_request.at("is_roundtrip"s).AsBool()) {
					for (int i = bus.route.size() - 2; i >= 0; --i) {
						bus.route.push_back(bus.route[i]);
					}
				}
			}
		}

		transport_catalogue_.AddBus(bus);
	}

	void JsonReader::FillStatRequestQueue(const json::Dict& stat_request_dict) {
		assert(stat_request_dict.count("id"s));
		assert(stat_request_dict.count("type"s));
		assert(stat_request_dict.count("name"s));

		StatRequest stat_request;
		stat_request.id_ = stat_request_dict.at("id"s).AsInt();
		stat_request.name_ = stat_request_dict.at("name"s).AsString();
		if (stat_request_dict.at("type").AsString() == "Stop") {
			stat_request.type_ = StatRequestType::STOP;
		}
		else if (stat_request_dict.at("type").AsString() == "Bus") {
			stat_request.type_ = StatRequestType::BUS;
		}
		else {
			throw invalid_argument("Unknown request type"s);
		}

		stat_requests_.push_back(move(stat_request));
	}
	
	void JsonReader::OutputData(ostream& out) {
		json::Array responses;
		responses.reserve(stat_requests_.size());

		for (const auto& request : stat_requests_) {
			Node response;
			if (request.type_ == StatRequestType::STOP) {
				response = GetStopData(request);
			}
			else if (request.type_ == StatRequestType::BUS) {
				response = GetBusData(request);
			}
			responses.push_back(std::move(response));
		}

		Print(Document(Node(responses)), out);
	}

	Node JsonReader::GetStopData(const StatRequest& stat_request) {
		json::Dict response;
		response["request_id"s] = json::Node(stat_request.id_);
		Stop* stop = transport_catalogue_.FindStop(stat_request.name_);
		if (stop == nullptr) {
			response["error_message"s] = "not found"s;
		}
		else {
			json::Array buses;
			for (string_view bus_name : transport_catalogue_.GetStopToBusesList(stop)) {
				buses.push_back(json::Node(static_cast<std::string>(bus_name)));
			}
			response["buses"s] = std::move(buses);
		}

		return Node(response);
	}
	
	Node JsonReader::GetBusData(const StatRequest& request) {
		json::Dict response;
		response["request_id"s] = json::Node(request.id_);
		Bus* bus = transport_catalogue_.FindBus(request.name_);

		if (bus == nullptr) {
			response["error_message"s] = "not found"s;
		}
		else {
	        transport::detail::BusInfo bus_info = transport_catalogue_.GetBusInfo(bus);
			response["stop_count"s] = json::Node{ bus_info.stops_qty };
			response["unique_stop_count"s] = json::Node{ bus_info.unique_stops_qty };
			response["route_length"s] = json::Node{ bus_info.route_length };
			response["curvature"s] = json::Node{ bus_info.curvature };
		}

		return Node(response);
	}

}