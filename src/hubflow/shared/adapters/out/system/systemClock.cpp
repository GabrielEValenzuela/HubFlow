#include "hubflow/shared/adapters/out/system/systemClock.hpp"

#include <chrono>

namespace hubflow::shared::adapters
{

    std::int64_t SystemClock::now() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

} // namespace hubflow::shared::adapters
