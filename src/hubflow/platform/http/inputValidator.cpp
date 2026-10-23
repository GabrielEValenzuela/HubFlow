#include "hubflow/platform/http/inputValidator.hpp"

#include "hubflow/logistics/domain/valueobjects/catalogCode.hpp"
#include "hubflow/logistics/domain/valueobjects/trackingCode.hpp"

#include <regex>
#include <utility>

namespace hubflow::platform
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
            std::size_t  consumed {0};
            const auto&  text = value.get_ref<const std::string&>();
            const double parsed {std::stod(text, &consumed)};
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
    : m_source(std::move(source))
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

std::string
InputValidator::requiredText(const std::string& field, const std::size_t minLength, const std::size_t maxLength)
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
            fail(field,
                 "Debe tener entre " + std::to_string(minLength) + " y " + std::to_string(maxLength) + " caracteres");
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
    const std::string code {requiredText(field, 8, logistics::domain::TrackingCode::PUBLIC_LENGTH)};
    if (!code.empty() && !logistics::domain::TrackingCode::isValid(code))
    {
        fail(field, "No es un código de seguimiento válido: revisá que esté bien escrito");
    }
    return code;
}

std::vector<std::string>
InputValidator::trackingCodes(const std::string& field, const std::size_t minimum, const std::size_t maximum)
{
    std::vector<std::string> codes;
    if (!has(field) || !m_source.at(field).is_array() || m_source.at(field).size() < minimum ||
        m_source.at(field).size() > maximum)
    {
        fail(field,
             "Debe ser una lista de entre " + std::to_string(minimum) + " y " + std::to_string(maximum) +
                 " códigos de seguimiento");
    }
    else
    {
        for (const auto& item : m_source.at(field))
        {
            const std::string value {item.is_string() ? item.get<std::string>() : std::string {}};
            if (!logistics::domain::TrackingCode::isValid(value))
            {
                fail(field, "Cada elemento debe ser un código de seguimiento válido");
            }
            else
            {
                codes.push_back(value);
            }
        }
    }
    return codes;
}

std::vector<logistics::domain::ParcelData> InputValidator::parcels(const std::string& field, const std::size_t maximum)
{
    std::vector<logistics::domain::ParcelData> parcels;
    if (!has(field) || !m_source.at(field).is_array() || m_source.at(field).empty() ||
        m_source.at(field).size() > maximum)
    {
        fail(field, "Debe ser una lista de entre 1 y " + std::to_string(maximum) + " bultos");
    }
    else
    {
        std::size_t position {0};
        for (const auto& item : m_source.at(field))
        {
            const std::string prefix {field + "[" + std::to_string(position++) + "]."};
            if (!item.is_object())
            {
                fail(field, "Cada bulto debe ser un objeto con peso y medidas");
                continue;
            }
            InputValidator                one {item};
            logistics::domain::ParcelData parcel;
            parcel.weightKg = one.requiredNumber("weightKg", 0.01, logistics::domain::Weight::MAX_KILOGRAMS);
            parcel.lengthCm = one.requiredNumber("lengthCm", 0.1, logistics::domain::Dimensions::MAX_CENTIMETERS);
            parcel.widthCm  = one.requiredNumber("widthCm", 0.1, logistics::domain::Dimensions::MAX_CENTIMETERS);
            parcel.heightCm = one.requiredNumber("heightCm", 0.1, logistics::domain::Dimensions::MAX_CENTIMETERS);
            for (const auto& error : one.m_errors)
            {
                fail(prefix + error.field, error.message);
            }
            parcels.push_back(parcel);
        }
    }
    return parcels;
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

std::string InputValidator::code(const std::string& field)
{
    const std::string value {requiredText(field, 2, 16)};
    if (!value.empty() && !logistics::domain::CatalogCode::isValid(value))
    {
        fail(field, "Debe tener entre 2 y 16 caracteres: mayúsculas, números o guion bajo");
    }
    return value;
}

std::vector<std::string> InputValidator::optionalCodes(const std::string& field)
{
    std::vector<std::string> codes;
    if (has(field) && !m_source.at(field).is_array())
    {
        fail(field, "Debe ser una lista de códigos");
    }
    else if (has(field))
    {
        for (const auto& item : m_source.at(field))
        {
            const std::string value {item.is_string() ? item.get<std::string>() : std::string {}};
            if (!logistics::domain::CatalogCode::isValid(value))
            {
                fail(field, "Cada código debe tener entre 2 y 16 caracteres: mayúsculas, números o guion bajo");
            }
            else
            {
                codes.push_back(value);
            }
        }
    }
    return codes;
}

std::vector<std::pair<std::string, double>> InputValidator::textNumberPairs(const std::string& field,
                                                                            const std::string& textKey,
                                                                            const std::size_t  maxText,
                                                                            const std::string& numberKey,
                                                                            const double       maxNumber)
{
    std::vector<std::pair<std::string, double>> pairs;
    if (!has(field) || !m_source.at(field).is_array() || m_source.at(field).empty())
    {
        fail(field, "Debe ser una lista con al menos un elemento");
    }
    else
    {
        for (const auto& item : m_source.at(field))
        {
            const bool wellFormed {item.is_object() && item.contains(textKey) && item.at(textKey).is_string() &&
                                   item.contains(numberKey) && numberFrom(item.at(numberKey)).has_value()};
            const auto text   = wellFormed ? item.at(textKey).get<std::string>() : std::string {};
            const auto number = wellFormed ? *numberFrom(item.at(numberKey)) : 0.0;
            if (!wellFormed || text.empty() || text.size() > maxText || number <= 0.0 || number > maxNumber)
            {
                fail(field, "Cada elemento necesita " + textKey + " (texto) y " + numberKey + " (número positivo)");
            }
            else
            {
                pairs.emplace_back(text, number);
            }
        }
    }
    return pairs;
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

} // namespace hubflow::platform
