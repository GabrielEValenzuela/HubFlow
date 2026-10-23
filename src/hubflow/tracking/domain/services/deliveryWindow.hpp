#ifndef HUBFLOW_TRACKING_DOMAIN_SERVICES_DELIVERY_WINDOW_HPP
#define HUBFLOW_TRACKING_DOMAIN_SERVICES_DELIVERY_WINDOW_HPP

#include <cstdint>

namespace hubflow::tracking::domain
{

/// @brief Franja horaria (segundos desde epoch) en la que se espera la entrega.
struct DeliveryWindow
{
    std::int64_t from {0};
    std::int64_t to {0};
};

/**
 * @brief Convierte "faltan unos minutos para llegar" en una franja de una hora que el cliente puede esperar.
 *
 * La franja empieza en el cuarto de hora anterior a la llegada estimada (sin salir nunca antes de ahora) y dura una
 * hora, así la llegada estimada siempre queda adentro y no se promete un minuto exacto que nadie puede cumplir.
 */
class DeliveryWindowPolicy final
{
public:
    static constexpr std::int64_t ROUNDING_SECONDS {15 * 60};
    static constexpr std::int64_t LENGTH_SECONDS {60 * 60};

    static DeliveryWindow around(std::int64_t now, std::uint32_t minutesToArrival) noexcept;
};

} // namespace hubflow::tracking::domain

#endif // HUBFLOW_TRACKING_DOMAIN_SERVICES_DELIVERY_WINDOW_HPP
