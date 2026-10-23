#ifndef HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_CATALOG_PERSISTENCE_SUBSCRIBER_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_CATALOG_PERSISTENCE_SUBSCRIBER_HPP

#include "hubflow/logistics/application/ports/out/catalogRepository.hpp"
#include "hubflow/shared/domain/domainEventVisitor.hpp"

namespace hubflow::logistics::application
{

/// @brief Persiste zonas, niveles de servicio y características cada vez que se dan de alta o cambian.
class CatalogPersistenceSubscriber final : public shared::domain::DomainEventVisitor
{
public:
    explicit CatalogPersistenceSubscriber(CatalogRepository& repository);

private:
    void on(const domain::ZoneChanged& event) override;
    void on(const domain::ServiceLevelChanged& event) override;
    void on(const domain::ShipmentFeatureChanged& event) override;

    CatalogRepository& m_repository;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_SUBSCRIBERS_CATALOG_PERSISTENCE_SUBSCRIBER_HPP
