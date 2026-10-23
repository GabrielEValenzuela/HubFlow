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

ErrorKind business_error::kind() const noexcept
{
    return m_kind;
}

const std::string& business_error::code() const noexcept
{
    return m_code;
}

} // namespace hubflow::shared::domain
