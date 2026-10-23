#ifndef HUBFLOW_INFRASTRUCTURE_HTTP_HTTP_TYPES_HPP
#define HUBFLOW_INFRASTRUCTURE_HTTP_HTTP_TYPES_HPP

#include "application/shared/principal.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hubflow::infrastructure
{

using Json = nlohmann::json;

/// @brief Status HTTP usados por la API.
enum class HttpStatus
{
    Ok                  = 200,
    Created             = 201,
    NoContent           = 204,
    BadRequest          = 400,
    Unauthorized        = 401,
    Forbidden           = 403,
    NotFound            = 404,
    Conflict            = 409,
    UnprocessableEntity = 422,
    InternalServerError = 500
};

/// @brief Respuesta de un handler: status, cuerpo JSON y cabeceras adicionales.
struct HttpResponse
{
    HttpStatus                         status {HttpStatus::Ok};
    Json                               body;
    std::map<std::string, std::string> headers;

    static HttpResponse ok(Json body);
    static HttpResponse created(Json body, const std::string& location);
    static HttpResponse attachment(Json body, const std::string& fileName);
};

/// @brief Pedido ya autenticado que recibe un handler.
struct RequestContext
{
    const httplib::Request&               request;
    std::optional<application::Principal> principal;

    /// @brief Principal autenticado (los handlers protegidos siempre lo tienen).
    const application::Principal& user() const;

    /// @brief Parámetro de ruta (@c :nombre) o cadena vacía.
    std::string path(const std::string& name) const;

    /// @brief Parámetro de query o vacío.
    std::optional<std::string> query(const std::string& name) const;
};

/// @brief Error de un campo de entrada.
struct FieldError
{
    std::string field;
    std::string message;
};

/// @brief Entrada inválida: se responde 422 (o 400 si el cuerpo no es JSON) con los campos.
class validation_error final : public std::runtime_error
{
public:
    validation_error(HttpStatus status, std::vector<FieldError> errors);

    HttpStatus                     status() const noexcept;
    const std::vector<FieldError>& errors() const noexcept;

private:
    HttpStatus              m_status;
    std::vector<FieldError> m_errors;
};

using Handler = std::function<HttpResponse(const RequestContext&)>;

} // namespace hubflow::infrastructure

#endif // HUBFLOW_INFRASTRUCTURE_HTTP_HTTP_TYPES_HPP
