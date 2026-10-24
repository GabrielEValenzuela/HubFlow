#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_PHONE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_PHONE_HPP

#include <string>

namespace hubflow::logistics::domain
{

    /**
     * @brief Teléfono de contacto del destinatario.
     *
     * Se escribe como lo escribe la gente (con espacios, guiones, paréntesis y un signo + inicial) y se guarda sólo con
     * sus dígitos, que deben ser entre 8 y 15. Sirve para avisar al cliente y para confirmar que quien consulta un
     * paquete es quien lo espera, mediante sus últimos cuatro dígitos.
     */
    class Phone final
    {
    public:
        static constexpr std::size_t MIN_DIGITS {8};
        static constexpr std::size_t MAX_DIGITS {15};
        static constexpr std::size_t PROOF_DIGITS {4};

        /// @throws BusinessError (Validation) si no tiene entre 8 y 15 dígitos o trae caracteres que no son de un
        /// teléfono.
        explicit Phone(const std::string& text);

        /// @brief true si @p text es un teléfono aceptable.
        static bool accepts(const std::string& text);

        /// @brief true si @p text son exactamente cuatro dígitos.
        static bool isProof(const std::string& text);

        const std::string& digits() const noexcept;

        /// @brief Los últimos cuatro dígitos.
        std::string lastFour() const;

        /// @brief Forma que se puede mostrar o registrar sin revelar el número: sólo los últimos cuatro dígitos.
        std::string masked() const;

    private:
        std::string m_digits;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_PHONE_HPP
