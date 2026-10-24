#include "hubflow/logistics/adapters/in/http/catalogJsonPresenter.hpp"

namespace hubflow::logistics::adapters
{
namespace
{
using platform::Json;

template <typename TCatalog, typename TPresenter>
Json listOf(const TCatalog& catalog, TPresenter present)
{
    Json items = Json::array();
    catalog.forEach(
        [&items, &present](const auto& entry)
        {
            items.push_back(present(entry));
        });
    return Json {{"items", items}};
}
} // namespace

Json CatalogJsonPresenter::zone(const domain::Zone& zone)
{
    return Json {{"code", zone.code()}, {"name", zone.name()}, {"active", zone.isActive()}};
}

Json CatalogJsonPresenter::zones(const domain::ZoneCatalog& zones)
{
    return listOf(zones, &CatalogJsonPresenter::zone);
}

Json CatalogJsonPresenter::serviceLevel(const domain::ServiceLevel& service)
{
    return Json {{"code", service.code()},
                 {"name", service.name()},
                 {"priority", service.priority()},
                 {"active", service.isActive()}};
}

Json CatalogJsonPresenter::serviceLevels(const domain::ServiceLevelCatalog& services)
{
    return listOf(services, &CatalogJsonPresenter::serviceLevel);
}

Json CatalogJsonPresenter::feature(const domain::ShipmentFeature& feature)
{
    return Json {{"code", feature.code()}, {"name", feature.name()}, {"active", feature.isActive()}};
}

Json CatalogJsonPresenter::features(const domain::FeatureCatalog& features)
{
    return listOf(features, &CatalogJsonPresenter::feature);
}

} // namespace hubflow::logistics::adapters
