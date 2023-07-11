#include "input_reader.h"

#include <iostream>

using namespace std;

namespace transport {

	namespace input_reader {
		
		InputReader::InputReader(Catalogue& transport_catalogue)
			:transport_catalogue_(transport_catalogue) {
		}

		void InputReader::ReadData() {
			InputData();
			ProcessData();
		}

		void InputReader::InputData() {
			size_t request_cnt;
			cin >> request_cnt >> ws;

			stops_.reserve(request_cnt);
			buses_.reserve(request_cnt);

			for (size_t i = 0; i < request_cnt; ++i) {
				string request;
				getline(cin, request);
				if (request.substr(0,4) == "Stop"s) {
					stops_.push_back(request.substr(5, request.size())); // 4 символа в Stop + пробел
				}
				else if (request.substr(0, 3) == "Bus"s) {
					buses_.push_back(request.substr(4, request.size())); //3 символа Bus + пробел
				}
				else {
					throw invalid_argument("Unknown request type"s);
				}
			}
		}

		void InputReader::ProcessData() {
			//Сначала обрабатываем остановки, чтобы было что положить в маршруты
			ProcessStops();
			ProcessBuses();
			ProcessDistances();
		}

		void InputReader::ProcessStops() {
			for (const string& request : stops_) {
				Stop stop;

				//Отделяем наименование остановки
				size_t begin = 0; 
				size_t separator_pos = request.find(':', begin);
				string stop_name =
				stop.stop_name = SeparateWord(request, begin, separator_pos); 

				//Отделяем координаты
				begin = separator_pos + 1;
				separator_pos = request.find(',', begin);
				stop.coordinates.lat = stod(SeparateTrimmedWord(request, begin, separator_pos - 1));

				//Вторая координата
				begin = separator_pos + 1;
				separator_pos = request.find(',', begin);
				stop.coordinates.lng = stod(SeparateTrimmedWord(request, begin, separator_pos - 1));

				//Добавляем в транспортный каталог
				transport_catalogue_.AddStop(move(stop));
			}
		}

		void InputReader::ProcessBuses() {
			for (const string& request : buses_) {
				Bus bus;

				//Найдем наименование автобуса
				size_t begin = 0;
				size_t separator_pos = request.find(':', begin);
				bus.bus_name = SeparateWord(request, begin, separator_pos);

				//Теперь разбираем маршрут
				char separator;
				if (request.find('>') != string::npos) {
					separator = '>';
				}
				else {
					separator = '-';
				}

				while (separator_pos != string::npos) {
					begin = separator_pos + 1;
					separator_pos = request.find(separator, begin);
					string stop_name = SeparateTrimmedWord(request, begin, separator_pos - 1);
					Stop* stop = transport_catalogue_.FindStop(stop_name);
					bus.route.push_back(stop);
				}

				if (separator == '-') {
					for (int i = bus.route.size() - 2; i >= 0; --i) {
						bus.route.push_back(bus.route[i]);
					}
				}

				transport_catalogue_.AddBus(move(bus));
			}
		}

		void InputReader::ProcessDistances() {
			for (const string& request : stops_) {
				size_t separator_pos = request.find(':');
				Stop* stop1 = transport_catalogue_.FindStop(SeparateWord(request, 0, separator_pos));

				separator_pos = request.find(',', request.find(',') + 1);
				while (separator_pos != string::npos) {
					size_t begin = separator_pos + 1;
					separator_pos = request.find('m', begin);
					int distance = stoi(SeparateWord(request, begin, separator_pos));

					begin = request.find("to"s, separator_pos) + 3;
					separator_pos = request.find(',', begin);
					Stop* stop2 = transport_catalogue_.FindStop(SeparateWord(request, begin, separator_pos));

					transport_catalogue_.AddStopsDistance(stop1, stop2, distance);
				}
			}
		}

		string InputReader::SeparateWord(const string& str, size_t begin, size_t size) {
			return str.substr(begin, size - begin);
		}

		string InputReader::SeparateTrimmedWord(const string& str, size_t begin, size_t end) {
			return SeparateWord(str,
				str.find_first_not_of(' ', begin),
				str.find_last_not_of(' ', end) + 1);
		}
	
	}

}