#include "hubflow/tracking/domain/services/deliveryWindow.hpp"

namespace hubflow::tracking::domain
{
    namespace
    {
        constexpr std::int64_t SECONDS_PER_MINUTE {60};
    } // namespace

    DeliveryWindow DeliveryWindowPolicy::around(const std::int64_t now, const std::uint32_t minutesToArrival) noexcept
    {
        const std::int64_t arrival {now + std::int64_t {minutesToArrival} * SECONDS_PER_MINUTE};
        std::int64_t from {arrival - arrival % ROUNDING_SECONDS};
        if (from < now)
        {
            from = now;
        }
        return DeliveryWindow {from, from + LENGTH_SECONDS};
    }

} // namespace hubflow::tracking::domain
