#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_SHIPMENT_GROUP_CONTROLLER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_SHIPMENT_GROUP_CONTROLLER_HPP

#include "hubflow/logistics/application/ports/in/dissolveShipmentGroupUseCase.hpp"
#include "hubflow/logistics/application/ports/in/formShipmentGroupUseCase.hpp"
#include "hubflow/logistics/application/ports/in/getShipmentGroupUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentGroupsUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::logistics::adapters
{

    /// @brief Casos de uso de los grupos de paquetes que viajan juntos.
    struct ShipmentGroupUseCases
    {
        application::FormShipmentGroupUseCase& form;
        application::DissolveShipmentGroupUseCase& dissolve;
        const application::ListShipmentGroupsUseCase& list;
        const application::GetShipmentGroupUseCase& get;
    };

    /**
     * @brief Paquetes que viajan juntos.
     *
     * - @c GET /shipment-groups: grupos existentes.
     * - @c POST /shipment-groups {codes}: une dos o más paquetes; acepta @c Idempotency-Key y responde 201 con
     *   @c Location.
     * - @c GET /shipment-groups/:id: un grupo.
     * - @c DELETE /shipment-groups/:id: deshace el grupo; repetirlo responde 404 sin efecto.
     */
    class ShipmentGroupController final : public platform::Controller
    {
    public:
        explicit ShipmentGroupController(ShipmentGroupUseCases useCases);

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        ShipmentGroupUseCases m_useCases;
    };

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_SHIPMENT_GROUP_CONTROLLER_HPP
