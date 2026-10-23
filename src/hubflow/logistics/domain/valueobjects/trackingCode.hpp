#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_TRACKING_CODE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_TRACKING_CODE_HPP

#include <string>

namespace hubflow::logistics::domain
{

/**
 * @brief Identidad de un envío con formato @c PKG-NNNN.
 *
 * El largo fijo hace que el orden lexicográfico coincida con el numérico, de modo que el código
 * puede usarse directamente como clave del BST, del B+ y de la búsqueda binaria.
 */
class TrackingCode final
{
public:
    /// @throws business_error (Validation) si el formato es inválido.
    explicit TrackingCode(const std::string& value);

    /// @brief Indica si @p value respeta el formato sin construir el objeto.
    static bool isValid(const std::string& value);

    const std::string& value() const noexcept;

    bool operator<(const TrackingCode& other) const noexcept;

    bool operator==(const TrackingCode& other) const noexcept;

private:
    std::string m_value;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_TRACKING_CODE_HPP
