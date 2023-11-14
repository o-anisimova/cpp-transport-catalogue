#pragma once

#include <fstream>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

void SerializeCatalogue(std::ofstream& output, const transport::TransportCatalogue& catalogue, const transport::renderer::RenderSettings& settings, const transport::RoutingSettings& routing_settings);

void DeserializeCatalogue(std::istream& input, transport::TransportCatalogue& catalogue, transport::renderer::RenderSettings& render_settings, transport::RoutingSettings& routing_settings);