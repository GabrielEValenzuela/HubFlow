#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_SHIPMENT_FEATURE_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_SHIPMENT_FEATURE_USE_CASE_HPP

#include "hubflow/logistics/domain/services/referenceCatalog.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <optional>
#include <string>

namespace hubflow::logistics::application
{

/// @brief Cambios parciales sobre la característica: sólo se aplican los campos presentes.
struct UpdateShipmentFeatureRequest
{
    std::optional<std::string> name;
    std::optional<bool>        active;
};

/**
 * @brief Cambia el nombre de una característica o la habilita o desactiva. Desactivarla sólo impide etiquetar paquetes
 * nuevos: los que ya la tienen la conservan.
 *
 * @par Quién puede ejecutarlo
 * Sólo ADMIN.
 *
 * @par Errores
 * - Forbidden: el rol no puede administrar la configuración.
 * - NotFound: no existe una característica con ese código.
 * - Validation: nombre inválido.
 *
 * @par Idempotencia
 * Sí. Describe el estado final deseado de los campos enviados: aplicar el mismo cambio dos veces deja el dato
 * exactamente igual y la segunda vez no registra ningún evento.
 */
class UpdateShipmentFeatureUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

    virtual ~UpdateShipmentFeatureUseCase() = default;

    virtual const domain::ShipmentFeature& execute(const shared::application::Principal& principal,
                                                   const std::string&                    code,
                                                   const UpdateShipmentFeatureRequest&   request) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_UPDATE_SHIPMENT_FEATURE_USE_CASE_HPP
