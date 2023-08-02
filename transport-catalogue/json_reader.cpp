#include "json_reader.h"

#include <cassert>
#include <sstream>

using namespace std;
using namespace json;

namespace transport {

	namespace json_reader {

		void JsonReader::FillCatalogue(istream& in) {
			Document document = Load(in);
			const Dict& root_dict = document.GetRoot().AsMap();
			assert(root_dict.count("base_requests"s) > 0);

			const Array& base_requests = root_dict.at("base_requests"s).AsArray();

			//В первый проход заполняем только остановки
			FillStops(base_requests);

			//Во второй проход устанавливаем расстояния между остановками и маршруты
			for (const Node& request_node : base_requests) {
				const Dict& base_request = request_node.AsMap();
				//Расстояние между остановками может быть не указано
				if (base_request.at("type"s) == "Stop"s && base_request.count("road_distances"s) > 0) {
					FillDistances(base_request);
				}
				else if (base_request.at("type"s) == "Bus"s) {
					FillBus(base_request);
				}
			}

			if (root_dict.count("render_settings"s) > 0) {
				FillRenderSettings(root_dict.at("render_settings"s).AsMap());
			}

			//Сохраняем запросы к транспортному каталогу в вектор
			if (root_dict.count("stat_requests"s) > 0) {
				for (const auto& request_node : root_dict.at("stat_requests"s).AsArray()) {
					const Dict& stat_request_dict = request_node.AsMap();
					FillStatRequestQueue(stat_request_dict);
				}
			}
		}

		void JsonReader::FillStops(const Array& base_requests) {
			for (const Node& request_node : base_requests) {
				const Dict& stop_request = request_node.AsMap();
				assert(stop_request.count("type"s) > 0);
				if (stop_request.at("type"s) == "Stop"s) {
					assert(stop_request.count("name"s) > 0);
					assert(stop_request.count("latitude"s) > 0);
					assert(stop_request.count("longitude"s) > 0);

					Stop stop;
					stop.stop_name = stop_request.at("name"s).AsString();
					stop.coordinates.lat = stop_request.at("latitude"s).AsDouble();
					stop.coordinates.lng = stop_request.at("longitude"s).AsDouble();
					transport_catalogue_.AddStop(stop);
				}
			}
		}

		void JsonReader::FillDistances(const Dict& stop_request) {
			for (const auto& dist : stop_request.at("road_distances"s).AsMap()) {
				Stop* stop1 = transport_catalogue_.FindStop(stop_request.at("name"s).AsString());
				Stop* stop2 = transport_catalogue_.FindStop(dist.first);
				int distance = dist.second.AsInt();

				transport_catalogue_.SetStopsDistance(stop1, stop2, distance);
			}
		}

		void JsonReader::FillBus(const json::Dict& bus_request) {
			assert(bus_request.count("name"s) > 0);
			assert(bus_request.count("is_roundtrip"s) > 0);

			Bus bus;
			bus.bus_name = bus_request.at("name").AsString();
			bus.is_roundtrip = bus_request.at("is_roundtrip"s).AsBool();
			if (bus_request.count("stops"s) > 0) {
				for (const Node& request_node : bus_request.at("stops"s).AsArray()) {
					Stop* stop = transport_catalogue_.FindStop(request_node.AsString());
					bus.route.push_back(move(stop));
				}
			}

			transport_catalogue_.AddBus(bus);
		}

		void JsonReader::FillStatRequestQueue(const json::Dict& stat_request_dict) {
			assert(stat_request_dict.count("id"s));
			assert(stat_request_dict.count("type"s));

			StatRequest stat_request;
			stat_request.id_ = stat_request_dict.at("id"s).AsInt();
			if (stat_request_dict.at("type").AsString() == "Stop") {
				stat_request.type_ = StatRequestType::STOP;
			
				assert(stat_request_dict.count("name"s));
				stat_request.name_ = stat_request_dict.at("name"s).AsString();
			}
			else if (stat_request_dict.at("type").AsString() == "Bus") {
				stat_request.type_ = StatRequestType::BUS;

				assert(stat_request_dict.count("name"s));
				stat_request.name_ = stat_request_dict.at("name"s).AsString();
			}
			else if (stat_request_dict.at("type").AsString() == "Map") {
				stat_request.type_ = StatRequestType::MAP;
			}
			else {
				throw invalid_argument("Unknown request type"s);
			}

			stat_requests_.push_back(move(stat_request));
		}
	
		void JsonReader::FillRenderSettings(const Dict& render_settings_dict) {
			renderer::RenderSettings render_settings;

			assert(render_settings_dict.count("width"s) > 0);
			render_settings.width = render_settings_dict.at("width"s).AsDouble();

			assert(render_settings_dict.count("height"s) > 0);
			render_settings.height = render_settings_dict.at("height"s).AsDouble();

			assert(render_settings_dict.count("padding"s) > 0);
			render_settings.padding = render_settings_dict.at("padding"s).AsDouble();

			assert(render_settings_dict.count("line_width"s) > 0);
			render_settings.line_width = render_settings_dict.at("line_width"s).AsDouble();

			assert(render_settings_dict.count("stop_radius"s) > 0);
			render_settings.stop_radius = render_settings_dict.at("stop_radius"s).AsDouble();

			assert(render_settings_dict.count("bus_label_font_size"s) > 0);
			render_settings.bus_label_font_size = render_settings_dict.at("bus_label_font_size"s).AsInt();

			assert(render_settings_dict.count("bus_label_offset"s) > 0);
			const Array& bus_label_offset = render_settings_dict.at("bus_label_offset"s).AsArray();
			render_settings.bus_label_offset.x = bus_label_offset[0].AsDouble();
			render_settings.bus_label_offset.y = bus_label_offset[1].AsDouble();

			assert(render_settings_dict.count("stop_label_font_size"s) > 0);
			render_settings.stop_label_font_size = render_settings_dict.at("stop_label_font_size"s).AsInt();

			assert(render_settings_dict.count("stop_label_offset"s) > 0);
			const Array& stop_label_offset = render_settings_dict.at("stop_label_offset"s).AsArray();
			render_settings.stop_label_offset.x = stop_label_offset[0].AsDouble();
			render_settings.stop_label_offset.y = stop_label_offset[1].AsDouble();

			assert(render_settings_dict.count("underlayer_color"s) > 0);
			render_settings.underlayer_color = GetColor(render_settings_dict.at("underlayer_color"s));

			assert(render_settings_dict.count("underlayer_width"s) > 0);
			render_settings.underlayer_width = render_settings_dict.at("underlayer_width"s).AsDouble();

			assert(render_settings_dict.count("color_palette"s) > 0);
			const Array& color_palette_array = render_settings_dict.at("color_palette"s).AsArray();
			for (const auto& color_node : color_palette_array) {
				render_settings.color_palette.push_back(GetColor(color_node));
			}

			map_renderer_.SetRenderSettings(render_settings);
		}
	
		svg::Color JsonReader::GetColor(const Node& color_node) {
			svg::Color color;

			if (color_node.IsString()) {
				color = color_node.AsString();
			}
			else if (color_node.IsArray()) {
				const Array& color_array = color_node.AsArray();
				switch (color_array.size()) {
				case 3:
				{
					svg::Rgb rgb;
					rgb.red = color_array[0].AsInt();
					rgb.green = color_array[1].AsInt();
					rgb.blue = color_array[2].AsInt();
					color = rgb;
				}
					break;
				case 4:
				{
					svg::Rgba rgba;
					rgba.red = color_array[0].AsInt();
					rgba.green = color_array[1].AsInt();
					rgba.blue = color_array[2].AsInt();
					rgba.opacity = color_array[3].AsDouble();
					color = rgba;
				}
					break;
				default:
					throw invalid_argument("Invalid number of color array elements"s);
				}
			}
			else {
				throw invalid_argument("Invalid color data type"s);
			}

			return color;
		}	

		void JsonReader::OutputData(ostream& out) {
			json::Array responses;
			responses.reserve(stat_requests_.size());

			for (const auto& request : stat_requests_) {
				Node response;
				switch (request.type_) {
				case StatRequestType::STOP:
					response = OutputStop(request);
					break;
				case StatRequestType::BUS:
					response = OutputBus(request);
					break;
				case StatRequestType::MAP:
					response = OutputMap(request);
					break;
				}
				responses.push_back(std::move(response));
			}

			Print(Document(Node(responses)), out);
		}

		Node JsonReader::OutputStop(const StatRequest& stat_request) {
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
	
		Node JsonReader::OutputBus(const StatRequest& request) {
			json::Dict response;
			response["request_id"s] = json::Node(request.id_);
			optional<transport::BusStat> bus_stat = request_handler_.GetBusStat(request.name_);

			if (!bus_stat.has_value()) {
				response["error_message"s] = "not found"s;
			}
			else {
				response["stop_count"s] = json::Node{ bus_stat->stops_qty };
				response["unique_stop_count"s] = json::Node{ bus_stat->unique_stops_qty };
				response["route_length"s] = json::Node{ bus_stat->route_length };
				response["curvature"s] = json::Node{ bus_stat->curvature };
			}

			return Node(response);
		}

		json::Node JsonReader::OutputMap(const StatRequest& stat_request) {
			json::Dict response;
			response["request_id"s] = json::Node(stat_request.id_);
			std::ostringstream out;
			request_handler_.RenderMap().Render(out);
			response["map"s] = json::Node(out.str()); 
			return Node(response);
		}

	} // namespace json_resder

} // namespace transport