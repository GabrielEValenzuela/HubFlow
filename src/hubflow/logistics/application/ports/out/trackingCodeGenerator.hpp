#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_TRACKING_CODE_GENERATOR_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_TRACKING_CODE_GENERATOR_HPP

#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"

#include <string>

namespace hubflow::logistics::application
{

/**
 * @brief Asigna el código de seguimiento de un envío nuevo (puerto).
 *
 * El operador no elige el código: lo decide el sistema según la forma de trabajo de la empresa (códigos públicos no
 * adivinables o códigos correlativos).
 */
class TrackingCodeGenerator
{
public:
    virtual ~TrackingCodeGenerator() = default;

    /**
     * @brief Devuelve un código válido que no usa ningún envío de @p catalog.
     * @throws business_error (Conflict) si no quedan códigos disponibles.
     */
    virtual std::string next(const domain::ShipmentCatalog& catalog) = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_TRACKING_CODE_GENERATOR_HPP
