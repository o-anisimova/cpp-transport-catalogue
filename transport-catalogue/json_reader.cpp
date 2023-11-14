#include "json_reader.h"
#include "serialization.h"

#include <cassert>
#include <sstream>
#include <fstream>

using namespace std;
using namespace json;

namespace transport {

	namespace json_reader {

		BaseRequest::BaseRequest(int id)
			:id_(id) {
		}

		int BaseRequest::GetId() const {
			return id_;
		}

		StopRequest::StopRequest(int id, const std::string& name)
			:BaseRequest(id), name_(name) {
		}

		json::Dict StopRequest::GetRequestedData(const RequestHandler& request_handler) const {
			Builder builder;
			builder.StartDict().Key("request_id"s).Value(GetId());
			Stop* stop = request_handler.FindStop(name_);
			if (stop == nullptr) {
				builder.Key("error_message"s).Value("not found"s);
			}
			else {
				builder.Key("buses"s).StartArray();
				for (string_view bus_name : request_handler.GetStopToBusesList(stop)) {
					builder.Value(static_cast<std::string>(bus_name));
				}
				builder.EndArray();
			}
			builder.EndDict();
			return builder.Build().AsDict();
		}

		BusRequest::BusRequest(int id, const string& name)
			:BaseRequest(id), name_(name) {
		}

		json::Dict BusRequest::GetRequestedData(const RequestHandler& request_handler) const {
			Builder builder;
			builder.StartDict().Key("request_id"s).Value(GetId());
			optional<transport::BusStat> bus_stat = request_handler.GetBusStat(name_);
			if (!bus_stat.has_value()) {
				builder.Key("error_message"s).Value("not found"s);
			}
			else {
				builder.Key("stop_count"s).Value(bus_stat->stops_qty);
				builder.Key("unique_stop_count"s).Value(bus_stat->unique_stops_qty);
				builder.Key("route_length"s).Value(bus_stat->route_length);
				builder.Key("curvature"s).Value(bus_stat->curvature);
			}
			builder.EndDict();
			return builder.Build().AsDict();
		}

		MapRequest::MapRequest(int id)
			:BaseRequest(id) {
		}

		json::Dict MapRequest::GetRequestedData(const RequestHandler& request_handler) const {
			ostringstream out;
			request_handler.RenderMap().Render(out);
			Builder builder;
			builder.StartDict()
				.Key("request_id"s).Value(GetId())
				.Key("map"s).Value(out.str())
				.EndDict();
			return builder.Build().AsDict();
		}

		RouteRequest::RouteRequest(int id, const std::string& from, const std::string& to)
			:BaseRequest(id), from_(from), to_(to) {
		}

		json::Dict RouteRequest::GetRequestedData(const RequestHandler& request_handler) const {
			std::optional<Route> route = request_handler.BuildRoute(from_, to_);
			Builder builder;
			if (route.has_value()) {
				builder.StartDict()
					.Key("total_time"s).Value(route->total_weight)
					.Key("request_id"s).Value(GetId())
					   .Key("items"s).StartArray();
				for (const auto& event : route->events) {
					builder.StartDict()
						.Key("time"s).Value(event.weight);
					switch (event.event_type) {
					case(EventType::WAIT):
						builder.Key("stop_name"s).Value(static_cast<string>(event.stop_name))
								.Key("type"s).Value("Wait"s);
					    break; 
					case(EventType::BUS_TRIP): 
						builder.Key("bus"s).Value(static_cast<string>(event.bus_name))
							.Key("span_count"s).Value(static_cast<int>(event.span_count))
							.Key("type"s).Value("Bus"s);

					}
					builder.EndDict();
				}
				builder.EndArray()
					   .EndDict();
			}
			else {
				builder.StartDict()
					.Key("request_id"s).Value(GetId())
					.Key("error_message"s).Value("not found"s)
					.EndDict();
				
			}
			return builder.Build().AsDict();	
		}

		void JsonReader::MakeBase(istream& in) {
			Document document = Load(in);
			const Dict& root_dict = document.GetRoot().AsDict();

			std::string filename = root_dict.at("serialization_settings"s).AsDict().at("file"s).AsString();
			ofstream out_file(filename, ios::binary);


			assert(root_dict.count("base_requests"s) > 0);

			const Array& base_requests = root_dict.at("base_requests"s).AsArray();

			//В первый проход заполняем только остановки
			FillStops(base_requests);

			//Во второй проход устанавливаем расстояния между остановками и маршруты
			for (const Node& request_node : base_requests) {
				const Dict& base_request = request_node.AsDict();
				//Расстояние между остановками может быть не указано
				if (base_request.at("type"s) == "Stop"s && base_request.count("road_distances"s) > 0) {
					FillDistances(base_request);
				}
				else if (base_request.at("type"s) == "Bus"s) {
					FillBus(base_request);
				}
			}

			renderer::RenderSettings settings;
			if (root_dict.count("render_settings"s) > 0) {
				settings = FillRenderSettings(root_dict.at("render_settings"s).AsDict());

			}

			RoutingSettings routing_settings;
			if (root_dict.count("routing_settings"s) > 0) {
				routing_settings = FillRoutingSettings(root_dict.at("routing_settings"s).AsDict());
			}

			request_handler_.Serialize(out_file, settings, routing_settings);
		}

		void JsonReader::FillStops(const Array& base_requests) {
			for (const Node& request_node : base_requests) {
				const Dict& stop_request = request_node.AsDict();
				assert(stop_request.count("type"s) > 0);
				if (stop_request.at("type"s) == "Stop"s) {
					assert(stop_request.count("name"s) > 0);
					assert(stop_request.count("latitude"s) > 0);
					assert(stop_request.count("longitude"s) > 0);

					Stop stop;
					stop.stop_name = stop_request.at("name"s).AsString();
					stop.coordinates.lat = stop_request.at("latitude"s).AsDouble();
					stop.coordinates.lng = stop_request.at("longitude"s).AsDouble();
					request_handler_.AddStop(stop);
				}
			}
		}

		void JsonReader::FillDistances(const Dict& stop_request) {
			for (const auto& dist : stop_request.at("road_distances"s).AsDict()) {
				Stop* stop1 = request_handler_.FindStop(stop_request.at("name"s).AsString());
				Stop* stop2 = request_handler_.FindStop(dist.first);
				int distance = dist.second.AsInt();
				request_handler_.SetStopsDistance(stop1, stop2, distance);
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
					Stop* stop = request_handler_.FindStop(request_node.AsString());
					bus.route.push_back(move(stop));
				}
			}

			request_handler_.AddBus(bus);
		}

		void JsonReader::FillStatRequestQueue(const json::Dict& stat_request_dict) {
			assert(stat_request_dict.count("id"s));
			assert(stat_request_dict.count("type"s));

			if (stat_request_dict.at("type").AsString() == "Stop") {
				assert(stat_request_dict.count("name"s));
				unique_ptr<BaseRequest> request_ptr = make_unique<StopRequest>(StopRequest (stat_request_dict.at("id"s).AsInt(),
																								  stat_request_dict.at("name"s).AsString()));
				stat_requests_.push_back(move(request_ptr));
			}
			else if (stat_request_dict.at("type").AsString() == "Bus") {
				assert(stat_request_dict.count("name"s));
				unique_ptr<BaseRequest> request_ptr = make_unique<BusRequest>(BusRequest(stat_request_dict.at("id"s).AsInt(),
																							   stat_request_dict.at("name"s).AsString()));
				stat_requests_.push_back(move(request_ptr));
			}
			else if (stat_request_dict.at("type").AsString() == "Map") {
				unique_ptr<BaseRequest> request_ptr = make_unique<MapRequest>(MapRequest(stat_request_dict.at("id"s).AsInt()));
				stat_requests_.push_back(move(request_ptr));
			}
			else if (stat_request_dict.at("type").AsString() == "Route") {
				assert(stat_request_dict.count("from"s));
				assert(stat_request_dict.count("to"s));
				unique_ptr<BaseRequest> request_ptr = make_unique<RouteRequest>(RouteRequest(stat_request_dict.at("id"s).AsInt(),
																								   stat_request_dict.at("from"s).AsString(),
																								   stat_request_dict.at("to"s).AsString()));
				stat_requests_.push_back(move(request_ptr));
			}
			else {
				throw invalid_argument("Unknown request type"s);
			}
		}
	
		renderer::RenderSettings JsonReader::FillRenderSettings(const Dict& render_settings_dict) {
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

			return render_settings;
		}

		RoutingSettings JsonReader::FillRoutingSettings(const json::Dict& routing_settings_dict) {
			RoutingSettings routing_settings;

			assert(routing_settings_dict.count("bus_wait_time"s) > 0);
			routing_settings.bus_wait_time = routing_settings_dict.at("bus_wait_time"s).AsDouble();

			assert(routing_settings_dict.count("bus_velocity"s) > 0);
			routing_settings.bus_velocity = routing_settings_dict.at("bus_velocity"s).AsDouble();

			return routing_settings;
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
			Builder builder;
			builder.StartArray();
			for (const auto& request : stat_requests_) {
				builder.Value(request->GetRequestedData(request_handler_));
			}
			builder.EndArray();
			Print(Document(builder.Build()), out);
		}

		void JsonReader::ProcessRequest(istream& in, ostream& out) {
			Document document = Load(in);
			const Dict& root_dict = document.GetRoot().AsDict();

			std::string filename = root_dict.at("serialization_settings"s).AsDict().at("file"s).AsString();
			ifstream in_file(filename, ios::binary);

			//Сохраняем запросы к транспортному каталогу в вектор
			if (root_dict.count("stat_requests"s) > 0) {
				for (const auto& request_node : root_dict.at("stat_requests"s).AsArray()) {
					const Dict& stat_request_dict = request_node.AsDict();
					FillStatRequestQueue(stat_request_dict);
				}
			}

			//Десериализовали справочник
			request_handler_.Deserialize(in_file);

			OutputData(out);
		}

	} // namespace json_resder

} // namespace transport