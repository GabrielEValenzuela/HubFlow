#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_CUSTOMER_NOTICE_LOG_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_CUSTOMER_NOTICE_LOG_HPP

#include "hubflow/tracking/application/ports/out/customerNotifier.hpp"

#include <cstddef>
#include <cstdint>

namespace hubflow::tracking::application
{

    /// @brief Un aviso guardado, con su número de orden.
    struct StoredCustomerNotice
    {
        std::uint64_t id {0};
        CustomerNotice notice;
    };

    /// @brief Receptor de avisos guardados.
    class CustomerNoticeVisitor
    {
    public:
        virtual ~CustomerNoticeVisitor() = default;

        virtual void visit(const StoredCustomerNotice& stored) = 0;
    };

    /// @brief Registro durable de los avisos que se mandaron (o se habrían mandado) a los clientes.
    class CustomerNoticeLog
    {
    public:
        virtual ~CustomerNoticeLog() = default;

        virtual void append(const CustomerNotice& notice) = 0;

        /// @brief Avisos más recientes primero.
        virtual void forEachRecent(std::size_t limit, std::size_t offset, CustomerNoticeVisitor& visitor) const = 0;

        virtual std::size_t count() const = 0;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_OUT_CUSTOMER_NOTICE_LOG_HPP
