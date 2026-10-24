#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_UPDATE_ZONE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_UPDATE_ZONE_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/updateZoneUseCase.hpp"
#include "hubflow/logistics/domain/services/plantOccupancy.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::logistics::application
{

    /// @brief Implementación de UpdateZoneUseCase.
    class UpdateZone final : public UpdateZoneUseCase
    {
    public:
        UpdateZone(LogisticsModel& model,
                   const domain::PlantOccupancy& occupancy,
                   shared::application::EventBus& bus,
                   const shared::application::Clock& clock);

    private:
        const domain::Zone& execute(const shared::application::Principal& principal,
                                    const std::string& code,
                                    const UpdateZoneRequest& request) override;

        LogisticsModel& m_model;
        const domain::PlantOccupancy& m_occupancy;
        shared::application::EventBus& m_bus;
        const shared::application::Clock& m_clock;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_UPDATE_ZONE_HPP
