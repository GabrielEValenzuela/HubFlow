#ifndef HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_LIST_COURIERS_USE_CASE_HPP
#define HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_LIST_COURIERS_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/identity/domain/entities/user.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::identity::application
{

    /**
     * @brief Lista los repartidores PUM activos, que son los únicos a quienes se puede asignar una ruta.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede asignar rutas.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class ListCouriersUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ListCouriersUseCase() = default;

        virtual ds::DynamicArray<domain::UserState> execute(const shared::application::Principal& principal) const = 0;
    };

} // namespace hubflow::identity::application

#endif // HUBFLOW_IDENTITY_APPLICATION_PORTS_IN_LIST_COURIERS_USE_CASE_HPP
