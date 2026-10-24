#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_DELIVERY_PROGRESS_SOURCE_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_DELIVERY_PROGRESS_SOURCE_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace hubflow::tracking::application
{

    /// @brief Avance del reparto que lleva un paquete: cuánto falta para que llegue y cuántas paradas hay antes.
    struct DeliveryProgress
    {
        std::uint32_t minutesToArrival {0};
        std::size_t stopsBefore {0};
    };

    /**
     * @brief Consulta al contexto de ruteo cómo va el reparto de un paquete (Anti-Corruption Layer).
     *
     * El cálculo usa la misma estimación de tiempos que ve la planta al armar la ruta.
     */
    class DeliveryProgressSource
    {
    public:
        virtual ~DeliveryProgressSource() = default;

        /// @brief El avance, o nada si el paquete no está en una hoja de ruta con su parada pendiente.
        virtual std::optional<DeliveryProgress> progressOf(const std::string& code) const = 0;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_DELIVERY_PROGRESS_SOURCE_HPP
