#include "hubflow/logistics/domain/entities/zone.hpp"

#include "hubflow/logistics/domain/events/catalogEvents.hpp"
#include "hubflow/logistics/domain/valueobjects/catalogCode.hpp"
#include "hubflow/logistics/domain/valueobjects/catalogName.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <utility>

namespace hubflow::logistics::domain
{
    namespace
    {
        [[noreturn]] void refuse(const char* reason, const char* message)
        {
            throw shared::domain::business_error {shared::domain::ErrorKind::Conflict, reason, message};
        }
    } // namespace

    Zone::Zone(ZoneState state)
        : m_state {std::move(state)}
    {
    }

    Zone* Zone::open(const std::string& code, const std::string& name, const shared::domain::ActorContext& context)
    {
        Zone* pZone = new Zone {ZoneState {CatalogCode {code}.value(), CatalogName {name}.value(), true}};
        pZone->raise(new ZoneChanged {pZone->m_state, true, context});
        return pZone;
    }

    Zone* Zone::restore(const ZoneState& state)
    {
        return new Zone {state};
    }

    void Zone::revise(const std::optional<std::string>& name,
                      const std::optional<bool>& active,
                      const PlantOccupancy& occupancy,
                      const shared::domain::ActorContext& context)
    {
        ZoneState next {m_state};
        if (name.has_value())
        {
            next.name = CatalogName {*name}.value();
        }
        if (active.has_value() && *active != m_state.active)
        {
            if (!*active)
            {
                ensureCanClose(m_state.code, occupancy);
            }
            next.active = *active;
        }
        if (next.name != m_state.name || next.active != m_state.active)
        {
            m_state = next;
            raise(new ZoneChanged {m_state, false, context});
        }
    }

    void Zone::ensureCanClose(const std::string& code, const PlantOccupancy& occupancy)
    {
        if (occupancy.hasShipmentsInZone(code))
        {
            refuse("zone.has_shipments", "Hay paquetes en la planta para esta zona: despachalos antes de desactivarla");
        }
        if (occupancy.hasRoutesInProgressInZone(code))
        {
            refuse("zone.has_routes", "Hay repartos en curso que pasan por esta zona: esperá a que terminen");
        }
        if (occupancy.hasActiveMapNodesInZone(code))
        {
            refuse("zone.has_map_nodes", "Esta zona tiene puntos del mapa activos: pasalos a otra zona o desactivalos");
        }
    }

    const std::string& Zone::code() const noexcept
    {
        return m_state.code;
    }

    const std::string& Zone::name() const noexcept
    {
        return m_state.name;
    }

    bool Zone::isActive() const noexcept
    {
        return m_state.active;
    }

    const ZoneState& Zone::state() const noexcept
    {
        return m_state;
    }

} // namespace hubflow::logistics::domain
