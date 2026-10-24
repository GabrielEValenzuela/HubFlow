#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_CATALOG_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_CATALOG_HPP

#include "datastructures/bPlusTree.hpp"
#include "datastructures/binarySearchTree.hpp"
#include "datastructures/singlyLinkedList.hpp"
#include "hubflow/logistics/domain/entities/shipment.hpp"

namespace hubflow::logistics::domain
{

    using ShipmentBst = ds::BinarySearchTree<std::string, Shipment>;
    using ShipmentBPlusTree = ds::BPlusTree<std::string, Shipment, 4>;

    /**
     * @brief Colección en memoria de todos los envíos conocidos y sus índices por código.
     *
     * Ownership:
     * - Es el ÚNICO dueño de los Shipment: los recibe por @c adopt y los destruye en su destructor.
     * - El árbol binario de búsqueda y el B+ guardan punteros NO propietarios y liberan sólo sus nodos.
     * Los envíos nunca se eliminan mientras el sistema vive, por lo que ninguna referencia queda
     * colgando en pendientes, lotes, colas, pilas o conjuntos.
     */
    class ShipmentCatalog final
    {
    public:
        ShipmentCatalog() = default;

        ~ShipmentCatalog();

        ShipmentCatalog(const ShipmentCatalog&) = delete;
        ShipmentCatalog& operator=(const ShipmentCatalog&) = delete;

        /**
         * @brief Toma ownership de @p pShipment y lo indexa en la lista, el BST y el B+.
         * @throws BusinessError (Conflict) si el código ya existe; en ese caso libera @p pShipment.
         */
        Shipment& adopt(Shipment* pShipment);

        /// @brief Indica si el código ya fue registrado (consulta el B+). O(log n).
        bool contains(const std::string& code) const;

        /**
         * @brief Envío por código usando el B+. O(log n).
         * @throws BusinessError (NotFound) si no existe.
         */
        Shipment& require(const std::string& code) const;

        /// @brief Visita los envíos en orden de alta.
        template<typename TVisitor>
        void forEach(TVisitor visit) const
        {
            m_shipments.forEach([&visit](Shipment* pShipment) { visit(*pShipment); });
        }

        const ds::SinglyLinkedList<Shipment*>& allShipments() const noexcept;
        const ShipmentBst& bst() const noexcept;
        const ShipmentBPlusTree& bPlusTree() const noexcept;
        std::size_t size() const noexcept;

    private:
        ds::SinglyLinkedList<Shipment*> m_shipments;
        ShipmentBst m_bst;
        ShipmentBPlusTree m_bPlusTree;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_CATALOG_HPP
