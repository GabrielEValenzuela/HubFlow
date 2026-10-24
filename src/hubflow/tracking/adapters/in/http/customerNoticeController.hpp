#ifndef HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_CUSTOMER_NOTICE_CONTROLLER_HPP
#define HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_CUSTOMER_NOTICE_CONTROLLER_HPP

#include "hubflow/platform/http/controller.hpp"
#include "hubflow/tracking/application/ports/in/listCustomerNoticesUseCase.hpp"

namespace hubflow::tracking::adapters
{

    /// @brief Avisos a clientes: @c GET /customer-notices (administrador), más recientes primero, paginado.
    class CustomerNoticeController final : public platform::Controller
    {
    public:
        explicit CustomerNoticeController(const application::ListCustomerNoticesUseCase& list);

    private:
        void registerRoutes(platform::ApiRouter& router) override;

        const application::ListCustomerNoticesUseCase& m_list;
    };

} // namespace hubflow::tracking::adapters

#endif // HUBFLOW_TRACKING_ADAPTERS_IN_HTTP_CUSTOMER_NOTICE_CONTROLLER_HPP
