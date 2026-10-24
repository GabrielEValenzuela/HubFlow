#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DISSOLVE_SHIPMENT_GROUP_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DISSOLVE_SHIPMENT_GROUP_USE_CASE_HPP

#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

    /**
     * @brief Deshace un grupo: sus paquetes dejan de viajar juntos y vuelven a tratarse por separado.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede operar envíos.
     * - NotFound: el grupo no existe (o ya fue deshecho).
     * - Conflict: algún paquete del grupo está en reparto.
     *
     * @par Idempotencia
     * Sí. El efecto buscado, que el grupo no exista, queda igual al repetirla: el segundo intento responde NotFound sin
     * modificar nada.
     */
    class DissolveShipmentGroupUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~DissolveShipmentGroupUseCase() = default;

        virtual void execute(const shared::application::Principal& principal, const std::string& groupId) = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_DISSOLVE_SHIPMENT_GROUP_USE_CASE_HPP
