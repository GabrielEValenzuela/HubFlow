#include "hubflow/logistics/domain/entities/shipmentGroup.hpp"

#include "hubflow/logistics/domain/events/shipmentGroupEvents.hpp"
#include "hubflow/logistics/domain/valueobjects/deliveryPoint.hpp"
#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"
#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::logistics::domain
{
    namespace
    {
        constexpr std::size_t MIN_MEMBERS {2};
        constexpr std::size_t MAX_MEMBERS {20};
    } // namespace

    ShipmentGroup::ShipmentGroup(const ShipmentGroupState& state)
        : m_id {state.id}
        , m_deliveryPoint {DeliveryPoint {state.deliveryPoint}.nodeId()}
    {
        if (state.codes.size() < MIN_MEMBERS || state.codes.size() > MAX_MEMBERS)
        {
            throw shared::domain::BusinessError {shared::domain::ErrorKind::Validation,
                                                 "shipment_group.size_invalid",
                                                 "Un grupo necesita entre 2 y 20 paquetes"};
        }
        for (std::size_t codeIndex = 0; codeIndex < state.codes.size(); ++codeIndex)
        {
            const std::string code {TrackingCode {state.codes[codeIndex]}.value()};
            if (includes(code))
            {
                throw shared::domain::BusinessError {shared::domain::ErrorKind::Validation,
                                                     "shipment_group.duplicate_member",
                                                     "Un paquete no puede estar dos veces en el mismo grupo"};
            }
            m_codes.pushBack(code);
        }
    }

    ShipmentGroup* ShipmentGroup::form(const ShipmentGroupState& state, const shared::domain::ActorContext& context)
    {
        auto* pGroup = new ShipmentGroup {state};
        pGroup->raise(new ShipmentGroupFormed {pGroup->state(), context});
        return pGroup;
    }

    ShipmentGroup* ShipmentGroup::restore(const ShipmentGroupState& state)
    {
        return new ShipmentGroup {state};
    }

    void ShipmentGroup::dissolve(const shared::domain::ActorContext& context)
    {
        raise(new ShipmentGroupDissolved {state(), context});
    }

    bool ShipmentGroup::includes(const std::string& code) const noexcept
    {
        bool found {false};
        for (std::size_t i = 0; i < m_codes.size() && !found; ++i)
        {
            found = m_codes[i] == code;
        }
        return found;
    }

    const std::string& ShipmentGroup::id() const noexcept
    {
        return m_id.value();
    }

    const ds::DynamicArray<std::string>& ShipmentGroup::codes() const noexcept
    {
        return m_codes;
    }

    const std::string& ShipmentGroup::deliveryPoint() const noexcept
    {
        return m_deliveryPoint;
    }

    ShipmentGroupState ShipmentGroup::state() const
    {
        return ShipmentGroupState {id(), m_codes, m_deliveryPoint};
    }

} // namespace hubflow::logistics::domain
