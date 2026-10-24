#include "hubflow/platform/http/httpTypes.hpp"

#include <utility>

namespace hubflow::platform
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

    const shared::application::Principal& RequestContext::user() const
    {
        if (!principal.has_value())
        {
            throw std::logic_error("Protected endpoint without an authenticated user");
        }
        return *principal;
    }

    validation_error::validation_error(const HttpStatus status, std::vector<FieldError> errors)
        : std::runtime_error {"Invalid input"}
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

} // namespace hubflow::platform
