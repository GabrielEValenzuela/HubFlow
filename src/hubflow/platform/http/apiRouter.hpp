#ifndef HUBFLOW_PLATFORM_HTTP_API_ROUTER_HPP
#define HUBFLOW_PLATFORM_HTTP_API_ROUTER_HPP

#include "hubflow/identity/application/ports/in/authenticateUseCase.hpp"
#include "hubflow/platform/http/httpTypes.hpp"
#include "hubflow/shared/application/ports/out/idempotencyStore.hpp"
#include "hubflow/shared/application/useCase.hpp"
#include <cstdint>

#include <mutex>

namespace hubflow::platform
{

    /// @brief Si un endpoint exige sesión iniciada.
    enum class Access : std::uint8_t
    {
        Public,
        Authenticated
    };

    /**
     * @brief Registra los endpoints bajo @c /api/v1 y aplica las políticas comunes a todos.
     *
     * - Autenticación Bearer (JWT) mediante el caso de uso de autenticación.
     * - Un único candado serializa los pedidos: el modelo en memoria y la conexión a la base de datos
     *   no admiten acceso simultáneo y cada caso de uso debe ejecutarse completo.
     * - Los errores de negocio y de entrada se traducen a Problem Details (@c application/problem+json).
     * - Reintentos seguros: en un POST declarado como no idempotente, si el cliente envía la cabecera
     *   @c Idempotency-Key (1 a 80 caracteres), la primera respuesta exitosa se recuerda por usuario y
     *   clave; repetir el mismo pedido devuelve esa respuesta con @c Idempotent-Replayed: true sin volver a
     *   ejecutarlo, y reutilizar la clave en otro método o ruta se rechaza con 422.
     *
     * - Límite de intentos: un error de negocio de tipo RateLimited se responde 429 con la cabecera @c Retry-After.
     * - Los endpoints públicos (@c Access::Public) no tienen usuario: no usan @c Idempotency-Key aunque declaren un
     * POST no idempotente, y cada caso de uso documenta cómo se protege.
     *
     * La idempotencia de cada endpoint se toma de la constante @c IDEMPOTENCY de su caso de uso, de modo
     * que el contrato HTTP deriva del contrato del caso de uso.
     */
    class ApiRouter final
    {
    public:
        static constexpr const char* PREFIX {"/api/v1"};
        static constexpr const char* IDEMPOTENCY_KEY_HEADER {"Idempotency-Key"};
        static constexpr const char* REPLAY_HEADER {"Idempotent-Replayed"};
        static constexpr std::size_t MAX_IDEMPOTENCY_KEY_LENGTH {80};

        ApiRouter(httplib::Server& server,
                  const identity::application::AuthenticateUseCase& authenticate,
                  shared::application::IdempotencyStore& idempotencyStore,
                  std::mutex& mutex);

        /// @brief Declara un GET.
        void get(const std::string& pattern, Handler handler, Access access = Access::Authenticated);

        /// @brief Declara un POST; @p idempotency decide si acepta @c Idempotency-Key.
        void post(const std::string& pattern,
                  Handler handler,
                  shared::application::Idempotency idempotency,
                  Access access = Access::Authenticated);

        /// @brief Declara un PUT.
        void put(const std::string& pattern, Handler handler, Access access = Access::Authenticated);

        /// @brief Declara un PATCH.
        void patch(const std::string& pattern, Handler handler, Access access = Access::Authenticated);

        /// @brief Declara un DELETE.
        void remove(const std::string& pattern, Handler handler, Access access = Access::Authenticated);

        /// @brief Escribe un Problem Details en @p response.
        static void problem(httplib::Response& response,
                            HttpStatus status,
                            const std::string& code,
                            const std::string& detail,
                            const std::vector<FieldError>& errors = {});

    private:
        using Registration = void (*)(httplib::Server&, const std::string&, httplib::Server::Handler);

        void declare(Registration registration, const std::string& pattern, Handler handler, Access access, bool keyed);

        httplib::Server::Handler wrap(Handler handler, Access access, bool keyed) const;

        void
        respond(const RequestContext& context, const Handler& handler, httplib::Response& response, bool keyed) const;

        bool replay(const RequestContext& context, const std::string& key, httplib::Response& response) const;

        void remember(const RequestContext& context, const std::string& key, const HttpResponse& result) const;

        static void write(httplib::Response& response, const HttpResponse& result);

        std::optional<shared::application::Principal> principalOf(const httplib::Request& request, Access access) const;

        httplib::Server& m_server;
        const identity::application::AuthenticateUseCase& m_authenticate;
        shared::application::IdempotencyStore& m_idempotencyStore;
        std::mutex& m_mutex;
    };

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_HTTP_API_ROUTER_HPP
