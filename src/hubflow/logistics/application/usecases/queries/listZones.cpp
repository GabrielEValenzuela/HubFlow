#include "hubflow/logistics/application/usecases/queries/listZones.hpp"

namespace hubflow::logistics::application
{

    ListZones::ListZones(const LogisticsModel& model)
        : m_model {model}
    {
    }

    const domain::ZoneCatalog& ListZones::execute(const shared::application::Principal&) const
    {
        return m_model.zones;
    }

} // namespace hubflow::logistics::application
