#ifndef HUBFLOW_TESTS_SUPPORT_FIXTURES_HPP
#define HUBFLOW_TESTS_SUPPORT_FIXTURES_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"
#include "hubflow/routing/domain/entities/roadMap.hpp"

#include <vector>

namespace hubflow::test
{

/// @brief Dataset inicial de ocho envíos con su punto de entrega en el mapa.
std::vector<logistics::domain::ShipmentDraft> initialDataset();

/// @brief Construye un borrador válido.
logistics::domain::ShipmentDraft draft(const std::string&              code,
                                       logistics::domain::ServiceLevel service,
                                       logistics::domain::Zone         zone   = logistics::domain::Zone::Norte,
                                       double                          weight = 1.0);

/// @brief Carga en @p map la ciudad de referencia (HUB + barrios de CENTRO, NORTE y SUR).
void buildReferenceCity(routing::domain::RoadMap& map);

/// @brief Contexto de actor de prueba.
shared::domain::ActorContext actor(std::int64_t at = 1700000000);

} // namespace hubflow::test

#endif // HUBFLOW_TESTS_SUPPORT_FIXTURES_HPP
