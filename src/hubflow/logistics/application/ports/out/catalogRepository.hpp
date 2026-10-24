#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_CATALOG_REPOSITORY_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_CATALOG_REPOSITORY_HPP

#include "hubflow/logistics/domain/entities/serviceLevel.hpp"
#include "hubflow/logistics/domain/entities/shipmentFeature.hpp"
#include "hubflow/logistics/domain/entities/zone.hpp"

namespace hubflow::logistics::application
{

    /// @brief Receptor de los datos maestros durante la hidratación.
    class CatalogRestorer
    {
    public:
        virtual ~CatalogRestorer() = default;

        virtual void restoreZone(const domain::ZoneState& state) = 0;

        virtual void restoreServiceLevel(const domain::ServiceLevelState& state) = 0;

        virtual void restoreFeature(const domain::ShipmentFeatureState& state) = 0;
    };

    /**
     * @brief Persistencia de zonas, niveles de servicio y características.
     *
     * Cada @c save* inserta el dato si es nuevo o lo actualiza si ya existe (por su código).
     */
    class CatalogRepository
    {
    public:
        virtual ~CatalogRepository() = default;

        virtual void saveZone(const domain::ZoneState& state) = 0;

        virtual void saveServiceLevel(const domain::ServiceLevelState& state) = 0;

        virtual void saveFeature(const domain::ShipmentFeatureState& state) = 0;

        /// @brief Entrega todos los datos maestros al @p restorer.
        virtual void loadAll(CatalogRestorer& restorer) const = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_CATALOG_REPOSITORY_HPP
