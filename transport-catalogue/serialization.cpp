#pragma once

#include <string>
#include <transport_catalogue.pb.h>
#include "serialization.h"

transport_catalogue_serialize::Coordinates ConvertCoordinates(double lat, double lng) {
	transport_catalogue_serialize::Coordinates result;
	result.set_lat(lat);
	result.set_lng(lng);
	return result;
}

transport_catalogue_serialize::Point ConvertPoint(double x, double y) {
	transport_catalogue_serialize::Point point;
	point.set_x(x);
	point.set_y(y);
	return point;
}

transport_catalogue_serialize::Color ConvertColor(const svg::Color& color) {
	transport_catalogue_serialize::Color result;

	if (std::holds_alternative<std::monostate>(color)) {
		result.set_is_monostate(true);
	}
	else if (std::holds_alternative<std::string>(color)) {
		result.set_color_name(std::get<std::string>(color));
	}
	else if (std::holds_alternative<svg::Rgb>(color)) {
		svg::Rgb rgb = std::get<svg::Rgb>(color);

		transport_catalogue_serialize::Rgb rgb_db;
		rgb_db.set_red(rgb.red);
		rgb_db.set_green(rgb.green);
		rgb_db.set_blue(rgb.blue);

		*result.mutable_rgb() = rgb_db;
	}
	else if (std::holds_alternative<svg::Rgba>(color)) {
		svg::Rgba rgba = std::get<svg::Rgba>(color);

		transport_catalogue_serialize::Rgba rgba_db;
		rgba_db.set_red(rgba.red);
		rgba_db.set_green(rgba.green);
		rgba_db.set_blue(rgba.blue);
		rgba_db.set_opacity(rgba.opacity);

		*result.mutable_rgba() = rgba_db;
	}

	return result;
}

transport_catalogue_serialize::RenderSettings ConvertRenderSettings(const transport::renderer::RenderSettings& settings) {
	transport_catalogue_serialize::RenderSettings result;
	result.set_width(settings.width);
	result.set_height(settings.height);
	result.set_padding(settings.padding);
	result.set_line_width(settings.line_width);
	result.set_stop_radius(settings.stop_radius);
	result.set_bus_label_font_size(settings.bus_label_font_size);
	result.set_stop_label_font_size(settings.stop_label_font_size);
	result.set_underlayer_width(settings.underlayer_width);

	*result.mutable_bus_label_offset() = ConvertPoint(settings.bus_label_offset.x, settings.bus_label_offset.y);
	*result.mutable_stop_label_offset() = ConvertPoint(settings.stop_label_offset.x, settings.stop_label_offset.y);
	*result.mutable_underlayer_color() = ConvertColor(settings.underlayer_color);

	for (const auto& color : settings.color_palette) {
		transport_catalogue_serialize::Color* color_ptr = result.add_color_palette();
		*color_ptr = ConvertColor(color);
	}

	return result;
}

transport_catalogue_serialize::RoutingSettings ConvertRoutingSettings(const transport::RoutingSettings& routing_settings) {
	transport_catalogue_serialize::RoutingSettings result;
	result.set_bus_velocity(routing_settings.bus_velocity);
	result.set_bus_wait_time(routing_settings.bus_wait_time);
	return result;
}

void ConvertStopsList(const transport::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& catalogue_db) {
	for (const transport::Stop& stop : catalogue.GetStopList()) {
		transport_catalogue_serialize::Stop* stop_ptr = catalogue_db.add_stop();
		stop_ptr->set_id(stop.id);
		stop_ptr->set_stop_name(stop.stop_name);
		*stop_ptr->mutable_coordinates() = ConvertCoordinates(stop.coordinates.lat, stop.coordinates.lng);
	}
}

void ConvertStopsDistances(const transport::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& catalogue_db) {
	for (const auto& distance : catalogue.GetStopsDistances()) {
		transport_catalogue_serialize::StopsDistance* distance_ptr = catalogue_db.add_distance();
		distance_ptr->set_stop_from_id(distance.first.first->id);
		distance_ptr->set_stop_to_id(distance.first.second->id);
		distance_ptr->set_distance(distance.second);
	}
}

void ConvertBusList(const transport::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& catalogue_db) {
	for (const transport::Bus& bus : catalogue.GetBusList()) {
		transport_catalogue_serialize::Bus* bus_ptr = catalogue_db.add_bus();
		bus_ptr->set_bus_name(bus.bus_name);
		bus_ptr->set_is_roundtrip(bus.is_roundtrip);
		for (const transport::Stop* stop_ptr : bus.route) {
			bus_ptr->add_stop_ids(stop_ptr->id);
		}
	}
}

void SerializeCatalogue(std::ofstream& output, const transport::TransportCatalogue& catalogue, const transport::renderer::RenderSettings& settings, const transport::RoutingSettings& routing_settings) {
	transport_catalogue_serialize::TransportCatalogue catalogue_db;

	ConvertStopsList(catalogue, catalogue_db);
	ConvertStopsDistances(catalogue, catalogue_db);
	ConvertBusList(catalogue, catalogue_db);

	*catalogue_db.mutable_render_settings() = ConvertRenderSettings(settings);
	*catalogue_db.mutable_routing_settings() = ConvertRoutingSettings(routing_settings);

	catalogue_db.SerializeToOstream(&output);
}

svg::Color ConvertColor(const transport_catalogue_serialize::Color& color) {
	svg::Color result;
	if (color.is_monostate()) {
		result = std::monostate();
	}
	else if (color.has_rgb()) {
		svg::Rgb rgb;
		rgb.red = color.rgb().red();
		rgb.green = color.rgb().green();
		rgb.blue = color.rgb().blue();

		result = rgb;
	}
	else if (color.has_rgba()) {
		svg::Rgba rgba;
		rgba.red = color.rgba().red();
		rgba.green = color.rgba().green();
		rgba.blue = color.rgba().blue();
		rgba.opacity = color.rgba().opacity();

		result = rgba;
	}
	else {
		result = color.color_name();
	}

	return result;
}

void ConvertRenderSettings(const transport_catalogue_serialize::RenderSettings& render_settings, transport::renderer::RenderSettings& result) {
	result.width = render_settings.width();
	result.height = render_settings.height();
	result.padding = render_settings.padding();
	result.line_width = render_settings.line_width();
	result.stop_radius = render_settings.stop_radius();
	result.bus_label_font_size = render_settings.bus_label_font_size();
	result.stop_label_font_size = render_settings.stop_label_font_size();
	result.underlayer_width = render_settings.underlayer_width();
	result.bus_label_offset.x = render_settings.bus_label_offset().x();
	result.bus_label_offset.y = render_settings.bus_label_offset().y();
	result.stop_label_offset.x = render_settings.stop_label_offset().x();
	result.stop_label_offset.y = render_settings.stop_label_offset().y();
	result.underlayer_color = ConvertColor(render_settings.underlayer_color());

	for (size_t i = 0; i < render_settings.color_palette_size(); ++i) {
		svg::Color color = ConvertColor(render_settings.color_palette()[i]);
		result.color_palette.push_back(color);
	}
}

void ConvertRoutingSettings(const transport_catalogue_serialize::RoutingSettings& routing_settings, transport::RoutingSettings& result) {
	result.bus_velocity = routing_settings.bus_velocity();
	result.bus_wait_time = routing_settings.bus_wait_time();
}

void ConvertStopsList(const transport_catalogue_serialize::TransportCatalogue& catalogue_db, transport::TransportCatalogue& catalogue) {
	for (size_t i = 0; i < catalogue_db.stop_size(); ++i) {
		transport::Stop stop;
		stop.id = catalogue_db.stop()[i].id();
		stop.stop_name = catalogue_db.stop()[i].stop_name();
		stop.coordinates.lat = catalogue_db.stop()[i].coordinates().lat();
		stop.coordinates.lng = catalogue_db.stop()[i].coordinates().lng();
		catalogue.AddStop(stop);
	}
}

void ConvertStopsDistances(const transport_catalogue_serialize::TransportCatalogue& catalogue_db, transport::TransportCatalogue& catalogue) {
	for (size_t i = 0; i < catalogue_db.distance_size(); ++i) {
		const transport::Stop* stop_from_ptr = catalogue.FindStopByPos(catalogue_db.distance()[i].stop_from_id());
		const transport::Stop* stop_to_ptr = catalogue.FindStopByPos(catalogue_db.distance()[i].stop_to_id());
		int distance = catalogue_db.distance()[i].distance();
		catalogue.SetStopsDistance(stop_from_ptr, stop_to_ptr, distance);
	}
}

void ConvertBusList(const transport_catalogue_serialize::TransportCatalogue& catalogue_db, transport::TransportCatalogue& catalogue) {
	for (size_t i = 0; i < catalogue_db.bus_size(); ++i) {
		transport::Bus bus;
		bus.bus_name = catalogue_db.bus()[i].bus_name();
		bus.is_roundtrip = catalogue_db.bus()[i].is_roundtrip();

		for (size_t j = 0; j < catalogue_db.bus()[i].stop_ids_size(); ++j) {
			bus.route.push_back(catalogue.FindStopByPos(catalogue_db.bus()[i].stop_ids()[j]));
		}

		catalogue.AddBus(bus);
	}
}

void DeserializeCatalogue(std::istream& input, transport::TransportCatalogue& catalogue, transport::renderer::RenderSettings& render_settings, transport::RoutingSettings& routing_settings) {
	transport_catalogue_serialize::TransportCatalogue catalogue_db;
	catalogue_db.ParseFromIstream(&input);

	ConvertStopsList(catalogue_db, catalogue);
	ConvertStopsDistances(catalogue_db, catalogue);
	ConvertBusList(catalogue_db, catalogue);
	ConvertRenderSettings(catalogue_db.render_settings(), render_settings);
	ConvertRoutingSettings(catalogue_db.routing_settings(), routing_settings);
}