#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_POSTAL_CODE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_POSTAL_CODE_HPP

#include <string>

namespace hubflow::logistics::domain
{

    /**
     * @brief Código postal argentino del destinatario: el numérico de cuatro dígitos (1425) o el CPA completo, una
     * letra, cuatro dígitos y tres letras (C1425ABC).
     *
     * Se guarda en mayúsculas y sin espacios. Es el segundo dato con el que un cliente confirma que es quien espera el
     * paquete.
     */
    class PostalCode final
    {
    public:
        /// @throws business_error (Validation) si no tiene ninguno de los dos formatos.
        explicit PostalCode(const std::string& text);

        /// @brief true si @p text es un código postal aceptable.
        static bool accepts(const std::string& text);

        /// @brief Texto normalizado: mayúsculas y sin espacios.
        static std::string normalize(const std::string& text);

        const std::string& value() const noexcept;

    private:
        std::string m_value;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_POSTAL_CODE_HPP
