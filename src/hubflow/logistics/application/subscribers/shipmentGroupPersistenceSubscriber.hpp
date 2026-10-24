#ifndef HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_SHIPMENT_GROUP_PERSISTENCE_SUBSCRIBER_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_SHIPMENT_GROUP_PERSISTENCE_SUBSCRIBER_HPP

#include "hubflow/logistics/application/ports/out/shipmentGroupRepository.hpp"
#include "hubflow/logistics/domain/events/shipmentGroupEvents.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::logistics::application
{

    /// @brief Guarda los grupos de envíos que viajan juntos cuando se forman y los borra cuando se deshacen.
    class ShipmentGroupPersistenceSubscriber final : public shared::domain::DomainEventVisitor
    {
    public:
        explicit ShipmentGroupPersistenceSubscriber(ShipmentGroupRepository& repository);

    private:
        void on(const domain::ShipmentGroupFormed& event) override;
        void on(const domain::ShipmentGroupDissolved& event) override;

        ShipmentGroupRepository& m_repository;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_SHIPMENT_GROUP_PERSISTENCE_SUBSCRIBER_HPP
