#ifndef HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_RECORDING_CUSTOMER_NOTIFIER_HPP
#define HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_RECORDING_CUSTOMER_NOTIFIER_HPP

#include "hubflow/tracking/application/ports/out/customerNoticeLog.hpp"

namespace hubflow::tracking::adapters
{

    /// @brief Guarda cada aviso en el registro durable para que el administrador vea qué se habría mandado a los
    /// clientes.
    class RecordingCustomerNotifier final : public application::CustomerNotifier
    {
    public:
        explicit RecordingCustomerNotifier(application::CustomerNoticeLog& log);

    private:
        void send(const application::CustomerNotice& notice) override;

        application::CustomerNoticeLog& m_log;
    };

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_RECORDING_CUSTOMER_NOTIFIER_HPP
