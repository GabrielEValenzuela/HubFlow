#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_SHIPMENT_GROUP_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_SHIPMENT_GROUP_USE_CASE_HPP

#include "hubflow/logistics/domain/entities/shipmentGroup.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

/**
 * @brief Devuelve un grupo de paquetes por su identidad.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 * - NotFound: el grupo no existe.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class GetShipmentGroupUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~GetShipmentGroupUseCase() = default;

    virtual const domain::ShipmentGroup& execute(const shared::application::Principal& principal,
                                                 const std::string&                    groupId) const = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_GET_SHIPMENT_GROUP_USE_CASE_HPP
