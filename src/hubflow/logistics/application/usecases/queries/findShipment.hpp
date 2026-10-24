#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_SHIPMENT_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_SHIPMENT_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/findShipmentUseCase.hpp"

namespace hubflow::logistics::application
{

    /// @brief Implementación de FindShipmentUseCase.
    class FindShipment final : public FindShipmentUseCase
    {
    public:
        explicit FindShipment(const LogisticsModel& model);

    private:
        domain::LookupResult execute(const shared::application::Principal& principal,
                                     const std::string& code,
                                     LookupIndex index) const override;

        const LogisticsModel& m_model;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_FIND_SHIPMENT_HPP
