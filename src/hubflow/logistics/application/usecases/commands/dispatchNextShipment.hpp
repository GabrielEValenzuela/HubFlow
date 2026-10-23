#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_DISPATCH_NEXT_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_DISPATCH_NEXT_SHIPMENT_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/dispatchNextShipmentUseCase.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de DispatchNextShipmentUseCase.
class DispatchNextShipment final : public DispatchNextShipmentUseCase
{
public:
    DispatchNextShipment(LogisticsModel&                   model,
                         shared::application::EventBus&    bus,
                         const shared::application::Clock& clock);

private:
    const domain::Shipment& execute(const shared::application::Principal& principal, const std::string& note) override;

    LogisticsModel&                   m_model;
    shared::application::EventBus&    m_bus;
    const shared::application::Clock& m_clock;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_DISPATCH_NEXT_SHIPMENT_HPP
