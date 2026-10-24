#ifndef HUBFLOW_PLATFORM_HTTP_INPUT_VALIDATOR_HPP
#define HUBFLOW_PLATFORM_HTTP_INPUT_VALIDATOR_HPP

#include "hubflow/logistics/domain/entities/parcel.hpp"
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

        std::string requiredText(const std::string& field, std::size_t minLength, std::size_t maxLength);
        std::optional<std::string> optionalText(const std::string& field, std::size_t maxLength);
        double requiredNumber(const std::string& field, double minimum, double maximum);
        std::uint64_t requiredIdentifier(const std::string& field);
        std::optional<bool> optionalFlag(const std::string& field);
        std::string trackingCode(const std::string& field);

        /// @brief Lista de entre @p minimum y @p maximum códigos de seguimiento.
        std::vector<std::string> trackingCodes(const std::string& field, std::size_t minimum, std::size_t maximum);

        /**
         * @brief Bultos de un envío: entre 1 y @p maximum objetos con @c weightKg, @c lengthCm, @c widthCm y @c
         * heightCm. Los errores indican la posición del bulto, por ejemplo @c parcels[1].weightKg.
         */
        std::vector<logistics::domain::ParcelData> parcels(const std::string& field, std::size_t maximum);
        std::string email(const std::string& field);

        /// @brief Teléfono: entre 8 y 15 dígitos, con espacios, guiones, paréntesis y un + inicial opcionales.
        std::string phone(const std::string& field);

        /// @brief Código postal argentino: 4 dígitos o formato CPA (una letra, cuatro dígitos y tres letras).
        std::string postalCode(const std::string& field);

        /// @brief Código de catálogo (zona, servicio, característica): 2 a 16 caracteres entre A-Z, 0-9 y guion bajo.
        std::string code(const std::string& field);

        /**
         * @brief Lista (al menos un elemento) de objetos con un texto y un número, por ejemplo las calles de un punto
         * nuevo: @c [{"to":"C1","costKm":3.2}].
         * @param textKey clave del texto (1 a @p maxText caracteres).
         * @param numberKey clave del número (entre 0 y @p maxNumber, excluido el cero).
         */
        std::vector<std::pair<std::string, double>> textNumberPairs(const std::string& field,
                                                                    const std::string& textKey,
                                                                    std::size_t maxText,
                                                                    const std::string& numberKey,
                                                                    double maxNumber);

        /// @brief Lista opcional de códigos de catálogo (vacía si el campo falta).
        std::vector<std::string> optionalCodes(const std::string& field);

        /**
         * @brief Valida que el campo sea uno de los valores admitidos por @p parse.
         * @param field nombre del campo en el JSON.
         * @param parse función que convierte el texto al valor del vocabulario.
         * @param allowed lista legible de valores (para el mensaje de error).
         */
        template<typename TEnum, typename TParser>
        TEnum requiredTerm(const std::string& field, TParser parse, const std::string& allowed)
        {
            const std::string text {requiredText(field, 1, 32)};
            std::optional<TEnum> parsed {parse(text)};
            if (!parsed.has_value() && !text.empty())
            {
                fail(field, "Debe ser uno de: " + allowed);
            }
            return parsed.value_or(TEnum {});
        }

        /// @brief Variante opcional de @c requiredTerm.
        template<typename TEnum, typename TParser>
        std::optional<TEnum> optionalTerm(const std::string& field, TParser parse, const std::string& allowed)
        {
            std::optional<TEnum> result;
            if (has(field))
            {
                result = requiredTerm<TEnum>(field, parse, allowed);
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

        Json m_source;
        std::vector<FieldError> m_errors;
    };

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_HTTP_INPUT_VALIDATOR_HPP
