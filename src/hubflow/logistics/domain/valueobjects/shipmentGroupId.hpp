#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_GROUP_ID_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_GROUP_ID_HPP

#include <string>

namespace hubflow::logistics::domain
{

    /// @brief Identidad interna e inmutable de el grupo de envíos: un UUID versión 7 que nunca se muestra al cliente.
    class ShipmentGroupId final
    {
    public:
        /// @throws BusinessError (Validation) si @p value no es un UUID versión 7.
        explicit ShipmentGroupId(const std::string& value);

        static bool isValid(const std::string& value);

        const std::string& value() const noexcept;

        bool operator==(const ShipmentGroupId& other) const noexcept;

    private:
        std::string m_value;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_SHIPMENT_GROUP_ID_HPP
