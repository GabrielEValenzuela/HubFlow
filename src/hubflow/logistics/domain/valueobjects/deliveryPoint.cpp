#include "hubflow/logistics/domain/valueobjects/deliveryPoint.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr std::size_t MAX_NODE_ID_LENGTH {16};
} // namespace

DeliveryPoint::DeliveryPoint(const std::string& nodeId)
    : m_nodeId {nodeId}
{
    if (nodeId.empty() || nodeId.size() > MAX_NODE_ID_LENGTH)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation, "delivery_point.invalid", "Punto de entrega inválido"};
    }
}

const std::string& DeliveryPoint::nodeId() const noexcept
{
    return m_nodeId;
}

} // namespace hubflow::logistics::domain
