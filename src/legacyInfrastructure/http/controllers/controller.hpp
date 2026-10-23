#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_CONTROLLER_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_CONTROLLER_HPP

#include "infrastructure/http/apiRouter.hpp"

namespace hubflow::infrastructure
{

/// @brief Adaptador de entrada HTTP de un contexto: traduce pedidos REST a casos de uso.
class Controller
{
public:
    virtual ~Controller() = default;

    /// @brief Registra sus endpoints en el router.
    virtual void registerRoutes(ApiRouter& router) = 0;

protected:
    /// @brief Envuelve una colección en una página {items, total, limit, offset}.
    static Json page(const Json& items, std::size_t limit, std::size_t offset);
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_CONTROLLER_HPP
