#ifndef HUBFLOW_ROUTING_DOMAIN_EVENTS_ROAD_MAP_EVENTS_HPP
#define HUBFLOW_ROUTING_DOMAIN_EVENTS_ROAD_MAP_EVENTS_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/routing/domain/valueobjects/roadNode.hpp"
#include "hubflow/shared/domain/domainEvent.hpp"

namespace hubflow::routing::domain
{

    /// @brief Se agregó un punto al mapa junto con las calles que lo unen a la red.
    class RoadNodeAdded final : public shared::domain::DomainEvent
    {
    public:
        RoadNodeAdded(RoadNode node, ds::DynamicArray<RoadLink> links, shared::domain::ActorContext context);

        const RoadNode& node() const noexcept;
        const ds::DynamicArray<RoadLink>& links() const noexcept;

    private:
        void accept(shared::domain::DomainEventVisitor& visitor) const override;

        RoadNode m_node;
        ds::DynamicArray<RoadLink> m_links;
    };

    /// @brief Un punto del mapa cambió de zona o se activó o desactivó. Lleva el estado resultante.
    class RoadNodeChanged final : public shared::domain::DomainEvent
    {
    public:
        RoadNodeChanged(RoadNode node, shared::domain::ActorContext context);

        const RoadNode& node() const noexcept;

    private:
        void accept(shared::domain::DomainEventVisitor& visitor) const override;

        RoadNode m_node;
    };

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_EVENTS_ROAD_MAP_EVENTS_HPP
