#ifndef HUBFLOW_SHARED_APPLICATION_PORTS_OUT_CLOCK_HPP
#define HUBFLOW_SHARED_APPLICATION_PORTS_OUT_CLOCK_HPP

#include <cstdint>

namespace hubflow::shared::application
{

/// @brief Fuente de tiempo (puerto), reemplazable en tests.
class Clock
{
public:
    virtual ~Clock() = default;

    /// @brief Instante actual en segundos desde epoch.
    virtual std::int64_t now() const = 0;
};

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_PORTS_OUT_CLOCK_HPP
