#include "hubflow/logistics/domain/entities/shipmentFeature.hpp"

#include "hubflow/logistics/domain/events/catalogEvents.hpp"
#include "hubflow/logistics/domain/valueobjects/catalogCode.hpp"
#include "hubflow/logistics/domain/valueobjects/catalogName.hpp"

#include <utility>

namespace hubflow::logistics::domain
{

ShipmentFeature::ShipmentFeature(ShipmentFeatureState state)
    : m_state {std::move(state)}
{
}

ShipmentFeature*
ShipmentFeature::open(const std::string& code, const std::string& name, const shared::domain::ActorContext& context)
{
    ShipmentFeature* pFeature =
        new ShipmentFeature {ShipmentFeatureState {CatalogCode {code}.value(), CatalogName {name}.value(), true}};
    pFeature->raise(new ShipmentFeatureChanged {pFeature->m_state, true, context});
    return pFeature;
}

ShipmentFeature* ShipmentFeature::restore(const ShipmentFeatureState& state)
{
    return new ShipmentFeature {state};
}

void ShipmentFeature::revise(const std::optional<std::string>&   name,
                             const std::optional<bool>&          active,
                             const shared::domain::ActorContext& context)
{
    ShipmentFeatureState next {m_state};
    if (name.has_value())
    {
        next.name = CatalogName {*name}.value();
    }
    if (active.has_value())
    {
        next.active = *active;
    }
    if (next.name != m_state.name || next.active != m_state.active)
    {
        m_state = next;
        raise(new ShipmentFeatureChanged {m_state, false, context});
    }
}

const std::string& ShipmentFeature::code() const noexcept
{
    return m_state.code;
}

const std::string& ShipmentFeature::name() const noexcept
{
    return m_state.name;
}

bool ShipmentFeature::active() const noexcept
{
    return m_state.active;
}

const ShipmentFeatureState& ShipmentFeature::state() const noexcept
{
    return m_state;
}

} // namespace hubflow::logistics::domain
