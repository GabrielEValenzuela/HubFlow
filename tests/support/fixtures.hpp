#ifndef HUBFLOW_TESTS_SUPPORT_FIXTURES_HPP
#define HUBFLOW_TESTS_SUPPORT_FIXTURES_HPP

#include "hubflow/identity/domain/services/accessPolicy.hpp"
#include "hubflow/identity/domain/services/courierWorkload.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/logistics/domain/services/catalogPriorityPolicy.hpp"
#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"

#include <map>
#include <string>
#include <vector>

namespace hubflow::test
{

/// @brief Dataset inicial de ocho envíos con su punto de entrega en el mapa.
std::vector<logistics::domain::ShipmentDraft> initialDataset();

/// @brief Construye un borrador válido.
logistics::domain::ShipmentDraft
draft(const std::string& code, const std::string& service, const std::string& zone = "NORTE", double weight = 1.0);

/// @brief Carga CENTRO, NORTE y SUR en el catálogo de zonas.
void seedZones(logistics::domain::ZoneCatalog& zones);

/// @brief Carga EXPRESS (3), PRIORITARIO (2) y ESTANDAR (1) en el catálogo de servicios.
void seedServiceLevels(logistics::domain::ServiceLevelCatalog& services);

/// @brief Carga FRAGIL, REFRIGERADO y VOLUMINOSO en el catálogo de características.
void seedFeatures(logistics::domain::FeatureCatalog& features);

/// @brief Concede a cada rol los permisos que tiene la operación por defecto.
void grantDefaultPermissions(identity::domain::AccessPolicy& policy);

/// @brief Carga en @p map la ciudad de referencia (HUB + barrios de CENTRO, NORTE y SUR).
void buildReferenceCity(routing::domain::RoadMap&                               map,
                        const std::map<std::string, routing::domain::RoadNode>& overrides = {});

/// @brief Catálogo de servicios de ejemplo con su política de prioridad, para armar listas de pendientes.
struct ReferencePriorities
{
    ReferencePriorities();

    logistics::domain::ServiceLevelCatalog   services;
    logistics::domain::CatalogPriorityPolicy policy;
};

/// @brief Ningún chofer tiene trabajo pendiente.
class IdleCouriers final : public identity::domain::CourierWorkload
{
private:
    bool hasRouteInProgress(std::uint64_t) const override
    {
        return false;
    }
};

/// @brief Contexto de actor de prueba.
shared::domain::ActorContext actor(std::int64_t at = 1700000000);

} // namespace hubflow::test

#endif // HUBFLOW_TESTS_SUPPORT_FIXTURES_HPP
