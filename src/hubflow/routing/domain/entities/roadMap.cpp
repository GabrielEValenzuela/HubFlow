#include "hubflow/routing/domain/entities/roadMap.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <cmath>

namespace hubflow::routing::domain
{
namespace
{
constexpr double COST_TOLERANCE {1e-9};
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
