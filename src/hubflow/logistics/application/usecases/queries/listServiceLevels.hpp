#ifndef HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SERVICE_LEVELS_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SERVICE_LEVELS_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/listServiceLevelsUseCase.hpp"

namespace hubflow::logistics::application
{

    /// @brief Implementación de ListServiceLevelsUseCase.
    class ListServiceLevels final : public ListServiceLevelsUseCase
    {
    public:
        explicit ListServiceLevels(const LogisticsModel& model);

    private:
        const domain::ServiceLevelCatalog& execute(const shared::application::Principal& principal) const override;

        const LogisticsModel& m_model;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_USECASES_QUERIES_LIST_SERVICE_LEVELS_HPP
