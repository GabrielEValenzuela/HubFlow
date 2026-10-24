#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_ZONE_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_ZONE_USE_CASE_HPP

#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <optional>
#include <string>

namespace hubflow::logistics::application
{

    /// @brief Cambios parciales sobre la zona: sólo se aplican los campos presentes.
    struct UpdateZoneRequest
    {
        std::optional<std::string> name;
        std::optional<bool> active;
    };

    /**
     * @brief Cambia el nombre de una zona o la habilita o desactiva. Desactivar exige que no haya paquetes en la planta
     * para la zona, que ningún reparto en curso pase por ella y que no le queden puntos activos del mapa (se pasan a
     * otra zona o se desactivan).
     *
     * @par Quién puede ejecutarlo
     * Sólo ADMIN.
     *
     * @par Errores
     * - Forbidden: el rol no puede administrar la configuración.
     * - NotFound: no existe una zona con ese código.
     * - Validation: nombre inválido.
     * - Conflict: la zona tiene paquetes en planta, repartos en curso o puntos activos del mapa.
     *
     * @par Idempotencia
     * Sí. Describe el estado final deseado de los campos enviados: aplicar el mismo cambio dos veces deja el dato
     * exactamente igual y la segunda vez no registra ningún evento.
     */
    class UpdateZoneUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~UpdateZoneUseCase() = default;

        virtual const domain::Zone& execute(const shared::application::Principal& principal,
                                            const std::string& code,
                                            const UpdateZoneRequest& request) = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_ZONE_USE_CASE_HPP
