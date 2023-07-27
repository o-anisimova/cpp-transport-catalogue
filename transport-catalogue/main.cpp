#include "json_reader.h"

int main()
{
    transport::Catalogue transport_catalogue;
	transport::JsonReader json_reader(transport_catalogue);
	json_reader.FillCatalogue(cin);
	json_reader.OutputData(cout);
}