#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_SERVICE_LEVELS_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_SERVICE_LEVELS_USE_CASE_HPP

#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

namespace hubflow::logistics::application
{

    /**
     * @brief Devuelve todos los niveles de servicio, activos y desactivados, en orden de alta.
     *
     * @par Quién puede ejecutarlo
     * Cualquier usuario autenticado: son los datos con los que se completan las pantallas de la planta.
     *
     * @par Errores
     * - Ninguno propio de la operación.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class ListServiceLevelsUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~ListServiceLevelsUseCase() = default;

        virtual const domain::ServiceLevelCatalog& execute(const shared::application::Principal& principal) const = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_LIST_SERVICE_LEVELS_USE_CASE_HPP
