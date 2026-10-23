#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_DISSOLVE_SHIPMENT_GROUP_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_DISSOLVE_SHIPMENT_GROUP_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/dissolveShipmentGroupUseCase.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de DissolveShipmentGroupUseCase.
class DissolveShipmentGroup final : public DissolveShipmentGroupUseCase
{
public:
    DissolveShipmentGroup(LogisticsModel&                   model,
                          shared::application::EventBus&    bus,
                          const shared::application::Clock& clock);

private:
    void execute(const shared::application::Principal& principal, const std::string& groupId) override;

    LogisticsModel&                   m_model;
    shared::application::EventBus&    m_bus;
    const shared::application::Clock& m_clock;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_DISSOLVE_SHIPMENT_GROUP_HPP
