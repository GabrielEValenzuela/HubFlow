#include "infrastructure/http/apiRouter.hpp"

#include "domain/shared/businessError.hpp"

#include <iostream>
#include <utility>

namespace hubflow::infrastructure
{
namespace
{
using domain::ErrorKind;

constexpr const char* BEARER_PREFIX {"Bearer "};
constexpr const char* JSON_TYPE {"application/json; charset=utf-8"};
constexpr const char* PROBLEM_TYPE {"application/problem+json; charset=utf-8"};

struct KindStatus
{
    ErrorKind  kind;
    HttpStatus status;
};

struct StatusTitle
{
    HttpStatus  status;
    const char* title;
};

constexpr KindStatus KIND_STATUSES[] {{ErrorKind::Validation, HttpStatus::UnprocessableEntity},
                                      {ErrorKind::NotFound, HttpStatus::NotFound},
                                      {ErrorKind::Conflict, HttpStatus::Conflict},
                                      {ErrorKind::InvalidState, HttpStatus::Conflict},
                                      {ErrorKind::Unauthorized, HttpStatus::Unauthorized},
                                      {ErrorKind::Forbidden, HttpStatus::Forbidden}};

constexpr StatusTitle STATUS_TITLES[] {{HttpStatus::BadRequest, "Pedido mal formado"},
                                       {HttpStatus::Unauthorized, "No autenticado"},
                                       {HttpStatus::Forbidden, "Sin permiso"},
                                       {HttpStatus::NotFound, "No encontrado"},
                                       {HttpStatus::Conflict, "Conflicto con el estado actual"},
                                       {HttpStatus::UnprocessableEntity, "Entrada inválida"},
                                       {HttpStatus::InternalServerError, "Error interno"}};

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
} // namespace

ApiRouter::ApiRouter(httplib::Server& server, const application::Authenticate& authenticate, std::mutex& mutex)
    : m_server {server}
    , m_authenticate {authenticate}
    , m_mutex {mutex}
{
}

void ApiRouter::get(const std::string& pattern, Handler handler, const Access access)
{
    m_server.Get(PREFIX + pattern, wrap(std::move(handler), access));
}

void ApiRouter::post(const std::string& pattern, Handler handler, const Access access)
{
    m_server.Post(PREFIX + pattern, wrap(std::move(handler), access));
}

void ApiRouter::put(const std::string& pattern, Handler handler, const Access access)
{
    m_server.Put(PREFIX + pattern, wrap(std::move(handler), access));
}

void ApiRouter::patch(const std::string& pattern, Handler handler, const Access access)
{
    m_server.Patch(PREFIX + pattern, wrap(std::move(handler), access));
}

void ApiRouter::remove(const std::string& pattern, Handler handler, const Access access)
{
    m_server.Delete(PREFIX + pattern, wrap(std::move(handler), access));
}

void ApiRouter::problem(httplib::Response&             response,
                        const HttpStatus               status,
                        const std::string&             code,
                        const std::string&             detail,
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

std::optional<application::Principal> ApiRouter::principalOf(const httplib::Request& request, const Access access) const
{
    std::optional<application::Principal> principal;
    if (access == Access::Authenticated)
    {
        const std::string header {request.get_header_value("Authorization")};
        if (header.rfind(BEARER_PREFIX, 0) != 0)
        {
            throw domain::business_error {ErrorKind::Unauthorized, "auth.missing_token", "Falta el token de sesión"};
        }
        principal = m_authenticate.execute(header.substr(std::char_traits<char>::length(BEARER_PREFIX)));
    }
    return principal;
}

httplib::Server::Handler ApiRouter::wrap(Handler handler, const Access access) const
{
    return [this, handler = std::move(handler), access](const httplib::Request& request, httplib::Response& response)
    {
        const std::lock_guard<std::mutex> lock {m_mutex};
        try
        {
            const RequestContext context {request, principalOf(request, access)};
            const HttpResponse   result = handler(context);
            response.status             = static_cast<int>(result.status);
            for (const auto& [name, value] : result.headers)
            {
                response.set_header(name, value);
            }
            if (result.status != HttpStatus::NoContent)
            {
                response.set_content(result.body.dump(), JSON_TYPE);
            }
        }
        catch (const validation_error& error)
        {
            problem(response, error.status(), "request.invalid", "Revisá los campos indicados", error.errors());
        }
        catch (const domain::business_error& error)
        {
            problem(response, statusOf(error.kind()), error.code(), error.what());
        }
        catch (const std::exception& error)
        {
            std::cerr << "[hubflow] error no controlado en " << request.method << ' ' << request.path << ": "
                      << error.what() << '\n';
            problem(response, HttpStatus::InternalServerError, "server.error", "Ocurrió un error inesperado");
        }
    };
}

} // namespace hubflow::infrastructure
