#include "hubflow/tracking/adapters/out/notification/compositeCustomerNotifier.hpp"

#include <exception>

namespace hubflow::tracking::adapters
{

void CompositeCustomerNotifier::add(application::CustomerNotifier& notifier)
{
    m_notifiers.push_back(&notifier);
}

void CompositeCustomerNotifier::send(const application::CustomerNotice& notice)
{
    std::exception_ptr failure;
    for (auto* pNotifier : m_notifiers)
    {
        try
        {
            pNotifier->send(notice);
        }
        catch (...)
        {
            failure = std::current_exception();
        }
    }
    if (failure)
    {
        std::rethrow_exception(failure);
    }
}

} // namespace hubflow::tracking::adapters
