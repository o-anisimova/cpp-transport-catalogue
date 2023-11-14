#pragma once
#include "request_handler.h"
#include "json_builder.h"

namespace transport {

	namespace json_reader {

		class BaseRequest {
		public:
			BaseRequest(int id);
			virtual ~BaseRequest() = default;
			virtual json::Dict GetRequestedData(const RequestHandler& request_handler) const = 0;
			int GetId() const;
		private:
			int id_;
		};

		class StopRequest : public BaseRequest {
		public:
			StopRequest(int id, const std::string& name);
			json::Dict GetRequestedData(const RequestHandler& request_handler) const override;
		private:
			std::string name_;
		};

		class BusRequest : public BaseRequest {
		public:
			BusRequest(int id, const std::string& name);
			json::Dict GetRequestedData(const RequestHandler& request_handler) const override;
		private:
			std::string name_;
		};

		class MapRequest: public BaseRequest{
		public:
			MapRequest(int id);
			json::Dict GetRequestedData(const RequestHandler& request_handler) const override;
		private:
		};

		 class RouteRequest : public BaseRequest {
		public:
			RouteRequest(int id, const std::string& from, const std::string& to);
			json::Dict GetRequestedData(const RequestHandler& request_handler) const override;
		private:
			std::string from_;
			std::string to_;
		};

		class JsonReader {
		public:
			JsonReader(TransportCatalogue& transport_catalogue)
				:request_handler_(transport_catalogue) {
			}

			void MakeBase(std::istream& in);
			void ProcessRequest(std::istream& in, std::ostream& out);

		private:
			void FillStops(const json::Array& base_requests);
			void FillDistances(const json::Dict& stop_request);
			void FillBus(const json::Dict& bus_request);
			void FillStatRequestQueue(const json::Dict& stat_request_dict);
			renderer::RenderSettings FillRenderSettings(const json::Dict& bus_request);
			RoutingSettings FillRoutingSettings(const json::Dict& routing_settings_dict);
			svg::Color GetColor(const json::Node& color_node);
			void OutputData(std::ostream& out);
		
			RequestHandler request_handler_;
			std::vector<std::unique_ptr<BaseRequest>> stat_requests_;
		};

	} // namespace json_reader

} // namespace transport