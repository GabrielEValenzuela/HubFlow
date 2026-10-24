#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_UPDATE_SHIPMENT_FEATURE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_UPDATE_SHIPMENT_FEATURE_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/updateShipmentFeatureUseCase.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::logistics::application
{

    /// @brief Implementación de UpdateShipmentFeatureUseCase.
    class UpdateShipmentFeature final : public UpdateShipmentFeatureUseCase
    {
    public:
        UpdateShipmentFeature(LogisticsModel& model,
                              shared::application::EventBus& bus,
                              const shared::application::Clock& clock);

    private:
        const domain::ShipmentFeature& execute(const shared::application::Principal& principal,
                                               const std::string& code,
                                               const UpdateShipmentFeatureRequest& request) override;

        LogisticsModel& m_model;
        shared::application::EventBus& m_bus;
        const shared::application::Clock& m_clock;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_UPDATE_SHIPMENT_FEATURE_HPP
