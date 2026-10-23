#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_PENDING_SHIPMENTS_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_PENDING_SHIPMENTS_USE_CASE_HPP

#include "hubflow/logistics/domain/services/pendingShipments.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Devuelve los envíos que esperan despacho, en el orden en que se despacharán.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 *
 * @par Idempotencia
 * Sí. Es una lectura sin efectos.
 */
class ListPendingShipmentsUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~ListPendingShipmentsUseCase() = default;

    virtual const domain::PendingShipments& execute(const shared::application::Principal& principal) const = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_PENDING_SHIPMENTS_USE_CASE_HPP
