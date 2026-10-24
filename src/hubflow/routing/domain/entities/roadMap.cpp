#include "hubflow/routing/domain/entities/roadMap.hpp"

#include "hubflow/routing/domain/events/roadMapEvents.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <cmath>

namespace hubflow::routing::domain
{
    namespace
    {
        constexpr double COST_TOLERANCE {1e-9};
        constexpr std::size_t MAX_ID_LENGTH {16};
        constexpr std::size_t MAX_LABEL_LENGTH {80};
    } // namespace

    std::size_t RoadMap::addNode(const RoadNode& node)
    {
        if (node.id.empty() || indexOf(node.id).has_value())
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Conflict, "road_node.invalid", "Nodo vacío o duplicado en el mapa"};
        }
        return m_graph.addVertex(node);
    }

    std::size_t RoadMap::declareNode(const RoadNode& node,
                                     const ds::DynamicArray<RoadLink>& links,
                                     const shared::domain::ActorContext& context)
    {
        if (node.id.empty() || node.id.size() > MAX_ID_LENGTH || node.label.empty() ||
            node.label.size() > MAX_LABEL_LENGTH)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "road_node.invalid_data",
                                                  "El punto necesita un código de hasta 16 caracteres y un nombre"};
        }
        if (links.isEmpty())
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "road_node.unconnected",
                                                  "El punto nuevo debe unirse al menos con una calle"};
        }
        RoadNode created {node};
        created.isHub = false;
        created.active = true;
        for (std::size_t linkIndex = 0; linkIndex < links.size(); ++linkIndex)
        {
            const RoadNode& target = this->node(require(links[linkIndex].destinationId));
            const double straight {std::hypot(created.x - target.x, created.y - target.y)};
            for (std::size_t earlierLinkIndex = 0; earlierLinkIndex < linkIndex; ++earlierLinkIndex)
            {
                if (links[earlierLinkIndex].destinationId == links[linkIndex].destinationId)
                {
                    throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                          "road_edge.duplicate",
                                                          "Hay dos calles hacia el mismo punto"};
                }
            }
            if (links[linkIndex].costKm + COST_TOLERANCE < straight)
            {
                throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                      "road_edge.inadmissible",
                                                      "La calle no puede ser más corta que la línea recta"};
            }
        }
        const std::size_t index {addNode(created)};
        for (std::size_t linkIndex = 0; linkIndex < links.size(); ++linkIndex)
        {
            connect(created.id, links[linkIndex].destinationId, links[linkIndex].costKm);
        }
        raise(new RoadNodeAdded {created, links, context});
        return index;
    }

    void RoadMap::reviseNode(const std::string& id,
                             const std::optional<std::string>& zone,
                             const std::optional<bool>& active,
                             const shared::domain::ActorContext& context)
    {
        const std::size_t index {require(id)};
        RoadNode next {m_graph.vertex(index)};
        if (zone.has_value())
        {
            next.zone = *zone;
        }
        if (active.has_value())
        {
            if (!*active && next.isHub)
            {
                throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                      "road_node.hub_required",
                                                      "El centro de distribución no se puede desactivar"};
            }
            next.active = *active;
        }
        const RoadNode& current = m_graph.vertex(index);
        if (next.zone != current.zone || next.active != current.active)
        {
            m_graph.replaceVertex(index, next);
            raise(new RoadNodeChanged {next, context});
        }
    }

    void RoadMap::connect(const std::string& originId, const std::string& destinationId, const double costKm)
    {
        const std::size_t origin {require(originId)};
        const std::size_t destination {require(destinationId)};
        if (origin == destination || costKm + COST_TOLERANCE < straightLine(origin, destination))
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Validation,
                "road_edge.inadmissible",
                "La arista es un lazo o su costo es menor que la distancia en línea recta"};
        }
        m_graph.addEdge(origin, destination, costKm);
    }

    std::optional<std::size_t> RoadMap::indexOf(const std::string& id) const
    {
        return m_graph.findVertex([&id](const RoadNode& node) { return node.id == id; });
    }

    std::size_t RoadMap::require(const std::string& id) const
    {
        const auto index = indexOf(id);
        if (!index.has_value())
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::NotFound, "road_node.not_found", "El nodo no existe en el mapa"};
        }
        return *index;
    }

    std::size_t RoadMap::hub() const
    {
        const auto index = m_graph.findVertex([](const RoadNode& node) { return node.isHub; });
        if (!index.has_value())
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::InvalidState, "road_map.no_hub", "El mapa no tiene centro de distribución"};
        }
        return *index;
    }

    double RoadMap::straightLine(const std::size_t origin, const std::size_t destination) const
    {
        const RoadNode& originNode = m_graph.vertex(origin);
        const RoadNode& destinationNode = m_graph.vertex(destination);
        return std::hypot(originNode.x - destinationNode.x, originNode.y - destinationNode.y);
    }

    const RoadNode& RoadMap::node(const std::size_t index) const
    {
        return m_graph.vertex(index);
    }

    const ds::Graph<RoadNode>& RoadMap::graph() const noexcept
    {
        return m_graph;
    }

    std::size_t RoadMap::nodeCount() const noexcept
    {
        return m_graph.vertexCount();
    }

} // namespace hubflow::routing::domain
