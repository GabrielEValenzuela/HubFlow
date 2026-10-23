#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_UUID_TEXT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_UUID_TEXT_HPP

#include <string>

namespace hubflow::logistics::domain
{

/**
 * @brief Formato textual de los identificadores internos: UUID versión 7 en minúsculas, por ejemplo
 * @c 018f3c52-7a1e-7c3b-9a41-5d2f8e0b6c11.
 *
 * Los primeros 48 bits son la hora de creación en milisegundos, de modo que ordenar los identificadores como texto
 * equivale a ordenarlos por momento de alta.
 */
class UuidText final
{
public:
    static constexpr std::size_t LENGTH {36};

    /// @brief Indica si @p value tiene el formato de un UUID versión 7.
    static bool isValid(const std::string& value);
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_UUID_TEXT_HPP
