#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FORM_SHIPMENT_GROUP_USE_CASE_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FORM_SHIPMENT_GROUP_USE_CASE_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/domain/entities/shipmentGroup.hpp"
#include "hubflow/shared/application/principal.hpp"
#include "hubflow/shared/application/useCase.hpp"

#include <string>

namespace hubflow::logistics::application
{

/**
 * @brief Une dos o más paquetes para que viajen juntos: se cargan juntos, salen en una sola parada y, si no se
 * pueden entregar, se reprograman juntos.
 *
 * Los paquetes deben compartir el punto de entrega, estar esperando despacho y no pertenecer ya a otro grupo.
 *
 * @par Quién puede ejecutarlo
 * ADMIN y OPERADOR.
 *
 * @par Errores
 * - Forbidden: el rol no puede operar envíos.
 * - NotFound: algún código no corresponde a un paquete.
 * - Conflict: algún paquete ya viaja con otros o ya salió a reparto.
 * - Validation: menos de dos paquetes, códigos repetidos o puntos de entrega distintos.
 *
 * @par Idempotencia
 * No. Crea un grupo nuevo con identidad propia. La clave natural es el conjunto de paquetes: repetir la petición falla
 * con Conflict porque ya están agrupados, sin crear otro grupo. Con Idempotency-Key el borde HTTP devuelve la
 * respuesta original.
 */
class FormShipmentGroupUseCase
{
public:
    static constexpr shared::application::Idempotency IDEMPOTENCY {shared::application::Idempotency::NonIdempotent};

    virtual ~FormShipmentGroupUseCase() = default;

    virtual const domain::ShipmentGroup& execute(const shared::application::Principal& principal,
                                                 const ds::DynamicArray<std::string>&  codes) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_IN_FORM_SHIPMENT_GROUP_USE_CASE_HPP
