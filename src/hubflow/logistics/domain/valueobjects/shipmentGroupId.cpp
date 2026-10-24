#include "hubflow/logistics/domain/valueobjects/shipmentGroupId.hpp"

#include "hubflow/logistics/domain/valueobjects/uuidText.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{

    ShipmentGroupId::ShipmentGroupId(const std::string& value)
        : m_value {value}
    {
        if (!isValid(value))
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::Validation,
                                                 "shipment_group_id.invalid",
                                                 "La identidad interna no es un UUID válido"};
        }
    }

    bool ShipmentGroupId::isValid(const std::string& value)
    {
        return UuidText::isValid(value);
    }

    const std::string& ShipmentGroupId::value() const noexcept
    {
        return m_value;
    }

    bool ShipmentGroupId::operator==(const ShipmentGroupId& other) const noexcept
    {
        return m_value == other.m_value;
    }

} // namespace hubflow::logistics::domain
