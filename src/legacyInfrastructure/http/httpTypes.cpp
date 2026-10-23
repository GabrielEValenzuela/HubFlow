#include "infrastructure/http/httpTypes.hpp"

#include <utility>

namespace hubflow::infrastructure
{

HttpResponse HttpResponse::ok(Json body)
{
    return HttpResponse {HttpStatus::Ok, std::move(body), {}};
}

HttpResponse HttpResponse::created(Json body, const std::string& location)
{
    return HttpResponse {HttpStatus::Created, std::move(body), {{"Location", location}}};
}

HttpResponse HttpResponse::attachment(Json body, const std::string& fileName)
{
    return HttpResponse {
        HttpStatus::Ok, std::move(body), {{"Content-Disposition", "attachment; filename=\"" + fileName + "\""}}};
}

const application::Principal& RequestContext::user() const
{
    if (!principal.has_value())
    {
        throw std::logic_error("Handler protegido sin principal");
    }
    return *principal;
}

std::string RequestContext::path(const std::string& name) const
{
    const auto found = request.path_params.find(name);
    return found == request.path_params.end() ? std::string {} : found->second;
}

std::optional<std::string> RequestContext::query(const std::string& name) const
{
    return request.has_param(name) ? std::optional<std::string> {request.get_param_value(name)} : std::nullopt;
}

validation_error::validation_error(const HttpStatus status, std::vector<FieldError> errors)
    : std::runtime_error {"Entrada inválida"}
    , m_status {status}
    , m_errors {std::move(errors)}
{
}

HttpStatus validation_error::status() const noexcept
{
    return m_status;
}

const std::vector<FieldError>& validation_error::errors() const noexcept
{
    return m_errors;
}

} // namespace hubflow::infrastructure
