#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_PUM_AGENT_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_PUM_AGENT_HPP

#include "hubflow/routing/domain/services/search/searchStrategy.hpp"

namespace hubflow::routing::domain
{

/**
 * @brief Agente PUM (Proveedor de Última Milla): agente basado en objetivos que se mueve por el
 * mapa resolviendo cada tramo con su estrategia de búsqueda.
 */
class PumAgent final
{
public:
    PumAgent(const RoadMap& map, const SearchStrategy& strategy);

    /// @brief Resuelve el tramo @p from → @p to y devuelve la traza de la búsqueda.
    SearchTrace travel(std::size_t from, std::size_t to) const;

    const RoadMap&  map() const noexcept;
    SearchAlgorithm algorithm() const noexcept;

private:
    const RoadMap&        m_map;
    const SearchStrategy& m_strategy;
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_PUM_AGENT_HPP
