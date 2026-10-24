#include "bootstrap/moduleSet.hpp"

#include "hubflow/shared/application/eventBus.hpp"

namespace hubflow::bootstrap
{

    void PhasedSubscribers::add(const SubscriptionPhase phase, shared::domain::DomainEventVisitor& subscriber)
    {
        m_entries.emplace_back(phase, &subscriber);
    }

    void PhasedSubscribers::subscribe(const SubscriptionPhase phase, shared::application::EventBus& bus) const
    {
        for (const auto& entry : m_entries)
        {
            if (entry.first == phase)
            {
                bus.subscribe(*entry.second);
            }
        }
    }

    void BoundedContextModule::subscribe(SubscriptionPhase, shared::application::EventBus&) {}

    void BoundedContextModule::hydrate() {}

    void BoundedContextModule::seed() {}

    void ModuleSet::subscribeAll(shared::application::EventBus& bus)
    {
        for (const SubscriptionPhase phase : SUBSCRIPTION_ORDER)
        {
            for (auto& spModule : m_modules)
            {
                spModule->subscribe(phase, bus);
            }
        }
    }

    void ModuleSet::hydrateAll()
    {
        for (auto& spModule : m_modules)
        {
            spModule->hydrate();
        }
    }

    void ModuleSet::seedAll()
    {
        for (auto& spModule : m_modules)
        {
            spModule->seed();
        }
    }

    void ModuleSet::registerAllRoutes(platform::ApiRouter& router)
    {
        for (auto& spModule : m_modules)
        {
            spModule->registerRoutes(router);
        }
    }

} // namespace hubflow::bootstrap
