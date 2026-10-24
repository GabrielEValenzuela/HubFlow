#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CREATE_ZONE_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CREATE_ZONE_USE_CASE_HPP

#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

    /// @brief Datos de alta de la zona.
    struct CreateZoneRequest
    {
        std::string code;
        std::string name;
    };

    /**
     * @brief Da de alta una zona de reparto activa. El código identifica a la zona para siempre; el nombre es el que ve
     * el personal.
     *
     * @par Quién puede ejecutarlo
     * Sólo ADMIN.
     *
     * @par Errores
     * - Forbidden: el rol no puede administrar la configuración.
     * - Conflict: ya existe una zona con ese código.
     * - Validation: código, nombre inválidos.
     *
     * @par Idempotencia
     * No. Crea un dato nuevo. La clave natural es el código: repetir la petición falla con Conflict sin duplicar. Con
     * el encabezado Idempotency-Key el borde HTTP devuelve la respuesta original.
     */
    class CreateZoneUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

        virtual ~CreateZoneUseCase() = default;

        virtual const domain::Zone& execute(const shared::application::Principal& principal,
                                            const CreateZoneRequest& request) = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CREATE_ZONE_USE_CASE_HPP
