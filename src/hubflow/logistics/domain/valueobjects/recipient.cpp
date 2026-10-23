#include "hubflow/logistics/domain/valueobjects/recipient.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr std::size_t MIN_NAME_LENGTH {2};
constexpr std::size_t MAX_NAME_LENGTH {80};

std::string trim(const std::string& text)
{
    std::size_t begin {0};
    std::size_t end {text.size()};
    while (begin < end && text[begin] == ' ')
    {
        ++begin;
    }
    while (end > begin && text[end - 1] == ' ')
    {
        --end;
    }
    return text.substr(begin, end - begin);
}
} // namespace

Recipient::Recipient(const std::string& name, const std::string& phone, const std::string& postalCode)
    : m_name {trim(name)}
    , m_phone {phone}
    , m_postalCode {postalCode}
{
    if (m_name.size() < MIN_NAME_LENGTH || m_name.size() > MAX_NAME_LENGTH)
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "recipient.invalid",
                                              "El destinatario debe tener entre 2 y 80 caracteres"};
    }
}

const std::string& Recipient::name() const noexcept
{
    return m_name;
}

const Phone& Recipient::phone() const noexcept
{
    return m_phone;
}

const PostalCode& Recipient::postalCode() const noexcept
{
    return m_postalCode;
}

std::string Recipient::firstName() const
{
    return m_name.substr(0, m_name.find(' '));
}

} // namespace hubflow::logistics::domain
