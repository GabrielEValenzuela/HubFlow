#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_CATALOG_JSON_PRESENTER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_CATALOG_JSON_PRESENTER_HPP

#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/platform/http/httpTypes.hpp"

namespace hubflow::logistics::adapters
{

    /// @brief Representación externa de las zonas, los niveles de servicio y las características de envío.
    class CatalogJsonPresenter final
    {
    public:
        static platform::Json zone(const domain::Zone& zone);
        static platform::Json zones(const domain::ZoneCatalog& zones);

        static platform::Json serviceLevel(const domain::ServiceLevel& service);
        static platform::Json serviceLevels(const domain::ServiceLevelCatalog& services);

        static platform::Json feature(const domain::ShipmentFeature& feature);
        static platform::Json features(const domain::FeatureCatalog& features);
    };

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_CATALOG_JSON_PRESENTER_HPP
