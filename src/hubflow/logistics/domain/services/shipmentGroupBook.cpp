#include "hubflow/logistics/domain/services/shipmentGroupBook.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{

ShipmentGroupBook::~ShipmentGroupBook()
{
    m_groups.forEach(
        [](ShipmentGroup* pGroup)
        {
            delete pGroup;
        });
}

ShipmentGroup& ShipmentGroupBook::form(const std::string&                   id,
                                       const ds::DynamicArray<std::string>& codes,
                                       const ShipmentCatalog&               catalog,
                                       const shared::domain::ActorContext&  context)
{
    std::string deliveryPoint;
    for (std::size_t i = 0; i < codes.size(); ++i)
    {
        const Shipment& shipment = catalog.require(codes[i]);
        if (groupOf(shipment.code()) != nullptr)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  "shipment_group.already_grouped",
                                                  "El paquete " + shipment.code() + " ya viaja junto con otros"};
        }
        if (!shipment.isAwaitingDispatch())
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  "shipment_group.not_awaiting",
                                                  "El paquete " + shipment.code() + " ya salió a reparto o se entregó"};
        }
        if (i == 0)
        {
            deliveryPoint = shipment.deliveryPoint();
        }
        else if (shipment.deliveryPoint() != deliveryPoint)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                                  "shipment_group.different_destination",
                                                  "Los paquetes de un grupo deben entregarse en el mismo punto"};
        }
    }
    return adopt(ShipmentGroup::form(ShipmentGroupState {id, codes, deliveryPoint}, context));
}

ShipmentGroup& ShipmentGroupBook::adopt(ShipmentGroup* pGroup)
{
    m_groups.pushBack(pGroup);
    return *pGroup;
}

ShipmentGroup& ShipmentGroupBook::require(const std::string& id) const
{
    ShipmentGroup* pFound = nullptr;
    m_groups.forEach(
        [&pFound, &id](ShipmentGroup* pGroup)
        {
            pFound = pGroup->id() == id ? pGroup : pFound;
        });
    if (pFound == nullptr)
    {
        throw shared::domain::business_error {
            shared::domain::ErrorKind::NotFound, "shipment_group.not_found", "El grupo de paquetes no existe"};
    }
    return *pFound;
}

ShipmentGroup* ShipmentGroupBook::groupOf(const std::string& code) const
{
    ShipmentGroup* pFound = nullptr;
    m_groups.forEach(
        [&pFound, &code](ShipmentGroup* pGroup)
        {
            pFound = pGroup->includes(code) ? pGroup : pFound;
        });
    return pFound;
}

void ShipmentGroupBook::ensureDissolvable(const ShipmentGroup& group, const ShipmentCatalog& catalog) const
{
    for (std::size_t i = 0; i < group.codes().size(); ++i)
    {
        if (catalog.require(group.codes()[i]).status() == ShipmentStatus::EnReparto)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict,
                                                  "shipment_group.in_delivery",
                                                  "El grupo está en reparto: no se puede deshacer hasta que vuelva"};
        }
    }
}

void ShipmentGroupBook::discard(const std::string& id)
{
    ShipmentGroup* pDoomed = nullptr;
    m_groups.removeFirstIf(
        [&pDoomed, &id](ShipmentGroup* pGroup)
        {
            const bool matches {pGroup->id() == id};
            pDoomed = matches ? pGroup : pDoomed;
            return matches;
        });
    delete pDoomed;
}

std::size_t ShipmentGroupBook::size() const noexcept
{
    return m_groups.size();
}

} // namespace hubflow::logistics::domain
