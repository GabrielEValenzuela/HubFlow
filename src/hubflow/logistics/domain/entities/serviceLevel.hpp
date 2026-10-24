#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SERVICE_LEVEL_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SERVICE_LEVEL_HPP

#include "hubflow/logistics/domain/services/plantOccupancy.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <optional>
#include <string>

namespace hubflow::logistics::domain
{

    /// @brief Estado persistible de un nivel de servicio.
    struct ServiceLevelState
    {
        std::string code;
        std::string name;
        unsigned priority {1};
        bool active {true};
    };

    /**
     * @brief Nivel de servicio contratado por el cliente, con su orden de prioridad de despacho.
     *
     * Un número de prioridad mayor se despacha antes. La prioridad se fija al darlo de alta y no cambia, para que el
     * orden de la lista de pendientes nunca se altere por debajo. Agregar un servicio nuevo no requiere tocar la lista.
     */
    class ServiceLevel final : public shared::domain::AggregateRoot
    {
    public:
        /**
         * @brief Alta de un nivel de servicio activo. Ownership del resultado al llamador.
         * @throws business_error (Validation) si el código, el nombre o la prioridad (1 a 1000) son inválidos.
         */
        static ServiceLevel* open(const std::string& code,
                                  const std::string& name,
                                  unsigned priority,
                                  const shared::domain::ActorContext& context);

        /// @brief Reconstituye un nivel persistido. Ownership al llamador.
        static ServiceLevel* restore(const ServiceLevelState& state);

        /**
         * @brief Aplica los cambios pedidos (nombre y/o habilitación) como una sola operación: o se aplican todos o
         * ninguno. Si no cambia nada no registra eventos, por lo que repetir el pedido deja todo igual.
         * @throws business_error (Validation) si el nombre es inválido.
         * @throws business_error (Conflict) al desactivar si hay paquetes en la planta con este servicio.
         */
        void revise(const std::optional<std::string>& name,
                    const std::optional<bool>& active,
                    const PlantOccupancy& occupancy,
                    const shared::domain::ActorContext& context);

        const std::string& code() const noexcept;
        const std::string& name() const noexcept;
        unsigned priority() const noexcept;
        bool isActive() const noexcept;
        const ServiceLevelState& state() const noexcept;

    private:
        explicit ServiceLevel(ServiceLevelState state);

        ServiceLevelState m_state;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SERVICE_LEVEL_HPP
