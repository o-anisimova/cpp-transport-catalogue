#pragma once

#include "transport_catalogue.h"

#include <iostream>

namespace transport {

	namespace stat_reader {

		void OutputData(istream& is, ostream& os, const Catalogue& transport_catalogue);

		std::vector<std::string> InputRequests(istream& is);

		void ReadStop(ostream& os, const Catalogue& transport_catalogue, string_view request);

		void ReadBus(ostream& os, const Catalogue& transport_catalogue, string_view request);

		std::string SeparateWord(const std::string& str, size_t begin, size_t size);

		std::string SeparateTrimmedWord(const std::string& str, size_t begin, size_t end);
	
	}

}

