#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_FORM_SHIPMENT_GROUP_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_FORM_SHIPMENT_GROUP_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/formShipmentGroupUseCase.hpp"
#include "hubflow/shared/application/eventBus.hpp"
#include "hubflow/shared/application/ports/out/clock.hpp"
#include "hubflow/shared/application/ports/out/identityGenerator.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de FormShipmentGroupUseCase.
class FormShipmentGroup final : public FormShipmentGroupUseCase
{
public:
    FormShipmentGroup(LogisticsModel&                         model,
                      shared::application::IdentityGenerator& identities,
                      shared::application::EventBus&          bus,
                      const shared::application::Clock&       clock);

private:
    const domain::ShipmentGroup& execute(const shared::application::Principal& principal,
                                         const ds::DynamicArray<std::string>&  codes) override;

    LogisticsModel&                         m_model;
    shared::application::IdentityGenerator& m_identities;
    shared::application::EventBus&          m_bus;
    const shared::application::Clock&       m_clock;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_COMMANDS_FORM_SHIPMENT_GROUP_HPP
