#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_OPERATIONS_CONTROLLER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_OPERATIONS_CONTROLLER_HPP

#include "hubflow/logistics/application/ports/in/findHeaviestShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/getOperationsMetricsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/summarizeZoneUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::logistics::adapters
{

    /// @brief Casos de uso de seguimiento de la operación.
    struct OperationsUseCases
    {
        const application::SummarizeZoneUseCase& summarizeZone;
        const application::FindHeaviestShipmentUseCase& findHeaviest;
        const application::GetOperationsMetricsUseCase& metrics;
    };

    /**
     * @brief Seguimiento de la operación (todas son consultas).
     *
     * - @c GET /zones/:zone/summary: cantidad, peso y envíos express pendientes de una zona.
     * - @c GET /zones/:zone/heaviest-shipment: el envío pendiente más pesado de la zona.
     * - @c GET /metrics: totales por estado y por zona.
     */
    class OperationsController final : public platform::Controller
    {
    public:
        explicit OperationsController(const OperationsUseCases& useCases);

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        OperationsUseCases m_useCases;
    };

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_OPERATIONS_CONTROLLER_HPP
