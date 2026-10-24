#ifndef HUBFLOW_PLATFORM_HTTP_HTTP_TYPES_HPP
#define HUBFLOW_PLATFORM_HTTP_HTTP_TYPES_HPP

#include "hubflow/shared/application/principal.hpp"

#include <functional>
#include <httplib.h>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hubflow::platform
{

    using Json = nlohmann::json;

    /// @brief Códigos de estado HTTP que usa la API.
    enum class HttpStatus
    {
        Ok = 200,
        Created = 201,
        NoContent = 204,
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        Conflict = 409,
        UnprocessableEntity = 422,
        TooManyRequests = 429,
        InternalServerError = 500,
        ServiceUnavailable = 503
    };

    /// @brief Respuesta de un endpoint: estado, cuerpo JSON y cabeceras adicionales.
    struct HttpResponse
    {
        HttpStatus status {HttpStatus::Ok};
        Json body;
        std::map<std::string, std::string> headers;

        /// @brief Respuesta 200 con el cuerpo indicado.
        static HttpResponse ok(Json body);

        /// @brief Respuesta 201 que apunta con @c Location al recurso creado.
        static HttpResponse created(Json body, const std::string& location);

        /// @brief Respuesta 200 que el navegador ofrece descargar como archivo.
        static HttpResponse attachment(Json body, const std::string& fileName);
    };

    /// @brief Pedido ya autenticado que recibe un endpoint.
    struct RequestContext
    {
        const httplib::Request& request;
        std::optional<shared::application::Principal> principal;

        /// @brief Usuario autenticado; los endpoints protegidos siempre lo tienen.
        const shared::application::Principal& user() const;
    };

    /// @brief Error de un campo de entrada.
    struct FieldError
    {
        std::string field;
        std::string message;
    };

    /// @brief Entrada inválida: se responde 422 (o 400 si el cuerpo no es JSON) detallando cada campo.
    class validation_error final : public std::runtime_error
    {
    public:
        validation_error(HttpStatus status, std::vector<FieldError> errors);

        HttpStatus status() const noexcept;

        const std::vector<FieldError>& errors() const noexcept;

    private:
        HttpStatus m_status;
        std::vector<FieldError> m_errors;
    };

    /// @brief Lógica de un endpoint.
    using Handler = std::function<HttpResponse(const RequestContext&)>;

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_HTTP_HTTP_TYPES_HPP
