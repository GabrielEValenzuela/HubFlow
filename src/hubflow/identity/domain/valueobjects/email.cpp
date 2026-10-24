#include "hubflow/identity/domain/valueobjects/email.hpp"
#include <algorithm>

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::identity::domain
{
    namespace
    {
        constexpr std::size_t MAX_EMAIL_LENGTH {120};

        std::string lowercase(const std::string& text)
        {
            std::string result {text};
            std::transform(result.begin(),
                           result.end(),
                           result.begin(),
                           [](const char character)
                           {
                               return character >= 'A' && character <= 'Z' ? static_cast<char>(character - 'A' + 'a')
                                                                           : character;
                           });
            return result;
        }

        bool isWellFormed(const std::string& value)
        {
            const std::size_t at {value.find('@')};
            const std::size_t dot {value.rfind('.')};
            return !value.empty() && value.size() <= MAX_EMAIL_LENGTH && at != std::string::npos && at > 0 &&
                   value.find('@', at + 1) == std::string::npos && dot != std::string::npos && dot > at + 1 &&
                   dot + 1 < value.size() && value.find(' ') == std::string::npos;
        }
    } // namespace

    Email::Email(const std::string& value)
        : m_value {lowercase(value)}
    {
        if (!isWellFormed(m_value))
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::Validation, "email.invalid", "Email inválido"};
        }
    }

    const std::string& Email::value() const noexcept
    {
        return m_value;
    }

} // namespace hubflow::identity::domain
