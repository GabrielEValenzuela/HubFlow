#include "hubflow/logistics/domain/valueobjects/postalCode.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr std::size_t NUMERIC_LENGTH {4};
constexpr std::size_t CPA_LENGTH {8};

bool isDigit(const char character)
{
    return character >= '0' && character <= '9';
}

bool isLetter(const char character)
{
    return character >= 'A' && character <= 'Z';
}

bool isNumeric(const std::string& text)
{
    bool valid {text.size() == NUMERIC_LENGTH};
    for (std::size_t i = 0; i < text.size() && valid; ++i)
    {
        valid = isDigit(text[i]);
    }
    return valid;
}

bool isCpa(const std::string& text)
{
    bool valid {text.size() == CPA_LENGTH && isLetter(text[0])};
    for (std::size_t i = 1; i <= NUMERIC_LENGTH && valid; ++i)
    {
        valid = isDigit(text[i]);
    }
    for (std::size_t i = NUMERIC_LENGTH + 1; i < CPA_LENGTH && valid; ++i)
    {
        valid = isLetter(text[i]);
    }
    return valid;
}
} // namespace

PostalCode::PostalCode(const std::string& text)
    : m_value {normalize(text)}
{
    if (!isNumeric(m_value) && !isCpa(m_value))
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "recipient.postal_code_invalid",
                                              "El código postal debe tener 4 dígitos (1425) o formato CPA (C1425ABC)"};
    }
}

bool PostalCode::accepts(const std::string& text)
{
    const std::string value {normalize(text)};
    return isNumeric(value) || isCpa(value);
}

std::string PostalCode::normalize(const std::string& text)
{
    std::string value;
    for (const char character : text)
    {
        if (character != ' ')
        {
            value.push_back(character >= 'a' && character <= 'z' ? static_cast<char>(character - 'a' + 'A')
                                                                 : character);
        }
    }
    return value;
}

const std::string& PostalCode::value() const noexcept
{
    return m_value;
}

} // namespace hubflow::logistics::domain
