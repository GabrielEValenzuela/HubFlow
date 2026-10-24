#include "hubflow/shared/domain/businessError.hpp"

#include <utility>

namespace hubflow::shared::domain
{

    BusinessError::BusinessError(const ErrorKind kind, std::string code, const std::string& message)
        : std::runtime_error {message}
        , m_kind {kind}
        , m_code {std::move(code)}
    {
    }

    BusinessError::BusinessError(const ErrorKind kind,
                                 std::string code,
                                 const std::string& message,
                                 const std::uint32_t retryAfterSeconds)
        : std::runtime_error {message}
        , m_kind {kind}
        , m_code {std::move(code)}
        , m_retryAfterSeconds {retryAfterSeconds}
    {
    }

    ErrorKind BusinessError::errorKind() const noexcept
    {
        return m_kind;
    }

    const std::string& BusinessError::code() const noexcept
    {
        return m_code;
    }

    std::uint32_t BusinessError::retryAfterSeconds() const noexcept
    {
        return m_retryAfterSeconds;
    }

} // namespace hubflow::shared::domain
