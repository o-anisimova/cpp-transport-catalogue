#pragma once

#include "transport_catalogue.h"

#include <iostream>

namespace transport {

	namespace input_reader {
		
		class InputReader {
		public:
			InputReader(Catalogue& transport_catalogue);

			void ReadData(istream& is);

		private:
			void InputData(istream& is);
	
			void ProcessData();

			void ProcessStops();

			void ProcessBuses();

			void ProcessDistances();

			std::string SeparateWord(const std::string& str, size_t begin, size_t size);

			std::string SeparateTrimmedWord(const std::string& str, size_t begin = 0, size_t end = 0);
	
			Catalogue& transport_catalogue_;
			std::vector<std::string> stops_;
			std::vector<std::string> buses_;
		};
	
	}
	
}



