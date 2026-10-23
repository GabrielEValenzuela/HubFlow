#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_CATALOG_NAME_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_CATALOG_NAME_HPP

#include <string>

namespace hubflow::logistics::domain
{

/// @brief Nombre legible de un dato maestro, tal como lo ve el personal de la planta (2 a 60 caracteres).
class CatalogName final
{
public:
    /// @throws business_error (Validation) si el nombre es demasiado corto o largo.
    explicit CatalogName(const std::string& value);

    const std::string& value() const noexcept;

private:
    std::string m_value;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_CATALOG_NAME_HPP
