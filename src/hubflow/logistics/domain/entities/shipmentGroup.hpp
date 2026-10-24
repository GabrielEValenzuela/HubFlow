#ifndef HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_GROUP_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_GROUP_HPP

#include "datastructures/dynamicArray.hpp"
#include "hubflow/logistics/domain/valueobjects/shipmentGroupId.hpp"
#include "hubflow/shared/domain/aggregateRoot.hpp"

#include <string>

namespace hubflow::logistics::domain
{

    /// @brief Estado de un grupo: su identidad, los códigos de seguimiento de los envíos que viajan juntos y el punto
    /// de entrega que comparten.
    struct ShipmentGroupState
    {
        std::string id;
        ds::DynamicArray<std::string> codes;
        std::string deliveryPoint;
    };

    /**
     * @brief Agregado "envíos que viajan juntos": dos o más envíos del mismo pedido o destinatario que deben cargarse,
     * repartirse y reprogramarse como una unidad.
     *
     * El grupo sólo conoce a sus miembros por código de seguimiento. Las reglas que dependen de los envíos (que
     * existan, que compartan el punto de entrega y que ninguno esté ya en otro grupo) las verifica ShipmentGroupBook
     * antes de crearlo.
     */
    class ShipmentGroup final : public shared::domain::AggregateRoot
    {
    public:
        /**
         * @brief Fábrica: crea el grupo y registra ShipmentGroupFormed.
         * @throws BusinessError (Validation) si el identificador es inválido, hay menos de dos envíos o hay códigos
         * repetidos.
         */
        static ShipmentGroup* form(const ShipmentGroupState& state, const shared::domain::ActorContext& context);

        /// @brief Reconstituye un grupo persistido sin generar eventos.
        static ShipmentGroup* restore(const ShipmentGroupState& state);

        /// @brief Registra ShipmentGroupDissolved; quien lo disuelve debe luego quitarlo de la agenda de grupos.
        void dissolve(const shared::domain::ActorContext& context);

        bool includes(const std::string& code) const noexcept;

        const std::string& id() const noexcept;
        const ds::DynamicArray<std::string>& codes() const noexcept;
        const std::string& deliveryPoint() const noexcept;
        ShipmentGroupState state() const;

    private:
        explicit ShipmentGroup(const ShipmentGroupState& state);

        ShipmentGroupId m_id;
        ds::DynamicArray<std::string> m_codes;
        std::string m_deliveryPoint;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_ENTITIES_SHIPMENT_GROUP_HPP
