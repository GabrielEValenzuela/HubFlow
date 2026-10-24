#include "hubflow/logistics/domain/valueobjects/shipmentId.hpp"

#include "hubflow/logistics/domain/valueobjects/uuidText.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{

    ShipmentId::ShipmentId(const std::string& value)
        : m_value {value}
    {
        if (!isValid(value))
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "shipment_id.invalid",
                                                  "La identidad interna no es un UUID válido"};
        }
    }

    bool ShipmentId::isValid(const std::string& value)
    {
        return UuidText::isValid(value);
    }

    const std::string& ShipmentId::value() const noexcept
    {
        return m_value;
    }

    bool ShipmentId::operator==(const ShipmentId& other) const noexcept
    {
        return m_value == other.m_value;
    }

} // namespace hubflow::logistics::domain
