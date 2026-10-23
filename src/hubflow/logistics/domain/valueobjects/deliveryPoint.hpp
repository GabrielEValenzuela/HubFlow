#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_DELIVERY_POINT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_DELIVERY_POINT_HPP

#include <string>

namespace hubflow::logistics::domain
{

/// @brief Identificador del nodo del mapa donde se entrega el envío (referencia por identidad).
class DeliveryPoint final
{
public:
    /// @throws business_error (Validation) si está vacío o supera 16 caracteres.
    explicit DeliveryPoint(const std::string& nodeId);

    const std::string& nodeId() const noexcept;

private:
    std::string m_nodeId;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_DELIVERY_POINT_HPP
