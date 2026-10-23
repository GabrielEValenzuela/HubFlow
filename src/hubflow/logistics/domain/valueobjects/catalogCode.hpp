#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_CATALOG_CODE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_CATALOG_CODE_HPP

#include <string>

namespace hubflow::logistics::domain
{

/**
 * @brief Código corto con el que se identifica un dato maestro (zona, nivel de servicio o característica).
 *
 * Tiene entre 2 y 16 caracteres, sólo mayúsculas, dígitos y guion bajo. Es la identidad estable del dato: se
 * puede renombrar o desactivar, pero el código no cambia.
 */
class CatalogCode final
{
public:
    /// @throws business_error (Validation) si el código no cumple el formato.
    explicit CatalogCode(const std::string& value);

    const std::string& value() const noexcept;

    static bool isValid(const std::string& value) noexcept;

private:
    std::string m_value;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_CATALOG_CODE_HPP
