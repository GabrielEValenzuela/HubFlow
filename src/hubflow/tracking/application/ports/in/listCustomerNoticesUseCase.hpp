#ifndef HUBFLOW_TRACKING_APPLICATION_PORTS_IN_LIST_CUSTOMER_NOTICES_USE_CASE_HPP
#define HUBFLOW_TRACKING_APPLICATION_PORTS_IN_LIST_CUSTOMER_NOTICES_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include "hubflow/tracking/application/ports/out/customerNoticeLog.hpp"

#include <cstddef>

namespace hubflow::tracking::application
{

    /**
     * @brief Lista los avisos que se mandaron (o se habrían mandado) a los clientes, del más reciente al más antiguo.
     *
     * Cada aviso muestra el paquete, el canal, el contacto enmascarado, el mensaje y cuándo salió.
     *
     * @par Quién puede ejecutarlo
     * Sólo ADMIN.
     *
     * @par Errores
     * - Forbidden: el solicitante no es ADMIN.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class ListCustomerNoticesUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ListCustomerNoticesUseCase() = default;

        /// @return la cantidad total de avisos registrados.
        virtual std::size_t execute(const shared::application::Principal& principal,
                                    std::size_t limit,
                                    std::size_t offset,
                                    CustomerNoticeVisitor& visitor) const = 0;
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_PORTS_IN_LIST_CUSTOMER_NOTICES_USE_CASE_HPP
