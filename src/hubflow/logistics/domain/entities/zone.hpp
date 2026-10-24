#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_ZONE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_ZONE_HPP

#include "hubflow/logistics/domain/services/plantOccupancy.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <optional>
#include <string>

namespace hubflow::logistics::domain
{

    /// @brief Estado persistible de una zona de reparto.
    struct ZoneState
    {
        std::string code;
        std::string name;
        bool active {true};
    };

    /**
     * @brief Zona de reparto: un sector de la ciudad al que se destinan paquetes y se arman cargas.
     *
     * Las zonas son datos de la operación: se dan de alta, se renombran y se desactivan, nunca se borran. Una zona
     * desactivada deja de aceptar paquetes nuevos pero conserva su historia.
     */
    class Zone final : public shared::domain::AggregateRoot
    {
    public:
        /// @brief Alta de una zona activa. Ownership del resultado al llamador.
        static Zone*
        open(const std::string& code, const std::string& name, const shared::domain::ActorContext& context);

        /// @brief Reconstituye una zona persistida. Ownership al llamador.
        static Zone* restore(const ZoneState& state);

        /**
         * @brief Aplica los cambios pedidos (nombre y/o habilitación) como una sola operación: o se aplican todos o
         * ninguno. Si no cambia nada no registra eventos, por lo que repetir el pedido deja todo igual.
         * @throws business_error (Validation) si el nombre es inválido.
         * @throws business_error (Conflict) al desactivar si hay paquetes en planta para la zona, repartos en curso que
         * pasan por ella o puntos activos del mapa asignados a ella.
         */
        void revise(const std::optional<std::string>& name,
                    const std::optional<bool>& active,
                    const PlantOccupancy& occupancy,
                    const shared::domain::ActorContext& context);

        const std::string& code() const noexcept;
        const std::string& name() const noexcept;
        bool isActive() const noexcept;
        const ZoneState& state() const noexcept;

    private:
        explicit Zone(ZoneState state);

        static void ensureCanClose(const std::string& code, const PlantOccupancy& occupancy);

        ZoneState m_state;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_ZONE_HPP
