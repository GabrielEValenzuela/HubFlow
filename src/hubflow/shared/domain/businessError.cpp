#include "hubflow/shared/domain/businessError.hpp"

#include <utility>

namespace hubflow::shared::domain
{

    business_error::business_error(const ErrorKind kind, std::string code, const std::string& message)
        : std::runtime_error {message}
        , m_kind {kind}
        , m_code {std::move(code)}
    {
    }

    business_error::business_error(const ErrorKind kind,
                                   std::string code,
                                   const std::string& message,
                                   const std::uint32_t retryAfterSeconds)
        : std::runtime_error {message}
        , m_kind {kind}
        , m_code {std::move(code)}
        , m_retryAfterSeconds {retryAfterSeconds}
    {
    }

    ErrorKind business_error::errorKind() const noexcept
    {
        return m_kind;
    }

    const std::string& business_error::code() const noexcept
    {
        return m_code;
    }

    std::uint32_t business_error::retryAfterSeconds() const noexcept
    {
        return m_retryAfterSeconds;
    }

} // namespace hubflow::shared::domain
