#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_STATUS_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_STATUS_HPP

#include "datastructures/bitSet.hpp"
#include <cstdint>

#include <cstddef>

namespace hubflow::logistics::domain
{

    /// @brief Estado del envío. Su valor es la posición del bit en el StateHistory.
    enum class ShipmentStatus : std::uint8_t
    {
        Recibido = 0,
        Clasificado = 1,
        EnReparto = 2,
        Reprogramado = 3,
        Entregado = 4
    };

    inline constexpr std::size_t STATUS_COUNT {5};

    inline constexpr ShipmentStatus ALL_STATUSES[] {ShipmentStatus::Recibido,
                                                    ShipmentStatus::Clasificado,
                                                    ShipmentStatus::EnReparto,
                                                    ShipmentStatus::Reprogramado,
                                                    ShipmentStatus::Entregado};

    /// @brief Estados alcanzados históricamente por un envío: un bit por estado.
    using StateHistory = ds::BitSet<STATUS_COUNT>;

    /// @brief Posición del bit asociado a @p status.
    std::size_t bitOf(ShipmentStatus status) noexcept;

    /// @brief Un envío espera despacho (está en pendientes) en RECIBIDO, CLASIFICADO o REPROGRAMADO.
    bool isAwaitingDispatch(ShipmentStatus status) noexcept;

    /**
     * @brief Máquina de estados del envío.
     *
     * - RECIBIDO → CLASIFICADO | EN_REPARTO
     * - CLASIFICADO → EN_REPARTO
     * - EN_REPARTO → ENTREGADO | REPROGRAMADO
     * - REPROGRAMADO → CLASIFICADO | EN_REPARTO
     * - ENTREGADO es terminal.
     */
    bool canTransition(ShipmentStatus currentStatus, ShipmentStatus targetStatus) noexcept;

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_STATUS_HPP
