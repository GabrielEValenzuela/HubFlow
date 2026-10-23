#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_MOVEMENT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_MOVEMENT_HPP

#include "hubflow/logistics/domain/valueobjects/shipmentStatus.hpp"

#include <cstdint>
#include <string>

namespace hubflow::logistics::domain
{

/// @brief Movimiento del historial: número secuencial, estado, observación, autor y fecha.
struct Movement
{
    std::size_t    sequence {0};
    ShipmentStatus status {ShipmentStatus::Recibido};
    std::string    note;
    std::string    actorName;
    std::int64_t   occurredAt {0};
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_MOVEMENT_HPP
