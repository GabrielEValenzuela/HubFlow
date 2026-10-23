#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENT_FEATURES_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENT_FEATURES_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentFeaturesUseCase.hpp"

namespace hubflow::logistics::application
{

/// @brief Implementación de ListShipmentFeaturesUseCase.
class ListShipmentFeatures final : public ListShipmentFeaturesUseCase
{
public:
    explicit ListShipmentFeatures(const LogisticsModel& model);

private:
    const domain::FeatureCatalog& execute(const shared::application::Principal& principal) const override;

    const LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SHIPMENT_FEATURES_HPP
