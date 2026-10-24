#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_TRAVEL_TIME_POLICY_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_TRAVEL_TIME_POLICY_HPP

#include <cstddef>
#include <cstdint>

namespace hubflow::routing::domain
{

    /**
     * @brief Supuestos con los que se estima cuánto tarda un reparto: velocidad urbana promedio y tiempo de entrega por
     * parada.
     *
     * Es una estimación, no una medición. La usan tanto el comparador de rutas que ve la planta como el aviso de
     * horario estimado que ve el cliente, para que ambos hablen del mismo tiempo.
     */
    class TravelTimePolicy final
    {
    public:
        static constexpr double AVERAGE_KMH {25.0};
        static constexpr std::uint32_t MINUTES_PER_STOP {4};

        /// @brief Minutos para recorrer @p km y atender @p stops paradas, redondeados al minuto más cercano.
        static std::uint32_t estimateMinutes(double km, std::size_t stops) noexcept;

        /// @brief Explicación de los supuestos, en lenguaje llano.
        static const char* assumptions() noexcept;
    };

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_TRAVEL_TIME_POLICY_HPP
