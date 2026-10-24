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

bool canTransition(const ShipmentStatus currentStatus, const ShipmentStatus targetStatus) noexcept
{
    const bool fromRecibido {
        currentStatus == ShipmentStatus::Recibido &&
        (targetStatus == ShipmentStatus::Clasificado || targetStatus == ShipmentStatus::EnReparto)};
    const bool fromClasificado {currentStatus == ShipmentStatus::Clasificado &&
                                targetStatus == ShipmentStatus::EnReparto};
    const bool fromEnReparto {
        currentStatus == ShipmentStatus::EnReparto &&
        (targetStatus == ShipmentStatus::Entregado || targetStatus == ShipmentStatus::Reprogramado)};
    const bool fromReprogramado {
        currentStatus == ShipmentStatus::Reprogramado &&
        (targetStatus == ShipmentStatus::Clasificado || targetStatus == ShipmentStatus::EnReparto)};
    return fromRecibido || fromClasificado || fromEnReparto || fromReprogramado;
}

} // namespace hubflow::logistics::domain
