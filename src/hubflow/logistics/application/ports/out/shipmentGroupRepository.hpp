#ifndef HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_SHIPMENT_GROUP_REPOSITORY_HPP
#define HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_SHIPMENT_GROUP_REPOSITORY_HPP

#include "hubflow/logistics/domain/entities/shipmentGroup.hpp"

namespace hubflow::logistics::application
{

/// @brief Receptor de grupos durante la hidratación desde el almacenamiento.
class ShipmentGroupRestorer
{
public:
    virtual ~ShipmentGroupRestorer() = default;

    virtual void restoreGroup(const domain::ShipmentGroupState& state) = 0;
};

/// @brief Persistencia de los grupos de envíos que viajan juntos.
class ShipmentGroupRepository
{
public:
    virtual ~ShipmentGroupRepository() = default;

    /// @brief Guarda un grupo nuevo con sus miembros.
    virtual void add(const domain::ShipmentGroupState& state) = 0;

    /// @brief Borra el grupo y sus miembros.
    virtual void remove(const std::string& id) = 0;

    virtual void loadAll(ShipmentGroupRestorer& restorer) const = 0;
};

} // namespace hubflow::logistics::application

#endif // HUBFLOW_LOGISTICS_APPLICATION_PORTS_OUT_SHIPMENT_GROUP_REPOSITORY_HPP
