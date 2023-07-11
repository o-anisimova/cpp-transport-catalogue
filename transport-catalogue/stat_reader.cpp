#include "stat_reader.h"

#include <iostream>
#include <iomanip>

using namespace std;

namespace transport {

	namespace stat_reader {
		
		void OutputData(const Catalogue& transport_catalogue) {
			vector<string> request_queue = InputRequests();

			for (string_view request : request_queue) {
				cout << request << ": "s;

				//Отделяем основную часть от слов "Stop", "Bus"
				if (request.substr(0, 4) == "Stop"s) {
					request = request.substr(5, request.size()); // 4 символа в Stop + пробел
					ReadStop(transport_catalogue, request);
				}
				else if (request.substr(0, 3) == "Bus"s) {
					request = request.substr(4, request.size()); //3 символа Bus + пробел
					ReadBus(transport_catalogue, request);
				}
				else {
					throw invalid_argument("Unknown request type"s);
				}
			}
		}

		vector<string> InputRequests() {
			size_t request_qty;
			cin >> request_qty >> ws;

			vector<string> request_queue;
			request_queue.reserve(request_qty);

			for (size_t i = 0; i < request_qty; ++i) {
				string request;
				getline(cin, request);
				request_queue.push_back(move(request));
			}

			return request_queue;
		}

		void ReadStop(const Catalogue& transport_catalogue, string_view request) {
			Stop* stop = transport_catalogue.FindStop(request);

			if (stop == nullptr) {
				cout << "not found" << endl;
				return;
			}
	
			set<string_view> buses = transport_catalogue.GetStopToBusesList(stop);
			if (buses.size() == 0) {
				cout << "no buses"s << endl;
				return;
			}
	
			cout << "buses"s;
			for (string_view bus_name : buses) {
				cout << " "s << bus_name;
			}
			cout << endl;
		}

		void ReadBus(const Catalogue& transport_catalogue, string_view request) {
			Bus* bus = transport_catalogue.FindBus(request);

			if (bus == nullptr) {
				cout << "not found"s << endl;
				return;
			}
	
			detail::BusInfo bus_info = transport_catalogue.GetBusInfo(bus);
			cout << bus_info.stops_qty << " stops on route, " << bus_info.unique_stops_qty << " unique stops, "s << bus_info.route_length << " route length, "s << setprecision(6) << bus_info.curvature << " curvature"s << endl;
		}


		string SeparateWord(const string& str, size_t begin, size_t size) {
			return str.substr(begin, size - begin);
		}

		string SeparateTrimmedWord(const string& str, size_t begin, size_t end) {
			return SeparateWord(str,
				str.find_first_not_of(' ', begin),
				str.find_last_not_of(' ', end) + 1);
		}

	}

}

