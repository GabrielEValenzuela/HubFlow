#include "hubflow/logistics/domain/valueobjects/shipmentStatus.hpp"

namespace hubflow::logistics::domain
{

std::size_t bitOf(const ShipmentStatus status) noexcept
{
    return static_cast<std::size_t>(status);
}

bool isAwaitingDispatch(const ShipmentStatus status) noexcept
{
    return status == ShipmentStatus::Recibido || status == ShipmentStatus::Clasificado ||
           status == ShipmentStatus::Reprogramado;
}

bool canTransition(const ShipmentStatus from, const ShipmentStatus to) noexcept
{
    const bool fromRecibido {from == ShipmentStatus::Recibido &&
                             (to == ShipmentStatus::Clasificado || to == ShipmentStatus::EnReparto)};
    const bool fromClasificado {from == ShipmentStatus::Clasificado && to == ShipmentStatus::EnReparto};
    const bool fromEnReparto {from == ShipmentStatus::EnReparto &&
                              (to == ShipmentStatus::Entregado || to == ShipmentStatus::Reprogramado)};
    const bool fromReprogramado {from == ShipmentStatus::Reprogramado &&
                                 (to == ShipmentStatus::Clasificado || to == ShipmentStatus::EnReparto)};
    return fromRecibido || fromClasificado || fromEnReparto || fromReprogramado;
}

} // namespace hubflow::logistics::domain
