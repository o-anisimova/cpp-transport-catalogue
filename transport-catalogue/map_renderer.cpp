#include "map_renderer.h"

using namespace std;

namespace transport {
	
	namespace renderer {
		
		void MapRenderer::SetRenderSettings(const RenderSettings& render_settings) {
			render_settings_ = render_settings;
		}

        svg::Document MapRenderer::RenderMap(const std::vector<const Bus*>& bus_list, const std::vector<const Stop*>& stop_list) const {
            svg::Document document;
            SphereProjector proj = GetMapCoordinates(bus_list);

            RenderBusLines(document, bus_list, proj);
            RenderBusLabels(document, bus_list, proj);
            RenderStopSigns(document, stop_list, proj);
            RenderStopLabels(document, stop_list, proj);

            return document;
        }

        SphereProjector MapRenderer::GetMapCoordinates(const std::vector<const Bus*>& bus_list) const{
            //Заполняем вектор  географических координат
            std::vector<geo::Coordinates> geo_coords;
            for (const Bus* bus : bus_list) {
                for (const auto& stop : bus->route) {
                    geo_coords.push_back(stop->coordinates);
                }
            }

            // Создаём проектор сферических координат на карту
            return SphereProjector{
                geo_coords.begin(), geo_coords.end(), render_settings_.width, render_settings_.height, render_settings_.padding
            };
        }

        void MapRenderer::RenderBusLines(svg::Document& document, const std::vector<const Bus*>& bus_list, const SphereProjector& proj) const {
            const size_t COLOR_PALETTE_MAX_ELEM = render_settings_.color_palette.size() - 1;
            size_t j = 0;
            for (size_t i = 0; i < bus_list.size(); ++i) {
                if (!bus_list[i]->route.empty()) {
                    svg::Polyline polyline;

                    // Проецируем и выводим координаты
                    for (const auto& stop : GetBusFullRoute(bus_list[i])) {
                        const svg::Point screen_coord = proj(stop->coordinates);
                        polyline.AddPoint(screen_coord);
                    }
                    polyline.SetFillColor(svg::Color{}).SetStrokeColor(render_settings_.color_palette[j]).SetStrokeWidth(render_settings_.line_width);
                    polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                    document.Add(polyline);
                    
                    if (j == COLOR_PALETTE_MAX_ELEM) {
                        j = 0;
                    }
                    else {
                        ++j;
                    }
                }
            }
        }

        void MapRenderer::RenderBusLabels(svg::Document& document, const std::vector<const Bus*>& bus_list, const SphereProjector& proj) const {
            const size_t COLOR_PALETTE_MAX_ELEM = render_settings_.color_palette.size() - 1;
            size_t j = 0;
            for (size_t i = 0; i < bus_list.size(); ++i) {
                if (!bus_list[i]->route.empty()) {
                    svg::Text text;
                    text.SetPosition(proj(bus_list[i]->route[0]->coordinates)).SetOffset(render_settings_.bus_label_offset);
                    text.SetFontSize(render_settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
                    text.SetData(bus_list[i]->bus_name);

                    svg::Text underlayer_text = text;
                    underlayer_text.SetFillColor(render_settings_.underlayer_color).SetStrokeColor(render_settings_.underlayer_color).SetStrokeWidth(render_settings_.underlayer_width);
                    underlayer_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                    document.Add(underlayer_text);

                    text.SetFillColor(render_settings_.color_palette[j]);
                    document.Add(text);

                    if (!bus_list[i]->is_roundtrip && bus_list[i]->route.front() != bus_list[i]->route.back()) {
                        //Проецируем координаты для второй надписи
                        svg::Point point = proj(bus_list[i]->route.back()->coordinates);
                        //Подложка с новыми координатами
                        underlayer_text.SetPosition(point);
                        document.Add(underlayer_text);
                        //Надпись с новыми координатами
                        text.SetPosition(point);
                        document.Add(text);
                    }

                    if (j == COLOR_PALETTE_MAX_ELEM) {
                        j = 0;
                    }
                    else {
                        ++j;
                    }
                }
            }
        }

		void MapRenderer::RenderStopSigns(svg::Document& document, const std::vector<const Stop*>& stop_list, const SphereProjector& proj) const {
            for (const Stop* stop : stop_list) {
                svg::Circle circle;
                circle.SetCenter(proj(stop->coordinates)).SetRadius(render_settings_.stop_radius).SetFillColor("white"s);
                document.Add(circle);
            }
		}

        void MapRenderer::RenderStopLabels(svg::Document& document, const std::vector<const Stop*>& stop_list, const SphereProjector& proj) const {
            for (const Stop* stop : stop_list) {
                svg::Text text;
                text.SetPosition(proj(stop->coordinates)).SetOffset(render_settings_.stop_label_offset);
                text.SetFontSize(render_settings_.stop_label_font_size).SetFontFamily("Verdana"s);
                text.SetData(stop->stop_name);

                svg::Text underlayer_text = text;
                underlayer_text.SetFillColor(render_settings_.underlayer_color).SetStrokeColor(render_settings_.underlayer_color).SetStrokeWidth(render_settings_.underlayer_width); 
                underlayer_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                document.Add(underlayer_text);

                text.SetFillColor("black"s);
                document.Add(text);
            }
        }

	}

}
