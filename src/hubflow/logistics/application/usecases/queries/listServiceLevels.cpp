#include "hubflow/logistics/application/usecases/queries/listServiceLevels.hpp"

namespace hubflow::logistics::application
{

    ListServiceLevels::ListServiceLevels(const LogisticsModel& model)
        : m_model {model}
    {
    }

    const domain::ServiceLevelCatalog& ListServiceLevels::execute(const shared::application::Principal&) const
    {
        return m_model.services;
    }

} // namespace hubflow::logistics::application
