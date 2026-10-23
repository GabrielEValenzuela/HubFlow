#ifndef HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_SHIPMENT_PERSISTENCE_SUBSCRIBER_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_SHIPMENT_PERSISTENCE_SUBSCRIBER_HPP

#include "hubflow/logistics/application/ports/out/shipmentRepository.hpp"
#include "hubflow/logistics/domain/events/shipmentEvents.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::logistics::application
{

/// @brief Persiste el estado y los movimientos de cada envío a partir de sus eventos.
class ShipmentPersistenceSubscriber final : public shared::domain::DomainEventVisitor
{
public:
    explicit ShipmentPersistenceSubscriber(ShipmentRepository& repository);

private:
    void on(const domain::ShipmentRegistered& event) override;
    void on(const domain::ShipmentClassified& event) override;
    void on(const domain::ShipmentDispatched& event) override;
    void on(const domain::ShipmentRescheduled& event) override;
    void on(const domain::ShipmentDelivered& event) override;

    void persistChange(const domain::ShipmentEvent& event);

    ShipmentRepository& m_repository;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_SHIPMENT_PERSISTENCE_SUBSCRIBER_HPP
