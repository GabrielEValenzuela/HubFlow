#ifndef HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_CONFIRM_STOP_PARCEL_HPP
#define HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_CONFIRM_STOP_PARCEL_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/routing/application/ports/in/confirmStopParcelUseCase.hpp"
#include "hubflow/routing/application/routingModel.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::routing::application
{

/// @brief Implementación de ConfirmStopParcelUseCase.
class ConfirmStopParcel final : public ConfirmStopParcelUseCase
{
public:
    ConfirmStopParcel(logistics::application::LogisticsModel& logistics,
                      RoutingModel&                           routing,
                      shared::application::EventBus&          bus,
                      const shared::application::Clock&       clock);

private:
    const logistics::domain::Shipment& execute(const shared::application::Principal& principal,
                                               const ConfirmParcelRequest&           request) override;

    logistics::application::LogisticsModel& m_logistics;
    RoutingModel&                           m_routing;
    shared::application::EventBus&          m_bus;
    const shared::application::Clock&       m_clock;
};

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_USECASES_COMMANDS_CONFIRM_STOP_PARCEL_HPP
