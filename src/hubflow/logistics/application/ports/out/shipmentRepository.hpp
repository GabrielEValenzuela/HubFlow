#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_SHIPMENT_REPOSITORY_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_SHIPMENT_REPOSITORY_HPP

#include "hubflow/logistics/domain/entities/shipment.hpp"

namespace hubflow::logistics::application
{

    /// @brief Receptor de envíos durante la hidratación desde el almacenamiento.
    class ShipmentRestorer
    {
    public:
        virtual ~ShipmentRestorer() = default;

        /// @brief Un envío persistido.
        virtual void restoreShipment(const domain::ShipmentState& state) = 0;

        /// @brief Un movimiento persistido del envío @p code, en orden cronológico.
        virtual void restoreMovement(const std::string& code, const domain::Movement& movement) = 0;
    };

    /**
     * @brief Repositorio de persistencia de envíos (la implementación vive en infraestructura).
     *
     * El modelo en memoria (catálogo, pendientes, índices) es la fuente de las consultas;
     * el repositorio sólo garantiza durabilidad.
     */
    class ShipmentRepository
    {
    public:
        virtual ~ShipmentRepository() = default;

        /// @brief Persiste un envío nuevo.
        virtual void add(const domain::ShipmentState& state) = 0;

        /// @brief Actualiza estado, intentos, bits y ticket.
        virtual void update(const domain::ShipmentState& state) = 0;

        /// @brief Agrega un movimiento al historial persistido.
        virtual void appendMovement(const std::string& code, const domain::Movement& movement) = 0;

        /// @brief Entrega todos los envíos y luego todos los movimientos al @p restorer.
        virtual void loadAll(ShipmentRestorer& restorer) const = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_SHIPMENT_REPOSITORY_HPP
