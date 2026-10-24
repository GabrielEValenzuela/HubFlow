#ifndef HUBFLOW_ROUTING_DOMAIN_ENTITIES_ROAD_MAP_HPP
#define HUBFLOW_ROUTING_DOMAIN_ENTITIES_ROAD_MAP_HPP

#include "datastructures/dynamicArray.hpp"
#include "datastructures/graph.hpp"
#include "hubflow/routing/domain/valueobjects/roadNode.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <optional>

namespace hubflow::routing::domain
{

/**
 * @brief Mapa de la ciudad: grafo no dirigido ponderado en kilómetros.
 *
 * Invariante: el costo de cada arista es mayor o igual que la distancia euclídea entre sus
 * extremos. Por eso la heurística euclídea nunca sobreestima el costo restante: es admisible y
 * consistente, condición para que A* devuelva el camino de costo mínimo.
 */
class RoadMap final : public shared::domain::AggregateRoot
{
public:
    RoadMap() = default;

    RoadMap(const RoadMap&)            = delete;
    RoadMap& operator=(const RoadMap&) = delete;

    /// @throws business_error si el id está vacío o repetido.
    std::size_t addNode(const RoadNode& node);

    /**
     * @brief Da de alta un punto nuevo del mapa junto con las calles que lo unen a puntos existentes, y registra
     * RoadNodeAdded. O se agrega todo o no se agrega nada.
     * @throws business_error Conflict (id repetido), Validation (datos inválidos, sin calles, calle duplicada o más
     * corta que la línea recta) o NotFound (algún extremo no existe).
     */
    std::size_t declareNode(const RoadNode&                     node,
                            const ds::DynamicArray<RoadLink>&   links,
                            const shared::domain::ActorContext& context);

    /**
     * @brief Reasigna la zona de un punto y/o lo activa o desactiva, y registra RoadNodeChanged si algo cambió.
     * Repetir el mismo pedido no cambia nada.
     * @throws business_error NotFound (punto inexistente) o Conflict (el centro de distribución no se desactiva).
     */
    void reviseNode(const std::string&                  id,
                    const std::optional<std::string>&   zone,
                    const std::optional<bool>&          active,
                    const shared::domain::ActorContext& context);

    /// @throws business_error si algún extremo no existe o el costo es menor a la línea recta.
    void connect(const std::string& originId, const std::string& destinationId, double costKm);

    /// @brief Índice del nodo por id (búsqueda lineal O(V)).
    std::optional<std::size_t> indexOf(const std::string& id) const;

    /// @throws business_error (NotFound) si el id no existe.
    std::size_t require(const std::string& id) const;

    /// @throws business_error (InvalidState) si el mapa no tiene HUB.
    std::size_t hub() const;

    /// @brief Distancia en línea recta entre dos nodos: heurística h(n) del PUM.
    double straightLine(std::size_t origin, std::size_t destination) const;

    const RoadNode&            node(std::size_t index) const;
    const ds::Graph<RoadNode>& graph() const noexcept;
    std::size_t                nodeCount() const noexcept;

private:
    ds::Graph<RoadNode> m_graph;
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_ENTITIES_ROAD_MAP_HPP
