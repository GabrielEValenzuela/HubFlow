#ifndef HUBFLOW_ROUTING_DOMAIN_ENTITIES_DELIVERY_ROUTE_HPP
#define HUBFLOW_ROUTING_DOMAIN_ENTITIES_DELIVERY_ROUTE_HPP

#include "hubflow/routing/domain/services/pumAgent.hpp"
#include "hubflow/routing/domain/valueobjects/stopOutcome.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <string>

namespace hubflow::routing::domain
{

/// @brief Repartidor asignado (referencia por identidad al contexto de identidad).
struct Courier
{
    std::uint64_t id {0};
    std::string   name;
};

/// @brief Parada: envío (por código, no por puntero) y nodo del mapa donde se entrega.
struct RouteStop
{
    std::size_t order {0};
    std::string shipmentCode;
    std::string nodeId;
    StopOutcome outcome {StopOutcome::Pending};
};

/// @brief Datos persistidos de una ruta para reconstituirla.
struct RouteRecord
{
    std::uint64_t               id {0};
    Courier                     courier;
    SearchAlgorithm             algorithm {SearchAlgorithm::AStar};
    std::int64_t                createdAt {0};
    ds::DynamicArray<RouteStop> stops;
};

/**
 * @brief Agregado raíz "hoja de ruta" de un PUM.
 *
 * Las paradas están en orden de descarga; el PUM calcula los tramos HUB → parada 1 → … →
 * parada n → HUB. La ruta es dueña de las trazas de sus tramos (las libera en su destructor).
 * Referencia a los envíos por su código: los agregados de otro contexto no se apuntan.
 */
class DeliveryRoute final : public shared::domain::AggregateRoot
{
public:
    /**
     * @brief Fábrica: crea y planifica la ruta registrando DeliveryRouteAssigned.
     * @throws business_error si no hay paradas o algún tramo no tiene camino.
     */
    static DeliveryRoute* assign(std::uint64_t                       id,
                                 const Courier&                      courier,
                                 const ds::DynamicArray<RouteStop>&  stops,
                                 const PumAgent&                     agent,
                                 const shared::domain::ActorContext& context);

    /// @brief Reconstituye una ruta persistida (replanifica los tramos, sin eventos).
    static DeliveryRoute* restore(const RouteRecord& record, const PumAgent& agent);

    ~DeliveryRoute() override;

    /**
     * @brief Registra el resultado de la parada @p order y emite RouteStopResolved.
     *
     * Es idempotente: informar el mismo resultado que ya está registrado no cambia nada ni emite
     * eventos. Informar un resultado distinto sobre una parada ya resuelta es un conflicto.
     * @throws business_error NotFound (parada inexistente), Validation (resultado Pendiente) o
     * Conflict (la parada ya tiene otro resultado).
     */
    const RouteStop& resolveStop(std::size_t order, StopOutcome outcome, const shared::domain::ActorContext& context);

    /**
     * @brief Pasa la ruta a otro repartidor y registra DeliveryRouteReassigned. Si ya es de ese repartidor no hace
     * nada.
     * @throws business_error (Conflict) si la ruta ya está completada.
     */
    void reassign(const Courier& courier, const shared::domain::ActorContext& context);

    const RouteStop& stop(std::size_t order) const;

    /// @brief Visita los tramos en orden de recorrido.
    template <typename Visitor>
    void forEachSegment(Visitor visit) const
    {
        m_segments.forEach(
            [&visit](const SearchTrace* pTrace)
            {
                visit(*pTrace);
            });
    }

    std::uint64_t                      id() const noexcept;
    const Courier&                     courier() const noexcept;
    SearchAlgorithm                    algorithm() const noexcept;
    std::int64_t                       createdAt() const noexcept;
    const ds::DynamicArray<RouteStop>& stops() const noexcept;
    RouteStatus                        status() const noexcept;
    double                             totalCostKm() const noexcept;
    std::size_t                        totalNodesExplored() const noexcept;
    RouteRecord                        record() const;

private:
    explicit DeliveryRoute(const RouteRecord& record);

    void plan(const PumAgent& agent);

    std::uint64_t                      m_id;
    Courier                            m_courier;
    SearchAlgorithm                    m_algorithm;
    std::int64_t                       m_createdAt;
    ds::DynamicArray<RouteStop>        m_stops;
    ds::SinglyLinkedList<SearchTrace*> m_segments;
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_ENTITIES_DELIVERY_ROUTE_HPP
