#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_GROUP_BOOK_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_GROUP_BOOK_HPP

#include "datastructures/singlyLinkedList.hpp"
#include "hubflow/logistics/domain/entities/shipmentGroup.hpp"
#include "hubflow/logistics/domain/services/shipmentCatalog.hpp"

namespace hubflow::logistics::domain
{

/**
 * @brief Registro de los grupos de envíos que viajan juntos y reglas para formarlos y deshacerlos.
 *
 * Es dueño de los grupos: los recibe por @c adopt o @c form y los destruye en su destructor o al descartarlos. Un
 * envío pertenece a lo sumo a un grupo.
 *
 * Reglas para formar un grupo:
 * - todos los códigos existen en el catálogo;
 * - todos los envíos comparten el punto de entrega, porque el grupo se reparte en una sola parada;
 * - todos esperan despacho (ninguno está en reparto ni entregado);
 * - ninguno pertenece ya a otro grupo.
 *
 * Un grupo no se puede deshacer mientras alguno de sus envíos está en reparto.
 */
class ShipmentGroupBook final
{
public:
    ShipmentGroupBook() = default;

    ~ShipmentGroupBook();

    ShipmentGroupBook(const ShipmentGroupBook&)            = delete;
    ShipmentGroupBook& operator=(const ShipmentGroupBook&) = delete;

    /**
     * @brief Verifica las reglas, crea el grupo con identidad @p id y lo incorpora.
     * @throws business_error NotFound (código inexistente), Conflict (ya agrupado, no espera despacho) o Validation
     * (punto de entrega distinto, menos de dos envíos).
     */
    ShipmentGroup& form(const std::string&                   id,
                        const ds::DynamicArray<std::string>& codes,
                        const ShipmentCatalog&               catalog,
                        const shared::domain::ActorContext&  context);

    /// @brief Incorpora un grupo ya persistido. Toma ownership.
    ShipmentGroup& adopt(ShipmentGroup* pGroup);

    /// @throws business_error (NotFound) si no existe el grupo.
    ShipmentGroup& require(const std::string& id) const;

    /// @brief Grupo al que pertenece el envío con código @p code, o nullptr.
    ShipmentGroup* groupOf(const std::string& code) const;

    /// @throws business_error (Conflict) si algún envío del grupo está en reparto.
    void ensureDissolvable(const ShipmentGroup& group, const ShipmentCatalog& catalog) const;

    /// @brief Quita y destruye el grupo. Antes hay que haber publicado sus eventos.
    void discard(const std::string& id);

    template <typename TVisitor>
    void forEach(TVisitor visit) const
    {
        m_groups.forEach(
            [&visit](ShipmentGroup* pGroup)
            {
                visit(*pGroup);
            });
    }

    std::size_t size() const noexcept;

private:
    ds::SinglyLinkedList<ShipmentGroup*> m_groups;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_SHIPMENT_GROUP_BOOK_HPP
