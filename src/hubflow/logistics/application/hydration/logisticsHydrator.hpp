#ifndef HUBFLOW_LOGISTICS_APPLICATION_HYDRATION_LOGISTICS_HYDRATOR_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_HYDRATION_LOGISTICS_HYDRATOR_HPP

#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/out/shipmentRepository.hpp"

namespace hubflow::logistics::application
{

/**
 * @brief Reconstruye el modelo en memoria desde el repositorio al iniciar.
 *
 * Restaura cada envío en el catálogo (lista + BST + B+), reinserta en pendientes los que
 * esperan despacho según prioridad y ticket, y avanza el dispensador de tickets. No publica
 * eventos: los hechos ya ocurrieron.
 */
class LogisticsHydrator final : private ShipmentRestorer
{
public:
    explicit LogisticsHydrator(LogisticsModel& model);

    /// @brief Carga todos los envíos y movimientos persistidos.
    void hydrate(const ShipmentRepository& repository);

private:
    void restoreShipment(const domain::ShipmentState& state) override;
    void restoreMovement(const std::string& code, const domain::Movement& movement) override;

    LogisticsModel& m_model;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_HYDRATION_LOGISTICS_HYDRATOR_HPP
