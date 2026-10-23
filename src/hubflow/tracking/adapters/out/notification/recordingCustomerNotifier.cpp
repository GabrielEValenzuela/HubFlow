#include "hubflow/tracking/adapters/out/notification/recordingCustomerNotifier.hpp"

namespace hubflow::tracking::adapters
{

RecordingCustomerNotifier::RecordingCustomerNotifier(application::CustomerNoticeLog& log)
    : m_log {log}
{
}

void RecordingCustomerNotifier::send(const application::CustomerNotice& notice)
{
    m_log.append(notice);
}

} // namespace hubflow::tracking::adapters
