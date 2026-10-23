#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PRIORITY_POLICY_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PRIORITY_POLICY_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"

namespace hubflow::logistics::domain
{

/**
 * @brief Criterio que decide qué envío sale antes de la planta (Strategy).
 *
 * La lista de pendientes le pregunta la prioridad de cada envío y, a igual prioridad, desempata por orden de
 * llegada. Cambiar el criterio no requiere tocar la lista.
 */
class PriorityPolicy
{
public:
    virtual ~PriorityPolicy() = default;

    /// @brief Prioridad de despacho del envío: un valor mayor sale antes.
    virtual unsigned priorityOf(const Shipment& shipment) const = 0;

    /// @brief La mayor prioridad que puede tener un envío hoy (0 si no hay ninguna definida).
    virtual unsigned highest() const = 0;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PRIORITY_POLICY_HPP
