#pragma once
#include "request_handler.h"
#include "json_builder.h"

namespace transport {

	namespace json_reader {

		enum StatRequestType {
			STOP,
			BUS,
			MAP
		};

		struct StatRequest {
			int id_;
			StatRequestType type_;
			std::string name_;
		};

		class JsonReader {
		public:
			JsonReader(TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer)
			:request_handler_(transport_catalogue, map_renderer), transport_catalogue_(transport_catalogue), map_renderer_(map_renderer){
			}

			void FillCatalogue(std::istream& in);

			void OutputData(std::ostream& out);

		private:
			void FillStops(const json::Array& base_requests);

			void FillDistances(const json::Dict& stop_request);

			void FillBus(const json::Dict& bus_request);

			void FillStatRequestQueue(const json::Dict& stat_request_dict);

			void FillRenderSettings(const json::Dict& bus_request);

			svg::Color GetColor(const json::Node& color_node);

			json::Dict OutputStop(const StatRequest& stat_request);

			json::Dict OutputBus(const StatRequest& stat_request);

			json::Dict OutputMap(const StatRequest& stat_request);
		
			RequestHandler request_handler_;
			TransportCatalogue& transport_catalogue_;
			renderer::MapRenderer& map_renderer_;
			std::vector<StatRequest> stat_requests_;
		};

	} // namespace json_reader

} // namespace transport