#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_SHIPMENT_GROUP_OF_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_SHIPMENT_GROUP_OF_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipmentGroup.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

/**
 * @brief Indica con qué otros paquetes viaja un paquete: devuelve su grupo o nada si viaja solo.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 * - NotFound: el paquete no existe.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class FindShipmentGroupOfShipmentUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~FindShipmentGroupOfShipmentUseCase() = default;

    virtual const domain::ShipmentGroup* execute(const shared::application::Principal& principal,
                                                 const std::string&                    shipmentCode) const = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_SHIPMENT_GROUP_OF_SHIPMENT_USE_CASE_HPP
