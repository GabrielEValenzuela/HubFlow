#include "hubflow/tracking/adapters/out/notification/logCustomerNotifier.hpp"

#include <ostream>

namespace hubflow::tracking::adapters
{

    LogCustomerNotifier::LogCustomerNotifier(std::ostream& out)
        : m_out {out}
    {
    }

    void LogCustomerNotifier::send(const application::CustomerNotice& notice)
    {
        m_out << "[hubflow] aviso al cliente por " << notice.channel << " a " << notice.recipient << ": "
              << notice.message << '\n';
    }

} // namespace hubflow::tracking::adapters
