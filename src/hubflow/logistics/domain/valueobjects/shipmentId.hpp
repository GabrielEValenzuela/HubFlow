#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_ID_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_ID_HPP

#include <string>

namespace hubflow::logistics::domain
{

/// @brief Identidad interna e inmutable de el envío: un UUID versión 7 que nunca se muestra al cliente.
class ShipmentId final
{
public:
    /// @throws business_error (Validation) si @p value no es un UUID versión 7.
    explicit ShipmentId(const std::string& value);

    static bool isValid(const std::string& value);

    const std::string& value() const noexcept;

    bool operator==(const ShipmentId& other) const noexcept;

private:
    std::string m_value;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_ID_HPP
