#include "bootstrap/moduleSet.hpp"

#include "hubflow/identity/domain/events/userEvents.hpp"
#include "hubflow/shared/application/eventBus.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace hubflow;
using bootstrap::SubscriptionPhase;

namespace
{
class RecordingReactor final : public shared::domain::DomainEventVisitor
{
public:
    RecordingReactor(std::string name, std::vector<std::string>& journal)
        : m_name {std::move(name)}
        , m_journal {journal}
    {
    }

private:
    void on(const identity::domain::UserRegistered&) override
    {
        m_journal.push_back(m_name);
    }

    std::string               m_name;
    std::vector<std::string>& m_journal;
};

struct ReactorSpec
{
    SubscriptionPhase phase;
    const char*       name;
};

class FakeModule final : public bootstrap::BoundedContextModule
{
public:
    FakeModule(const std::vector<ReactorSpec>& specs, std::vector<std::string>& journal, std::string label)
        : m_label {std::move(label)}
        , m_journal {journal}
    {
        for (const auto& spec : specs)
        {
            m_reactors.push_back(std::make_unique<RecordingReactor>(spec.name, journal));
            m_subscribers.add(spec.phase, *m_reactors.back());
        }
    }

private:
    void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus) override
    {
        m_subscribers.subscribe(phase, bus);
    }

    void hydrate() override
    {
        m_journal.push_back("hydrate:" + m_label);
    }

    void seed() override
    {
        m_journal.push_back("seed:" + m_label);
    }

    void registerRoutes(hubflow::platform::ApiRouter&) override {}

    std::string                                    m_label;
    std::vector<std::string>&                      m_journal;
    bootstrap::PhasedSubscribers                   m_subscribers;
    std::vector<std::unique_ptr<RecordingReactor>> m_reactors;
};

identity::domain::UserRegistered someEvent()
{
    return identity::domain::UserRegistered {identity::domain::UserState {},
                                             shared::domain::ActorContext {1, "Prueba", 0}};
}
} // namespace

TEST(ModuleSet, ReactorsRunByPhaseNoMatterWhichModuleDeclaresThem)
{
    std::vector<std::string> journal;
    bootstrap::ModuleSet     modules;
    modules.add(std::make_unique<FakeModule>(
        std::vector<ReactorSpec> {{SubscriptionPhase::Audit, "audit"}, {SubscriptionPhase::Persistence, "users"}},
        journal,
        "first"));
    modules.add(std::make_unique<FakeModule>(std::vector<ReactorSpec> {{SubscriptionPhase::CustomerNotice, "notice"},
                                                                       {SubscriptionPhase::Persistence, "shipments"},
                                                                       {SubscriptionPhase::Projection, "pending"}},
                                             journal,
                                             "second"));
    modules.add(std::make_unique<FakeModule>(
        std::vector<ReactorSpec> {{SubscriptionPhase::Export, "export"}, {SubscriptionPhase::Persistence, "routes"}},
        journal,
        "third"));
    shared::application::EventBus bus;
    modules.subscribeAll(bus);

    static_cast<shared::domain::DomainEventPublisher&>(bus).publish(someEvent());

    const std::vector<std::string> expected {"pending", "users", "shipments", "routes", "export", "notice", "audit"};
    EXPECT_EQ(journal, expected);
}

TEST(ModuleSet, HydratesAndSeedsInRegistrationOrder)
{
    std::vector<std::string> journal;
    bootstrap::ModuleSet     modules;
    modules.add(std::make_unique<FakeModule>(std::vector<ReactorSpec> {}, journal, "identity"));
    modules.add(std::make_unique<FakeModule>(std::vector<ReactorSpec> {}, journal, "logistics"));

    modules.hydrateAll();
    modules.seedAll();

    const std::vector<std::string> expected {
        "hydrate:identity", "hydrate:logistics", "seed:identity", "seed:logistics"};
    EXPECT_EQ(journal, expected);
}
