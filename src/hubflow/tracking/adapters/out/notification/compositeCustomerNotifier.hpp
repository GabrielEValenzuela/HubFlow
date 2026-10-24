#ifndef HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_COMPOSITE_CUSTOMER_NOTIFIER_HPP
#define HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_COMPOSITE_CUSTOMER_NOTIFIER_HPP

#include "hubflow/tracking/application/ports/out/customerNotifier.hpp"

#include <vector>

namespace hubflow::tracking::adapters
{

    /**
     * @brief Reparte cada aviso entre varios canales (por ejemplo la salida estándar y el registro durable).
     *
     * Si un canal falla, los demás igual reciben el aviso y la falla se informa al final.
     */
    class CompositeCustomerNotifier final : public application::CustomerNotifier
    {
    public:
        /// @brief Agrega un canal; debe vivir al menos tanto como este objeto.
        void add(application::CustomerNotifier& notifier);

    private:
        void send(const application::CustomerNotice& notice) override;

        std::vector<application::CustomerNotifier*> m_notifiers;
    };

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_OUT_NOTIFICATION_COMPOSITE_CUSTOMER_NOTIFIER_HPP
