#ifndef HUBFLOW_PLATFORM_HTTP_INPUT_VALIDATOR_HPP
#define HUBFLOW_PLATFORM_HTTP_INPUT_VALIDATOR_HPP

#include "hubflow/platform/http/httpTypes.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hubflow::platform
{

/**
 * @brief Primera barrera de validación de la API: tipos, obligatoriedad, rangos, formatos y
 * enumerados. Acumula todos los errores y los informa juntos con @c validate().
 *
 * La segunda barrera son los Value Objects del dominio, que vuelven a validar sus invariantes.
 */
class InputValidator final
{
public:
    /// @brief Valida el cuerpo JSON del pedido. @throws validation_error (400) si no es un objeto JSON.
    static InputValidator body(const httplib::Request& request);

    /// @brief Valida los parámetros de query y de ruta del pedido.
    static InputValidator parameters(const RequestContext& context);

    std::string                requiredText(const std::string& field, std::size_t minLength, std::size_t maxLength);
    std::optional<std::string> optionalText(const std::string& field, std::size_t maxLength);
    double                     requiredNumber(const std::string& field, double minimum, double maximum);
    std::uint64_t              requiredIdentifier(const std::string& field);
    std::optional<bool>        optionalFlag(const std::string& field);
    std::string                trackingCode(const std::string& field);
    std::string                email(const std::string& field);

    /**
     * @brief Valida que el campo sea uno de los valores admitidos por @p parse.
     * @param allowed lista legible de valores (para el mensaje de error).
     */
    template <typename Enum, typename Parser>
    Enum requiredTerm(const std::string& field, Parser parse, const std::string& allowed)
    {
        const std::string   text {requiredText(field, 1, 32)};
        std::optional<Enum> parsed {parse(text)};
        if (!parsed.has_value() && !text.empty())
        {
            fail(field, "Debe ser uno de: " + allowed);
        }
        return parsed.value_or(Enum {});
    }

    /// @brief Variante opcional de @c requiredTerm.
    template <typename Enum, typename Parser>
    std::optional<Enum> optionalTerm(const std::string& field, Parser parse, const std::string& allowed)
    {
        std::optional<Enum> result;
        if (has(field))
        {
            result = requiredTerm<Enum>(field, parse, allowed);
        }
        return result;
    }

    /// @brief Paginación @c limit (1..200, por defecto 50) y @c offset (>= 0).
    std::pair<std::size_t, std::size_t> page();

    bool has(const std::string& field) const;

    /// @throws validation_error (422) si se acumuló algún error.
    void validate() const;

private:
    explicit InputValidator(Json source);

    void fail(const std::string& field, const std::string& message);

    Json                    m_source;
    std::vector<FieldError> m_errors;
};

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_HTTP_INPUT_VALIDATOR_HPP
