#include "hubflow/audit/adapters/in/http/auditController.hpp"

#include "hubflow/audit/adapters/in/http/auditJsonPresenter.hpp"
#include "hubflow/platform/http/inputValidator.hpp"

namespace hubflow::audit::adapters
{
    namespace
    {
        class EntryCollector final : public application::AuditEntryVisitor
        {
        public:
            platform::Json items = platform::Json::array();

        private:
            void visit(const application::AuditEntry& entry) override
            {
                items.push_back(AuditJsonPresenter::entry(entry));
            }
        };
    } // namespace

    AuditController::AuditController(const application::ListAuditEventsUseCase& list,
                                     const application::CountAuditEventsUseCase& count)
        : m_list {list}
        , m_count {count}
    {
    }

    void AuditController::registerRoutes(platform::ApiRouter& router)
    {
        router.get("/audit-events",
                   [this](const platform::RequestContext& context)
                   {
                       auto input = platform::InputValidator::parameters(context);
                       const auto paging = input.page();
                       input.validate();
                       EntryCollector collector;
                       m_list.execute(context.user(), paging.first, paging.second, collector);
                       return platform::HttpResponse::ok(platform::Json {{"items", collector.items},
                                                                         {"total", m_count.execute(context.user())},
                                                                         {"limit", paging.first},
                                                                         {"offset", paging.second}});
                   });
    }

} // namespace hubflow::audit::adapters
