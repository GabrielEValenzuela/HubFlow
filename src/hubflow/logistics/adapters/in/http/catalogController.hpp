#ifndef HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_CATALOG_CONTROLLER_HPP
#define HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_CATALOG_CONTROLLER_HPP

#include "hubflow/logistics/application/ports/in/createServiceLevelUseCase.hpp"
#include "hubflow/logistics/application/ports/in/createShipmentFeatureUseCase.hpp"
#include "hubflow/logistics/application/ports/in/createZoneUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listServiceLevelsUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listShipmentFeaturesUseCase.hpp"
#include "hubflow/logistics/application/ports/in/listZonesUseCase.hpp"
#include "hubflow/logistics/application/ports/in/updateServiceLevelUseCase.hpp"
#include "hubflow/logistics/application/ports/in/updateShipmentFeatureUseCase.hpp"
#include "hubflow/logistics/application/ports/in/updateZoneUseCase.hpp"
#include "hubflow/platform/http/controller.hpp"

namespace hubflow::logistics::adapters
{

/// @brief Casos de uso de configuración que expone el controlador.
struct CatalogUseCases
{
    application::CreateZoneUseCase&                 createZone;
    application::UpdateZoneUseCase&                 updateZone;
    const application::ListZonesUseCase&            listZones;
    application::CreateServiceLevelUseCase&         createService;
    application::UpdateServiceLevelUseCase&         updateService;
    const application::ListServiceLevelsUseCase&    listServices;
    application::CreateShipmentFeatureUseCase&      createFeature;
    application::UpdateShipmentFeatureUseCase&      updateFeature;
    const application::ListShipmentFeaturesUseCase& listFeatures;
};

/**
 * @brief Configuración de la operación: zonas, niveles de servicio y características de envío.
 *
 * - @c GET /zones, @c GET /service-levels, @c GET /shipment-features: todos los datos, activos y desactivados.
 * - @c POST /zones {code, name}, @c POST /service-levels {code, name, priority}, @c POST /shipment-features
 *   {code, name}: alta (acepta @c Idempotency-Key; repetir el código responde 409). Responde 201 con @c Location.
 * - @c PATCH /zones/:code, @c PATCH /service-levels/:code, @c PATCH /shipment-features/:code {name?, active?}:
 *   renombra o habilita o desactiva. No existe el borrado físico. Repetir el mismo cambio responde 200 sin efectos.
 */
class CatalogController final : public platform::Controller
{
public:
    explicit CatalogController(CatalogUseCases useCases);

private:
    void registerRoutes(platform::ApiRouter& router) override;

    CatalogUseCases m_useCases;
};

} // namespace hubflow::logistics::adapters

#endif // HUBFLOW_LOGISTICS_ADAPTERS_IN_HTTP_CATALOG_CONTROLLER_HPP
