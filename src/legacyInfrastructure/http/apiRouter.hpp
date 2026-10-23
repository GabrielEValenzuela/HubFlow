#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_API_ROUTER_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_API_ROUTER_HPP

#include "application/identity/sessionCommands.hpp"
#include "infrastructure/http/httpTypes.hpp"

#include <mutex>

namespace hubflow::infrastructure
{

/// @brief Si un endpoint exige sesión.
enum class Access
{
    Public,
    Authenticated
};

/**
 * @brief Registra endpoints bajo @c /api/v1 y aplica las políticas transversales:
 *
 * - Autenticación Bearer (JWT) mediante el caso de uso Authenticate.
 * - Serialización de los pedidos con un mutex: el modelo en memoria y la conexión MySQL no son
 *   thread-safe, y cada caso de uso se ejecuta de forma atómica.
 * - Traducción de errores a Problem Details (RFC 7807, @c application/problem+json).
 */
class ApiRouter final
{
public:
    static constexpr const char* PREFIX {"/api/v1"};

    ApiRouter(httplib::Server& server, const application::Authenticate& authenticate, std::mutex& mutex);

    void get(const std::string& pattern, Handler handler, Access access = Access::Authenticated);
    void post(const std::string& pattern, Handler handler, Access access = Access::Authenticated);
    void put(const std::string& pattern, Handler handler, Access access = Access::Authenticated);
    void patch(const std::string& pattern, Handler handler, Access access = Access::Authenticated);
    void remove(const std::string& pattern, Handler handler, Access access = Access::Authenticated);

    /// @brief Escribe un Problem Details en @p response.
    static void problem(httplib::Response&             response,
                        HttpStatus                     status,
                        const std::string&             code,
                        const std::string&             detail,
                        const std::vector<FieldError>& errors = {});

private:
    httplib::Server::Handler wrap(Handler handler, Access access) const;

    std::optional<application::Principal> principalOf(const httplib::Request& request, Access access) const;

    httplib::Server&                 m_server;
    const application::Authenticate& m_authenticate;
    std::mutex&                      m_mutex;
};

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_API_ROUTER_HPP
