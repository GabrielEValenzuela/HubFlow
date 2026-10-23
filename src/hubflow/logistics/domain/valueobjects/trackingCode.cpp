#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr const char* CODE_PREFIX {"PKG-"};
constexpr std::size_t PREFIX_LENGTH {4};
constexpr std::size_t CODE_LENGTH {8};
} // namespace

TrackingCode::TrackingCode(const std::string& value)
    : m_value {value}
{
    if (!isValid(value))
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation, "tracking_code.invalid", "El código debe tener el formato PKG-NNNN"};
    }
}

bool TrackingCode::isValid(const std::string& value)
{
    bool valid {value.size() == CODE_LENGTH && value.compare(0, PREFIX_LENGTH, CODE_PREFIX) == 0};
    for (std::size_t i = PREFIX_LENGTH; i < value.size() && valid; ++i)
    {
        valid = value[i] >= '0' && value[i] <= '9';
    }
    return valid;
}

const std::string& TrackingCode::value() const noexcept
{
    return m_value;
}

bool TrackingCode::operator<(const TrackingCode& other) const noexcept
{
    return m_value < other.m_value;
}

bool TrackingCode::operator==(const TrackingCode& other) const noexcept
{
    return m_value == other.m_value;
}

} // namespace hubflow::logistics::domain
