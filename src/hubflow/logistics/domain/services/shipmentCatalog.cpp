#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{

    ShipmentCatalog::~ShipmentCatalog()
    {
        m_shipments.forEach([](Shipment* pShipment) { delete pShipment; });
    }

    Shipment& ShipmentCatalog::adopt(Shipment* pShipment)
    {
        if (contains(pShipment->code()))
        {
            delete pShipment;
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::Conflict, "shipment.duplicate", "Ya existe un envío con ese código"};
        }
        m_shipments.pushBack(pShipment);
        m_bst.insert(pShipment->code(), pShipment);
        m_bPlusTree.insert(pShipment->code(), pShipment);
        return *pShipment;
    }

    bool ShipmentCatalog::contains(const std::string& code) const
    {
        return m_bPlusTree.find(code).pValue != nullptr;
    }

    Shipment& ShipmentCatalog::require(const std::string& code) const
    {
        Shipment* pShipment = m_bPlusTree.find(code).pValue;
        if (pShipment == nullptr)
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::NotFound, "shipment.not_found", "Envío no encontrado."};
        }
        return *pShipment;
    }

    const ds::SinglyLinkedList<Shipment*>& ShipmentCatalog::allShipments() const noexcept
    {
        return m_shipments;
    }

    const ShipmentBst& ShipmentCatalog::bst() const noexcept
    {
        return m_bst;
    }

    const ShipmentBPlusTree& ShipmentCatalog::bPlusTree() const noexcept
    {
        return m_bPlusTree;
    }

    std::size_t ShipmentCatalog::size() const noexcept
    {
        return m_shipments.size();
    }

} // namespace hubflow::logistics::domain
