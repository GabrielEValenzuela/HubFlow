#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CREATE_SERVICE_LEVEL_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CREATE_SERVICE_LEVEL_USE_CASE_HPP

#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

    /// @brief Datos de alta de el servicio.
    struct CreateServiceLevelRequest
    {
        std::string code;
        std::string name;
        unsigned priority {0};
    };

    /**
     * @brief Da de alta un nivel de servicio activo con su prioridad de despacho. Un número mayor sale antes de la
     * planta; la prioridad no se puede cambiar después.
     *
     * @par Quién puede ejecutarlo
     * Sólo ADMIN.
     *
     * @par Errores
     * - Forbidden: el rol no puede administrar la configuración.
     * - Conflict: ya existe un servicio con ese código.
     * - Validation: código, nombre o prioridad inválidos.
     *
     * @par Idempotencia
     * No. Crea un dato nuevo. La clave natural es el código: repetir la petición falla con Conflict sin duplicar. Con
     * el encabezado Idempotency-Key el borde HTTP devuelve la respuesta original.
     */
    class CreateServiceLevelUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

        virtual ~CreateServiceLevelUseCase() = default;

        virtual const domain::ServiceLevel& execute(const shared::application::Principal& principal,
                                                    const CreateServiceLevelRequest& request) = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_CREATE_SERVICE_LEVEL_USE_CASE_HPP
