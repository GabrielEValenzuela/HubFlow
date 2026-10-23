#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"

#include "hubflow/logistics/domain/valueobjects/publicCodeCheck.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr std::size_t PUBLIC_PREFIX_LENGTH {3};
constexpr std::size_t SEQUENTIAL_PREFIX_LENGTH {4};
} // namespace

TrackingCode::TrackingCode(const std::string& value)
    : m_value {value}
{
    if (!isValid(value))
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "tracking_code.invalid",
                                              "El código de seguimiento no es válido: revisá que esté bien escrito"};
    }
}

bool TrackingCode::isValid(const std::string& value)
{
    return isPublicFormat(value) || isSequentialFormat(value);
}

bool TrackingCode::isPublicFormat(const std::string& value)
{
    bool valid {value.size() == PUBLIC_LENGTH && value.compare(0, PUBLIC_PREFIX_LENGTH, PUBLIC_PREFIX) == 0};
    if (valid)
    {
        valid = PublicCodeCheck::isConsistent(value.substr(PUBLIC_PREFIX_LENGTH, PublicCodeCheck::BODY_LENGTH),
                                              value[PUBLIC_LENGTH - 1]);
    }
    return valid;
}

bool TrackingCode::isSequentialFormat(const std::string& value)
{
    bool valid {value.size() == SEQUENTIAL_LENGTH &&
                value.compare(0, SEQUENTIAL_PREFIX_LENGTH, SEQUENTIAL_PREFIX) == 0};
    for (std::size_t i = SEQUENTIAL_PREFIX_LENGTH; i < value.size() && valid; ++i)
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
