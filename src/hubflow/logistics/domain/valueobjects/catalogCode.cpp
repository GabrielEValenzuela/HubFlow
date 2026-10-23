#include "hubflow/logistics/domain/valueobjects/catalogCode.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr std::size_t MIN_CODE_LENGTH {2};
constexpr std::size_t MAX_CODE_LENGTH {16};

bool isAllowed(const char symbol) noexcept
{
    return (symbol >= 'A' && symbol <= 'Z') || (symbol >= '0' && symbol <= '9') || symbol == '_';
}
} // namespace

CatalogCode::CatalogCode(const std::string& value)
    : m_value {value}
{
    if (!isValid(value))
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation,
            "catalog_code.invalid",
            "El código debe tener entre 2 y 16 caracteres: mayúsculas, números o guion bajo"};
    }
}

const std::string& CatalogCode::value() const noexcept
{
    return m_value;
}

bool CatalogCode::isValid(const std::string& value) noexcept
{
    bool valid {value.size() >= MIN_CODE_LENGTH && value.size() <= MAX_CODE_LENGTH};
    for (std::size_t i = 0; i < value.size() && valid; ++i)
    {
        valid = isAllowed(value[i]);
    }
    return valid;
}

} // namespace hubflow::logistics::domain
