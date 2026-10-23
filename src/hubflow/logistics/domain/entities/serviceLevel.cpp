#include "hubflow/logistics/domain/entities/serviceLevel.hpp"

#include "hubflow/logistics/domain/events/catalogEvents.hpp"
#include "hubflow/logistics/domain/valueobjects/catalogCode.hpp"
#include "hubflow/logistics/domain/valueobjects/catalogName.hpp"
#include "hubflow/shared/domain/businessError.hpp"

#include <utility>

namespace hubflow::logistics::domain
{
namespace
{
constexpr unsigned MIN_PRIORITY {1};
constexpr unsigned MAX_PRIORITY {1000};

unsigned validPriority(const unsigned priority)
{
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY)
    {
        throw shared::domain::business_error {shared::domain::ErrorKind::Validation,
                                              "service_level.priority_invalid",
                                              "La prioridad debe estar entre 1 y 1000"};
    }
    return priority;
}
} // namespace

ServiceLevel::ServiceLevel(ServiceLevelState state)
    : m_state {std::move(state)}
{
}

ServiceLevel* ServiceLevel::open(const std::string&                  code,
                                 const std::string&                  name,
                                 const unsigned                      priority,
                                 const shared::domain::ActorContext& context)
{
    ServiceLevel* pService = new ServiceLevel {
        ServiceLevelState {CatalogCode {code}.value(), CatalogName {name}.value(), validPriority(priority), true}};
    pService->raise(new ServiceLevelChanged {pService->m_state, true, context});
    return pService;
}

ServiceLevel* ServiceLevel::restore(const ServiceLevelState& state)
{
    return new ServiceLevel {state};
}

void ServiceLevel::revise(const std::optional<std::string>&   name,
                          const std::optional<bool>&          active,
                          const PlantOccupancy&               occupancy,
                          const shared::domain::ActorContext& context)
{
    ServiceLevelState next {m_state};
    if (name.has_value())
    {
        next.name = CatalogName {*name}.value();
    }
    if (active.has_value() && *active != m_state.active)
    {
        if (!*active && occupancy.hasShipmentsWithService(m_state.code))
        {
            throw shared::domain::business_error {
                shared::domain::ErrorKind::Conflict,
                "service_level.has_shipments",
                "Hay paquetes en la planta con este servicio: despachalos antes de desactivarlo"};
        }
        next.active = *active;
    }
    if (next.name != m_state.name || next.active != m_state.active)
    {
        m_state = next;
        raise(new ServiceLevelChanged {m_state, false, context});
    }
}

const std::string& ServiceLevel::code() const noexcept
{
    return m_state.code;
}

const std::string& ServiceLevel::name() const noexcept
{
    return m_state.name;
}

unsigned ServiceLevel::priority() const noexcept
{
    return m_state.priority;
}

bool ServiceLevel::active() const noexcept
{
    return m_state.active;
}

const ServiceLevelState& ServiceLevel::state() const noexcept
{
    return m_state;
}

} // namespace hubflow::logistics::domain
