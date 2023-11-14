#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "serialization.h"

namespace transport{

    // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
    // с другими подсистемами приложения.
    class RequestHandler {
    public:
        RequestHandler(TransportCatalogue& transport_catalogue);

        void AddStop(const Stop& stop);
        void AddBus(const Bus& bus);
        void SetStopsDistance(Stop* lhs_stop, Stop* rhs_stop, int distance);

        // Возвращает указатель на остановку (запрос Stop)
        Stop* FindStop(std::string_view stop_name) const;
        // Возвращает список автобусов, останавливающихся на остановке (запрос Stop)
        const std::set<std::string_view>& GetStopToBusesList(Stop* stop) const;

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
        
        // Устанавливает настройки карты остановок и маршрутов (запрос Map)
        void SetRenderSettings(const renderer::RenderSettings& render_settings);
        // Возвращает карту остановок и маршрутов (запрос Map)
        svg::Document RenderMap() const;

        // Устанавливает настройки для формирования маршрута (запрос Route)
        void SetRoutingSettings(const RoutingSettings routing_settings);

        void BuildGraph();

        std::optional<Route> BuildRoute(std::string_view from, std::string_view to) const;

        void Serialize(std::ofstream& output, const renderer::RenderSettings& settings, const RoutingSettings& routing_settings) const;

        void Deserialize(std::ifstream& input);

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        TransportCatalogue& transport_catalogue_;
        renderer::MapRenderer map_renderer_;
        TransportRouter transport_router_;
    };

} // namespace transport