#ifndef HUBFLOW_SHARED_ADAPTERS_OUT_SYSTEM_SYSTEM_CLOCK_HPP
#define HUBFLOW_SHARED_ADAPTERS_OUT_SYSTEM_SYSTEM_CLOCK_HPP

#include "hubflow/shared/application/ports/out/clock.hpp"

namespace hubflow::shared::adapters
{

    /// @brief Reloj del sistema operativo: entrega la hora real al resto de la aplicación.
    class SystemClock final : public application::Clock
    {
    private:
        std::int64_t now() const override;
    };

} // namespace hubflow::shared::adapters

#endif // HUBFLOW_SHARED_ADAPTERS_OUT_SYSTEM_SYSTEM_CLOCK_HPP
