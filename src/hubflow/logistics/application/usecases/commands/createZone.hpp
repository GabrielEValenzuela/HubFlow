#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_CREATE_ZONE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_CREATE_ZONE_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/createZoneUseCase.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de CreateZoneUseCase.
class CreateZone final : public CreateZoneUseCase
{
public:
    CreateZone(LogisticsModel& model, shared::application::EventBus& bus, const shared::application::Clock& clock);

private:
    const domain::Zone& execute(const shared::application::Principal& principal,
                                const CreateZoneRequest&              request) override;

    LogisticsModel&                   m_model;
    shared::application::EventBus&    m_bus;
    const shared::application::Clock& m_clock;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_CREATE_ZONE_HPP
