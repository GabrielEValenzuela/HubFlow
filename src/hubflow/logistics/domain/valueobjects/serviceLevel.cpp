#include "hubflow/logistics/domain/valueobjects/serviceLevel.hpp"

namespace hubflow::logistics::domain
{
namespace
{
constexpr unsigned EXPRESS_PRIORITY {3};
constexpr unsigned PRIORITARIO_PRIORITY {2};
constexpr unsigned ESTANDAR_PRIORITY {1};
} // namespace

unsigned priorityOf(const ServiceLevel level) noexcept
{
    unsigned priority {ESTANDAR_PRIORITY};
    if (level == ServiceLevel::Express)
    {
        priority = EXPRESS_PRIORITY;
    }
    else if (level == ServiceLevel::Prioritario)
    {
        priority = PRIORITARIO_PRIORITY;
    }
    return priority;
}

} // namespace hubflow::logistics::domain
