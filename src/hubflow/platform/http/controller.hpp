#ifndef HUBFLOW_PLATFORM_HTTP_CONTROLLER_HPP
#define HUBFLOW_PLATFORM_HTTP_CONTROLLER_HPP

#include "hubflow/platform/http/apiRouter.hpp"

namespace hubflow::platform
{

/// @brief Adaptador de entrada HTTP de un contexto: traduce pedidos REST a casos de uso.
class Controller
{
public:
    virtual ~Controller() = default;

    /// @brief Declara los endpoints del controlador en el router.
    virtual void registerRoutes(ApiRouter& router) = 0;

protected:
    /// @brief Recorta una colección según @p limit y @p offset y la devuelve como {items, total, limit, offset}.
    static Json page(const Json& items, std::size_t limit, std::size_t offset);

    /// @brief Dirección absoluta de un recurso bajo el prefijo de la API.
    static std::string locationOf(const std::string& resourcePath);
};

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_HTTP_CONTROLLER_HPP
