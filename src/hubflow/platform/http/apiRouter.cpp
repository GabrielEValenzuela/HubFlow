#include "hubflow/platform/http/apiRouter.hpp"

#include "hubflow/shared/domain/businessError.hpp"

#include <iostream>
#include <utility>

namespace hubflow::platform
{
    namespace
    {
        using shared::domain::ErrorKind;

        constexpr const char* BEARER_PREFIX {"Bearer "};
        constexpr const char* JSON_TYPE {"application/json; charset=utf-8"};
        constexpr const char* PROBLEM_TYPE {"application/problem+json; charset=utf-8"};
        constexpr const char* KEY_FIELD {"Idempotency-Key"};
        constexpr const char* RETRY_AFTER_HEADER {"Retry-After"};
        constexpr int FIRST_SUCCESS {200};
        constexpr int LAST_SUCCESS {299};

        struct KindStatus
        {
            ErrorKind kind;
            HttpStatus status;
        };

        struct StatusTitle
        {
            HttpStatus status;
            const char* title;
        };

        constexpr KindStatus KIND_STATUSES[] {{ErrorKind::Validation, HttpStatus::UnprocessableEntity},
                                              {ErrorKind::NotFound, HttpStatus::NotFound},
                                              {ErrorKind::Conflict, HttpStatus::Conflict},
                                              {ErrorKind::InvalidState, HttpStatus::Conflict},
                                              {ErrorKind::Unauthorized, HttpStatus::Unauthorized},
                                              {ErrorKind::Forbidden, HttpStatus::Forbidden},
                                              {ErrorKind::RateLimited, HttpStatus::TooManyRequests}};

        constexpr StatusTitle STATUS_TITLES[] {{HttpStatus::BadRequest, "Pedido mal formado"},
                                               {HttpStatus::Unauthorized, "No autenticado"},
                                               {HttpStatus::Forbidden, "Sin permiso"},
                                               {HttpStatus::NotFound, "No encontrado"},
                                               {HttpStatus::MethodNotAllowed, "Método no permitido"},
                                               {HttpStatus::Conflict, "Conflicto con el estado actual"},
                                               {HttpStatus::UnprocessableEntity, "Entrada inválida"},
                                               {HttpStatus::TooManyRequests, "Demasiados intentos"},
                                               {HttpStatus::InternalServerError, "Error interno"},
                                               {HttpStatus::ServiceUnavailable, "Servicio no disponible"}};

        HttpStatus statusOf(const ErrorKind kind)
        {
            HttpStatus status {HttpStatus::UnprocessableEntity};
            for (const auto& entry : KIND_STATUSES)
            {
                if (entry.kind == kind)
                {
                    status = entry.status;
                }
            }
            return status;
        }

        std::string titleOf(const HttpStatus status)
        {
            std::string title {"Error"};
            for (const auto& entry : STATUS_TITLES)
            {
                if (entry.status == status)
                {
                    title = entry.title;
                }
            }
            return title;
        }

        void declareGet(httplib::Server& server, const std::string& pattern, httplib::Server::Handler handler)
        {
            server.Get(pattern, std::move(handler));
        }

        void declarePost(httplib::Server& server, const std::string& pattern, httplib::Server::Handler handler)
        {
            server.Post(pattern, std::move(handler));
        }

        void declarePut(httplib::Server& server, const std::string& pattern, httplib::Server::Handler handler)
        {
            server.Put(pattern, std::move(handler));
        }

        void declarePatch(httplib::Server& server, const std::string& pattern, httplib::Server::Handler handler)
        {
            server.Patch(pattern, std::move(handler));
        }

        void declareDelete(httplib::Server& server, const std::string& pattern, httplib::Server::Handler handler)
        {
            server.Delete(pattern, std::move(handler));
        }

        bool isSuccess(const HttpStatus status)
        {
            return static_cast<int>(status) >= FIRST_SUCCESS && static_cast<int>(status) <= LAST_SUCCESS;
        }
    } // namespace

    ApiRouter::ApiRouter(httplib::Server& server,
                         const identity::application::AuthenticateUseCase& authenticate,
                         shared::application::IdempotencyStore& idempotencyStore,
                         std::mutex& mutex)
        : m_server {server}
        , m_authenticate {authenticate}
        , m_idempotencyStore {idempotencyStore}
        , m_mutex {mutex}
    {
        m_server.set_error_handler(
            [](const httplib::Request& request, httplib::Response& response)
            {
                const bool isApi {request.path.rfind(PREFIX, 0) == 0};
                if (isApi && response.body.empty())
                {
                    const auto status = static_cast<HttpStatus>(response.status);
                    problem(response, status, "request.rejected", "El pedido no pudo atenderse");
                }
            });
    }

    void ApiRouter::get(const std::string& pattern, Handler handler, const Access access)
    {
        declare(declareGet, pattern, std::move(handler), access, false);
    }

    void ApiRouter::post(const std::string& pattern,
                         Handler handler,
                         const shared::application::Idempotency idempotency,
                         const Access access)
    {
        declare(declarePost,
                pattern,
                std::move(handler),
                access,
                idempotency == shared::application::Idempotency::NonIdempotent);
    }

    void ApiRouter::put(const std::string& pattern, Handler handler, const Access access)
    {
        declare(declarePut, pattern, std::move(handler), access, false);
    }

    void ApiRouter::patch(const std::string& pattern, Handler handler, const Access access)
    {
        declare(declarePatch, pattern, std::move(handler), access, false);
    }

    void ApiRouter::remove(const std::string& pattern, Handler handler, const Access access)
    {
        declare(declareDelete, pattern, std::move(handler), access, false);
    }

    void ApiRouter::declare(const Registration registration,
                            const std::string& pattern,
                            Handler handler,
                            const Access access,
                            const bool keyed)
    {
        registration(m_server, PREFIX + pattern, wrap(std::move(handler), access, keyed));
    }

    void ApiRouter::problem(httplib::Response& response,
                            const HttpStatus status,
                            const std::string& code,
                            const std::string& detail,
                            const std::vector<FieldError>& errors)
    {
        Json fieldErrors = Json::array();
        for (const auto& error : errors)
        {
            fieldErrors.push_back(Json {{"field", error.field}, {"message", error.message}});
        }
        const int statusCode {static_cast<int>(status)};
        const Json body {{"type", "https://hubflow.local/problems/" + code},
                         {"title", titleOf(status)},
                         {"status", statusCode},
                         {"detail", detail},
                         {"code", code},
                         {"errors", fieldErrors}};
        response.status = statusCode;
        response.set_content(body.dump(), PROBLEM_TYPE);
    }

    std::optional<shared::application::Principal> ApiRouter::principalOf(const httplib::Request& request,
                                                                         const Access access) const
    {
        std::optional<shared::application::Principal> principal;
        if (access == Access::Authenticated)
        {
            const std::string header {request.get_header_value("Authorization")};
            if (header.rfind(BEARER_PREFIX, 0) != 0)
            {
                throw shared::domain::business_error {
                    ErrorKind::Unauthorized, "auth.missing_token", "Falta el token de sesión"};
            }
            principal = m_authenticate.execute(header.substr(std::char_traits<char>::length(BEARER_PREFIX)));
        }
        return principal;
    }

    void ApiRouter::write(httplib::Response& response, const HttpResponse& result)
    {
        response.status = static_cast<int>(result.status);
        for (const auto& [name, value] : result.headers)
        {
            response.set_header(name, value);
        }
        if (result.status != HttpStatus::NoContent)
        {
            response.set_content(result.body.dump(), JSON_TYPE);
        }
    }

    bool ApiRouter::replay(const RequestContext& context, const std::string& key, httplib::Response& response) const
    {
        const auto stored = m_idempotencyStore.find(context.user().userId, key);
        if (stored.has_value())
        {
            if (stored->method != context.request.method || stored->path != context.request.path)
            {
                throw validation_error {HttpStatus::UnprocessableEntity,
                                        {{KEY_FIELD, "La clave ya se usó en otro método o ruta"}}};
            }
            const Json envelope = Json::parse(stored->body);
            HttpResponse replayed;
            replayed.status = static_cast<HttpStatus>(stored->status);
            replayed.body = envelope.at("body");
            if (envelope.contains("location"))
            {
                replayed.headers["Location"] = envelope.at("location").get<std::string>();
            }
            replayed.headers[REPLAY_HEADER] = "true";
            write(response, replayed);
        }
        return stored.has_value();
    }

    void ApiRouter::remember(const RequestContext& context, const std::string& key, const HttpResponse& result) const
    {
        Json envelope {{"body", result.body}};
        const auto location = result.headers.find("Location");
        if (location != result.headers.end())
        {
            envelope["location"] = location->second;
        }
        m_idempotencyStore.save(
            context.user().userId,
            key,
            shared::application::StoredResponse {
                context.request.method, context.request.path, static_cast<int>(result.status), envelope.dump()});
    }

    void ApiRouter::respond(const RequestContext& context,
                            const Handler& handler,
                            httplib::Response& response,
                            const bool keyed) const
    {
        const bool hasKey {keyed && context.request.has_header(IDEMPOTENCY_KEY_HEADER)};
        const std::string key {hasKey ? context.request.get_header_value(IDEMPOTENCY_KEY_HEADER) : std::string {}};
        if (hasKey && (key.empty() || key.size() > MAX_IDEMPOTENCY_KEY_LENGTH))
        {
            throw validation_error {HttpStatus::UnprocessableEntity,
                                    {{KEY_FIELD, "Debe tener entre 1 y 80 caracteres"}}};
        }
        const bool replayed {hasKey && replay(context, key, response)};
        if (!replayed)
        {
            const HttpResponse result = handler(context);
            write(response, result);
            if (hasKey && isSuccess(result.status))
            {
                remember(context, key, result);
            }
        }
    }

    httplib::Server::Handler ApiRouter::wrap(Handler handler, const Access access, const bool keyed) const
    {
        return [this, handler = std::move(handler), access, keyed](const httplib::Request& request,
                                                                   httplib::Response& response)
        {
            const std::lock_guard<std::mutex> lock {m_mutex};
            try
            {
                const RequestContext context {request, principalOf(request, access)};
                respond(context, handler, response, keyed && context.principal.has_value());
            }
            catch (const validation_error& error)
            {
                problem(response, error.status(), "request.invalid", "Revisá los campos indicados", error.errors());
            }
            catch (const shared::domain::business_error& error)
            {
                problem(response, statusOf(error.errorKind()), error.code(), error.what());
                if (error.retryAfterSeconds() > 0)
                {
                    response.set_header(RETRY_AFTER_HEADER, std::to_string(error.retryAfterSeconds()));
                }
            }
            catch (const std::exception& error)
            {
                std::cerr << "[hubflow] error no controlado en " << request.method << ' ' << request.path << ": "
                          << error.what() << '\n';
                problem(response, HttpStatus::InternalServerError, "server.error", "Ocurrió un error inesperado");
            }
        };
    }

} // namespace hubflow::platform
