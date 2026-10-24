#ifndef HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_LOG_CUSTOMER_NOTIFIER_HPP
#define HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_LOG_CUSTOMER_NOTIFIER_HPP

#include "hubflow/tracking/application/ports/out/customerNotifier.hpp"

#include <iosfwd>

namespace hubflow::tracking::adapters
{

    /// @brief Escribe cada aviso en la salida estándar, como lo haría el servicio de mensajería real.
    class LogCustomerNotifier final : public application::CustomerNotifier
    {
    public:
        explicit LogCustomerNotifier(std::ostream& out);

    private:
        void send(const application::CustomerNotice& notice) override;

        std::ostream& m_out;
    };

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_LOG_CUSTOMER_NOTIFIER_HPP
