#pragma once

#include "transport_catalogue.h"

namespace transport {

	namespace stat_reader {

		void OutputData(const Catalogue& transport_catalogue);

		std::vector<std::string> InputRequests();

		void ReadStop(const Catalogue& transport_catalogue, string_view request);

		void ReadBus(const Catalogue& transport_catalogue, string_view request);

		std::string SeparateWord(const std::string& str, size_t begin, size_t size);

		std::string SeparateTrimmedWord(const std::string& str, size_t begin, size_t end);
	
	}

}

