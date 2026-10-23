#ifndef HUBFLOW_INFRASTRUCTURE_SYSTEM_SYSTEM_CLOCK_HPP
#define HUBFLOW_INFRASTRUCTURE_SYSTEM_SYSTEM_CLOCK_HPP

#include "application/shared/clock.hpp"

namespace hubflow::infrastructure
{

/// @brief Reloj del sistema operativo.
class SystemClock final : public application::Clock
{
private:
    std::int64_t now() const override;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_SYSTEM_SYSTEM_CLOCK_HPP
