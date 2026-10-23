#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_SERVICE_LEVEL_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_SERVICE_LEVEL_USE_CASE_HPP

#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <optional>
#include <string>

namespace hubflow::logistics::application
{

/// @brief Cambios parciales sobre el servicio: sólo se aplican los campos presentes.
struct UpdateServiceLevelRequest
{
    std::optional<std::string> name;
    std::optional<bool>        active;
};

/**
 * @brief Cambia el nombre de un nivel de servicio o lo habilita o desactiva. Desactivar exige que no haya paquetes en
 * la planta con ese servicio.
 *
 * @par Quién puede ejecutarlo
 * Sólo ADMIN.
 *
 * @par Errores
 * - Forbidden: el rol no puede administrar la configuración.
 * - NotFound: no existe un servicio con ese código.
 * - Validation: nombre inválido.
 * - Conflict: hay paquetes en la planta con este servicio.
 *
 * @par Idempotencia
 * Sí. Describe el estado final deseado de los campos enviados: aplicar el mismo cambio dos veces deja el dato
 * exactamente igual y la segunda vez no registra ningún evento.
 */
class UpdateServiceLevelUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~UpdateServiceLevelUseCase() = default;

    virtual const domain::ServiceLevel& execute(const shared::application::Principal& principal,
                                                const std::string&                    code,
                                                const UpdateServiceLevelRequest&      request) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_SERVICE_LEVEL_USE_CASE_HPP
