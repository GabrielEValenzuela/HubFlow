#include "hubflow/routing/domain/entities/roadMap.hpp"

#include "hubflow/routing/domain/events/roadMapEvents.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <cmath>

namespace hubflow::routing::domain
{
namespace
{
constexpr double      COST_TOLERANCE {1e-9};
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

std::size_t RoadMap::declareNode(const RoadNode&                     node,
                                 const ds::DynamicArray<RoadLink>&   links,
                                 const shared::domain::ActorContext& context)
{
    if (node.id.empty() || node.id.size() > MAX_ID_LENGTH || node.label.empty() || node.label.size() > MAX_LABEL_LENGTH)
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "road_node.invalid_data",
                                              "El punto necesita un código de hasta 16 caracteres y un nombre"};
    }
    if (links.empty())
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "road_node.unconnected",
                                              "El punto nuevo debe unirse al menos con una calle"};
    }
    RoadNode created {node};
    created.isHub  = false;
    created.active = true;
    for (std::size_t i = 0; i < links.size(); ++i)
    {
        const RoadNode& target = this->node(require(links[i].toId));
        const double    straight {std::hypot(created.x - target.x, created.y - target.y)};
        for (std::size_t j = 0; j < i; ++j)
        {
            if (links[j].toId == links[i].toId)
            {
                throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                      "road_edge.duplicate",
                                                      "Hay dos calles hacia el mismo punto"};
            }
        }
        if (links[i].costKm + COST_TOLERANCE < straight)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "road_edge.inadmissible",
                                                  "La calle no puede ser más corta que la línea recta"};
        }
    }
    const std::size_t index {addNode(created)};
    for (std::size_t i = 0; i < links.size(); ++i)
    {
        connect(created.id, links[i].toId, links[i].costKm);
    }
    raise(new RoadNodeAdded {created, links, context});
    return index;
}

void RoadMap::reviseNode(const std::string&                  id,
                         const std::optional<std::string>&   zone,
                         const std::optional<bool>&          active,
                         const shared::domain::ActorContext& context)
{
    const std::size_t index {require(id)};
    RoadNode          next {m_graph.vertex(index)};
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

void RoadMap::connect(const std::string& fromId, const std::string& toId, const double costKm)
{
    const std::size_t from {require(fromId)};
    const std::size_t to {require(toId)};
    if (from == to || costKm + COST_TOLERANCE < straightLine(from, to))
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation,
            "road_edge.inadmissible",
            "La arista es un lazo o su costo es menor que la distancia en línea recta"};
    }
    m_graph.addEdge(from, to, costKm);
}

std::optional<std::size_t> RoadMap::indexOf(const std::string& id) const
{
    return m_graph.findVertex(
        [&id](const RoadNode& node)
        {
            return node.id == id;
        });
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
    const auto index = m_graph.findVertex(
        [](const RoadNode& node)
        {
            return node.isHub;
        });
    if (!index.has_value())
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::InvalidState, "road_map.no_hub", "El mapa no tiene centro de distribución"};
    }
    return *index;
}

double RoadMap::straightLine(const std::size_t from, const std::size_t to) const
{
    const RoadNode& a = m_graph.vertex(from);
    const RoadNode& b = m_graph.vertex(to);
    return std::hypot(a.x - b.x, a.y - b.y);
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
