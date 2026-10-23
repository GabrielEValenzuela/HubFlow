#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_DELIVERY_POINT_CATALOG_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_DELIVERY_POINT_CATALOG_HPP

#include <string>

namespace hubflow::logistics::domain
{

/**
 * @brief Puerto hacia el contexto de ruteo: valida que un punto de entrega exista en el mapa y
 * pertenezca a la zona del envío (Anti-Corruption Layer).
 *
 * Vive en el dominio, y no entre los puertos de salida de la aplicación, porque ReceptionService la
 * necesita para decidir si un paquete puede recibirse.
 */
class DeliveryPointCatalog
{
public:
    virtual ~DeliveryPointCatalog() = default;

    /// @brief true si @p nodeId es un punto de entrega habilitado de la zona @p zone.
    virtual bool isServiceable(const std::string& nodeId, const std::string& zone) const = 0;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_DELIVERY_POINT_CATALOG_HPP
