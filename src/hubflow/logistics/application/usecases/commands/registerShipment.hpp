#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_REGISTER_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_REGISTER_SHIPMENT_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/registerShipmentUseCase.hpp"
#include "hubflow/logistics/domain/services/deliveryPointCatalog.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de RegisterShipmentUseCase.
class RegisterShipment final : public RegisterShipmentUseCase
{
public:
    RegisterShipment(LogisticsModel&                     model,
                     const domain::DeliveryPointCatalog& deliveryPoints,
                     shared::application::EventBus&      bus,
                     const shared::application::Clock&   clock);

private:
    const domain::Shipment& execute(const shared::application::Principal& principal,
                                    const domain::ShipmentDraft&          draft) override;

    LogisticsModel&                     m_model;
    const domain::DeliveryPointCatalog& m_deliveryPoints;
    shared::application::EventBus&      m_bus;
    const shared::application::Clock&   m_clock;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_REGISTER_SHIPMENT_HPP
