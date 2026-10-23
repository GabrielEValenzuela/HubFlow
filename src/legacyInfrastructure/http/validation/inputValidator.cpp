#include "infrastructure/http/validation/inputValidator.hpp"

#include "domain/logistics/valueobjects/trackingCode.hpp"

#include <regex>
#include <utility>

namespace hubflow::infrastructure
{
namespace
{
constexpr std::size_t DEFAULT_PAGE_SIZE {50};
constexpr double      MAX_PAGE_SIZE {200};
constexpr double      MAX_OFFSET {1e9};
constexpr double      MAX_IDENTIFIER {9e15};

const std::regex EMAIL_PATTERN {R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)"};

std::optional<double> numberFrom(const Json& value)
{
    std::optional<double> number;
    if (value.is_number())
    {
        number = value.get<double>();
    }
    else if (value.is_string())
    {
        try
        {
            std::size_t   consumed {0};
            const auto&   text = value.get_ref<const std::string&>();
            const double  parsed {std::stod(text, &consumed)};
            if (consumed == text.size())
            {
                number = parsed;
            }
        }
        catch (const std::exception&)
        {
            number.reset();
        }
    }
    return number;
}
} // namespace

InputValidator::InputValidator(Json source)
    : m_source {std::move(source)}
{
}

InputValidator InputValidator::body(const httplib::Request& request)
{
    Json parsed = Json::parse(request.body.empty() ? "{}" : request.body, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object())
    {
        throw validation_error {HttpStatus::BadRequest, {{"body", "El cuerpo debe ser un objeto JSON válido"}}};
    }
    return InputValidator {std::move(parsed)};
}

InputValidator InputValidator::parameters(const RequestContext& context)
{
    Json source = Json::object();
    for (const auto& [name, value] : context.request.params)
    {
        source[name] = value;
    }
    for (const auto& [name, value] : context.request.path_params)
    {
        source[name] = value;
    }
    return InputValidator {std::move(source)};
}

bool InputValidator::has(const std::string& field) const
{
    return m_source.contains(field) && !m_source.at(field).is_null();
}

void InputValidator::fail(const std::string& field, const std::string& message)
{
    m_errors.push_back(FieldError {field, message});
}

std::string InputValidator::requiredText(const std::string& field, const std::size_t minLength, const std::size_t maxLength)
{
    std::string text;
    if (!has(field) || !m_source.at(field).is_string())
    {
        fail(field, "Es obligatorio y debe ser texto");
    }
    else
    {
        text = m_source.at(field).get<std::string>();
        if (text.size() < minLength || text.size() > maxLength)
        {
            fail(field, "Debe tener entre " + std::to_string(minLength) + " y " + std::to_string(maxLength) + " caracteres");
        }
    }
    return text;
}

std::optional<std::string> InputValidator::optionalText(const std::string& field, const std::size_t maxLength)
{
    std::optional<std::string> text;
    if (has(field))
    {
        text = requiredText(field, 0, maxLength);
    }
    return text;
}

double InputValidator::requiredNumber(const std::string& field, const double minimum, const double maximum)
{
    const auto number = has(field) ? numberFrom(m_source.at(field)) : std::nullopt;
    if (!number.has_value() || *number < minimum || *number > maximum)
    {
        fail(field, "Debe ser un número entre " + std::to_string(minimum) + " y " + std::to_string(maximum));
    }
    return number.value_or(minimum);
}

std::uint64_t InputValidator::requiredIdentifier(const std::string& field)
{
    const double value {requiredNumber(field, 1, MAX_IDENTIFIER)};
    if (value != static_cast<double>(static_cast<std::uint64_t>(value)))
    {
        fail(field, "Debe ser un entero positivo");
    }
    return static_cast<std::uint64_t>(value);
}

std::optional<bool> InputValidator::optionalFlag(const std::string& field)
{
    std::optional<bool> flag;
    if (has(field) && !m_source.at(field).is_boolean())
    {
        fail(field, "Debe ser true o false");
    }
    else if (has(field))
    {
        flag = m_source.at(field).get<bool>();
    }
    return flag;
}

std::string InputValidator::trackingCode(const std::string& field)
{
    const std::string code {requiredText(field, 8, 8)};
    if (!code.empty() && !domain::logistics::TrackingCode::isValid(code))
    {
        fail(field, "Debe tener el formato PKG-NNNN");
    }
    return code;
}

std::string InputValidator::email(const std::string& field)
{
    const std::string address {requiredText(field, 5, 120)};
    if (!address.empty() && !std::regex_match(address, EMAIL_PATTERN))
    {
        fail(field, "Debe ser un email válido");
    }
    return address;
}

std::pair<std::size_t, std::size_t> InputValidator::page()
{
    const std::size_t limit {has("limit") ? static_cast<std::size_t>(requiredNumber("limit", 1, MAX_PAGE_SIZE))
                                          : DEFAULT_PAGE_SIZE};
    const std::size_t offset {has("offset") ? static_cast<std::size_t>(requiredNumber("offset", 0, MAX_OFFSET)) : 0};
    return {limit, offset};
}

void InputValidator::validate() const
{
    if (!m_errors.empty())
    {
        throw validation_error {HttpStatus::UnprocessableEntity, m_errors};
    }
}

} // namespace hubflow::infrastructure
