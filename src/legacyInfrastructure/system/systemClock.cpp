#include "infrastructure/system/systemClock.hpp"

#include <chrono>

namespace hubflow::infrastructure
{

std::int64_t SystemClock::now() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

} // namespace hubflow::infrastructure
