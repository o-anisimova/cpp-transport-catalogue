#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport;

int main() {
	Catalogue transport_catalogue;
	input_reader::InputReader input_reader(transport_catalogue);
	input_reader.ReadData();
	stat_reader::OutputData(transport_catalogue);
}