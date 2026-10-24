#include "hubflow/logistics/domain/valueobjects/catalogName.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
    namespace
    {
        constexpr std::size_t MIN_NAME_LENGTH {2};
        constexpr std::size_t MAX_NAME_LENGTH {60};
    } // namespace

    CatalogName::CatalogName(const std::string& value)
        : m_value {value}
    {
        if (value.size() < MIN_NAME_LENGTH || value.size() > MAX_NAME_LENGTH)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "catalog_name.invalid",
                                                  "El nombre debe tener entre 2 y 60 caracteres"};
        }
    }

    const std::string& CatalogName::value() const noexcept
    {
        return m_value;
    }

} // namespace hubflow::logistics::domain
