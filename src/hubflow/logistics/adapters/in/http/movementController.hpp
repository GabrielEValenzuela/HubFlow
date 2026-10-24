#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_MOVEMENT_CONTROLLER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_MOVEMENT_CONTROLLER_HPP

#include "hubflow/logistics/application/ports/in/classifyShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/deliverShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/getShipmentUseCase.hpp"
#include "hubflow/logistics/application/ports/in/rescheduleShipmentUseCase.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentStatus.hpp"
#include "hubflow/platform/http/controller.hpp"

#include <functional>
#include <map>

namespace hubflow::logistics::adapters
{

    /// @brief Casos de uso que registran movimientos de un envío.
    struct MovementUseCases
    {
        application::ClassifyShipmentUseCase& classify;
        application::RescheduleShipmentUseCase& reschedule;
        application::DeliverShipmentUseCase& deliver;
        const application::GetShipmentUseCase& getShipment;
    };

    /**
     * @brief Historial de movimientos de un envío.
     *
     * - @c GET /shipments/:code/movements?order=asc|desc: movimientos en orden cronológico o inverso.
     * - @c POST /shipments/:code/movements {status, note, confirmAllParcels?}: registra clasificación, reprogramación o
     *   entrega. Para entregar hay que haber confirmado todos los bultos o pedir @c confirmAllParcels. Cada pedido
     * agrega un movimiento nuevo, por eso acepta @c Idempotency-Key. El despacho se hace con @c POST /dispatches.
     */
    class MovementController final : public platform::Controller
    {
    public:
        explicit MovementController(const MovementUseCases& useCases);

    private:
        using Transition = std::function<const domain::Shipment&(
            const shared::application::Principal&, const std::string&, const std::string&, bool)>;

        void registerRoutes(platform::ApiRouter& router) override;

        platform::HttpResponse record(const platform::RequestContext& context) const;

        MovementUseCases m_useCases;
        std::map<domain::ShipmentStatus, Transition> m_transitions;
    };

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_MOVEMENT_CONTROLLER_HPP
