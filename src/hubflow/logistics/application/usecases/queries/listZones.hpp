#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_ZONES_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_ZONES_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/listZonesUseCase.hpp"

namespace hubflow::logistics::application
{

    /// @brief Implementación de ListZonesUseCase.
    class ListZones final : public ListZonesUseCase
    {
    public:
        explicit ListZones(const LogisticsModel& model);

    private:
        const domain::ZoneCatalog& execute(const shared::application::Principal& principal) const override;

        const LogisticsModel& m_model;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_ZONES_HPP
