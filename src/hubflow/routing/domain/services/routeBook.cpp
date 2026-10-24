#include "hubflow/routing/domain/services/routeBook.hpp"

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::routing::domain
{

    RouteBook::~RouteBook()
    {
        m_routes.forEach([](DeliveryRoute* pRoute) { delete pRoute; });
    }

    std::uint64_t RouteBook::nextIdentity() const noexcept
    {
        return m_lastId + 1;
    }

    DeliveryRoute& RouteBook::adopt(DeliveryRoute* pRoute)
    {
        m_routes.pushBack(pRoute);
        if (pRoute->id() > m_lastId)
        {
            m_lastId = pRoute->id();
        }
        return *pRoute;
    }

    DeliveryRoute& RouteBook::require(const std::uint64_t id) const
    {
        DeliveryRoute* const* ppFound =
            m_routes.findFirst([id](const DeliveryRoute* pRoute) { return pRoute->id() == id; });
        if (ppFound == nullptr)
        {
            throw shared::domain::BusinessError {
                shared::domain::ErrorKind::NotFound, "route.not_found", "Hoja de ruta inexistente"};
        }
        return **ppFound;
    }

    std::size_t RouteBook::size() const noexcept
    {
        return m_routes.size();
    }

} // namespace hubflow::routing::domain
