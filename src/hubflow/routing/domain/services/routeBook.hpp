#ifndef HUBFLOW_ROUTING_DOMAIN_SERVICES_ROUTE_BOOK_HPP
#define HUBFLOW_ROUTING_DOMAIN_SERVICES_ROUTE_BOOK_HPP

#include "hubflow/routing/domain/entities/deliveryRoute.hpp"

namespace hubflow::routing::domain
{

/**
 * @brief Colección en memoria de hojas de ruta. Es dueña de cada DeliveryRoute y emite sus
 * identidades.
 */
class RouteBook final
{
public:
    RouteBook() = default;

    ~RouteBook();

    RouteBook(const RouteBook&)            = delete;
    RouteBook& operator=(const RouteBook&) = delete;

    /// @brief Próxima identidad de ruta.
    std::uint64_t nextIdentity() noexcept;

    /// @brief Toma ownership de @p pRoute.
    DeliveryRoute& adopt(DeliveryRoute* pRoute);

    /// @throws business_error (NotFound) si no existe.
    DeliveryRoute& require(std::uint64_t id) const;

    /// @brief Visita las rutas en orden de creación.
    template <typename TVisitor>
    void forEach(TVisitor visit) const
    {
        m_routes.forEach(
            [&visit](DeliveryRoute* pRoute)
            {
                visit(*pRoute);
            });
    }

    std::size_t size() const noexcept;

private:
    ds::SinglyLinkedList<DeliveryRoute*> m_routes;
    std::uint64_t                        m_lastId {0};
};

} // namespace hubflow::routing::domain

#endif // HUBFLOW_ROUTING_DOMAIN_SERVICES_ROUTE_BOOK_HPP
