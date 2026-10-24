#include "hubflow/routing/domain/services/pumAgent.hpp"

namespace hubflow::routing::domain
{

    PumAgent::PumAgent(const RoadMap& map, const SearchStrategy& strategy)
        : m_map {map}
        , m_strategy {strategy}
    {
    }

    SearchTrace PumAgent::travel(const std::size_t origin, const std::size_t destination) const
    {
        return m_strategy.search(m_map, origin, destination);
    }

    const RoadMap& PumAgent::roadMap() const noexcept
    {
        return m_map;
    }

    SearchAlgorithm PumAgent::algorithm() const noexcept
    {
        return m_strategy.algorithm();
    }

} // namespace hubflow::routing::domain
