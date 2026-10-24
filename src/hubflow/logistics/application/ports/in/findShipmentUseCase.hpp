#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_SHIPMENT_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_SHIPMENT_USE_CASE_HPP

#include "hubflow/logistics/domain/services/shipmentLookup.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

    /// @brief Estructura con la que se busca un envío por código.
    enum class LookupIndex
    {
        LinkedList,
        Bst,
        BPlusTree
    };

    /**
     * @brief Busca un envío por código de seguimiento usando la estructura elegida, e informa cuántos nodos recorrió
     * para encontrarlo.
     *
     * @par Quién puede ejecutarlo
     * ADMIN y OPERADOR.
     *
     * @par Errores
     * - Forbidden: el rol no puede operar envíos.
     *
     * @par Idempotencia
     * Sí. Es una lectura sin efectos.
     */
    class FindShipmentUseCase
    {
    public:
        static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::Idempotent};

        virtual ~FindShipmentUseCase() = default;

        virtual domain::LookupResult
        execute(const shared::application::Principal& principal, const std::string& code, LookupIndex index) const = 0;
    };

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FIND_SHIPMENT_USE_CASE_HPP
