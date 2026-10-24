#include "hubflow/tracking/adapters/in/http/customerNoticeController.hpp"

#include "hubflow/platform/http/inputValidator.hpp"
#include "hubflow/tracking/adapters/in/http/trackingJsonPresenter.hpp"

namespace hubflow::tracking::adapters
{
    namespace
    {
        class NoticeCollector final : public application::CustomerNoticeVisitor
        {
        public:
            platform::Json items = platform::Json::array();

        private:
            void visit(const application::StoredCustomerNotice& stored) override
            {
                items.push_back(TrackingJsonPresenter::notice(stored));
            }
        };
    } // namespace

    CustomerNoticeController::CustomerNoticeController(const application::ListCustomerNoticesUseCase& list)
        : m_list {list}
    {
    }

    void CustomerNoticeController::registerRoutes(platform::ApiRouter& router)
    {
        router.get(
            "/customer-notices",
            [this](const platform::RequestContext& context)
            {
                auto input = platform::InputValidator::parameters(context);
                const auto paging = input.page();
                input.validate();
                NoticeCollector collector;
                const auto total = m_list.execute(context.user(), paging.first, paging.second, collector);
                return platform::HttpResponse::ok(platform::Json {
                    {"items", collector.items}, {"total", total}, {"limit", paging.first}, {"offset", paging.second}});
            });
    }

} // namespace hubflow::tracking::adapters
