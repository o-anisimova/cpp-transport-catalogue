#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <optional>

namespace transport{

    struct BusStat {
        int stops_qty = 0;
        int unique_stops_qty = 0;
        int route_length = 0;
        double curvature = 0.0;
    };

    // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
    // с другими подсистемами приложения.
    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& transport_catalogue, const renderer::MapRenderer& renderer );

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& transport_catalogue_;
        const renderer::MapRenderer& map_renderer_;
    };

} // namespace transport