#include "hubflow/logistics/domain/valueobjects/weight.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{

Weight::Weight(const double kilograms)
    : m_kilograms {kilograms}
{
    if (!(kilograms > 0.0) || kilograms > MAX_KILOGRAMS)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::Validation, "weight.invalid", "El peso debe ser mayor a 0 y como máximo 50 kg"};
    }
}

double Weight::kilograms() const noexcept
{
    return m_kilograms;
}

} // namespace hubflow::logistics::domain
