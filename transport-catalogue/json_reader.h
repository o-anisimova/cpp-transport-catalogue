#pragma once
#include "transport_catalogue.h"
#include "json.h"

namespace transport {

	enum StatRequestType {
		STOP,
		BUS
	};

	struct StatRequest {
		int id_;
		StatRequestType type_;
		std::string name_;
	};

	class JsonReader {
	public:
		JsonReader(Catalogue& transport_catalogue)
		:transport_catalogue_(transport_catalogue){
		}

		void FillCatalogue(istream& in);

		void OutputData(ostream& out);

	private:
		void ProcessStop(const json::Dict& stop_request);

		void ProcessDistances(const json::Dict& stop_request);

		void ProcessBus(const json::Dict& bus_request);

		void FillStatRequestQueue(const json::Dict& stat_request_dict);

		json::Node GetStopData(const StatRequest& stat_request);

		json::Node GetBusData(const StatRequest& stat_request);
		
		Catalogue& transport_catalogue_;
		std::vector<StatRequest> stat_requests_;
	};

}