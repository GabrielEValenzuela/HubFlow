#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_ZONE_REPORT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_ZONE_REPORT_HPP

#include "hubflow/logistics/domain/services/pendingShipments.hpp"

namespace hubflow::logistics::domain
{

    /// @brief Resumen de pendientes de una zona. Urgentes: los de la mayor prioridad del catálogo.
    struct ZoneSummary
    {
        std::size_t count {0};
        double totalWeightKg {0.0};
        std::size_t urgentCount {0};
    };

    /**
     * @brief Resumen recursivo por zona sobre los nodos de pendientes.
     *
     * - Caso base: @p pNode == nullptr → resumen vacío.
     * - Llamada recursiva: resume el resto de la lista (@c pNode->pNext).
     * - Información por nodo: si el envío es de la zona aporta 1, su peso y 1 si tiene la mayor prioridad del catálogo.
     * - Combinación: resumen del resto + aporte del nodo actual.
     *
     * Tiempo O(n); espacio O(n) por la pila de llamadas.
     */
    ZoneSummary summarizeZone(const PendingNode* pNode, const std::string& zone, const PriorityPolicy& policy);

    /**
     * @brief Envío pendiente más pesado de la zona, en forma recursiva.
     *
     * Caso base nullptr → nullptr; en cada nodo se compara el envío actual (si es de la zona) con
     * el más pesado del resto. Tiempo O(n); espacio O(n).
     */
    const Shipment* heaviestInZone(const PendingNode* pNode, const std::string& zone);

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_ZONE_REPORT_HPP
