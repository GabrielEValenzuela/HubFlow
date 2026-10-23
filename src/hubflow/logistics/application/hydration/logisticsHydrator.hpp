#ifndef HUBFLOW_LOGISTICS_APPLICATION_HYDRATION_LOGISTICS_HYDRATOR_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_HYDRATION_LOGISTICS_HYDRATOR_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/out/catalogRepository.hpp"
#include "hubflow/logistics/application/ports/out/shipmentRepository.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Reconstruye el modelo en memoria desde los repositorios al iniciar.
 *
 * Primero restaura los datos maestros (zonas, niveles de servicio y características), después cada envío en el
 * catálogo (lista + BST + B+), reinserta en pendientes los que esperan despacho según la política de prioridad y el
 * ticket, y avanza el dispensador de tickets. No publica eventos: los hechos ya ocurrieron.
 */
class LogisticsHydrator final : private ShipmentRestorer, private CatalogRestorer
{
public:
    explicit LogisticsHydrator(LogisticsModel& model);

    /// @brief Carga los datos maestros, los envíos y los movimientos persistidos.
    void hydrate(const CatalogRepository& catalogs, const ShipmentRepository& shipments);

private:
    void restoreShipment(const domain::ShipmentState& state) override;
    void restoreMovement(const std::string& code, const domain::Movement& movement) override;
    void restoreZone(const domain::ZoneState& state) override;
    void restoreServiceLevel(const domain::ServiceLevelState& state) override;
    void restoreFeature(const domain::ShipmentFeatureState& state) override;

    LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_HYDRATION_LOGISTICS_HYDRATOR_HPP
