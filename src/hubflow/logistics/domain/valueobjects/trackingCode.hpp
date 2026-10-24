#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_TRACKING_CODE_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_TRACKING_CODE_HPP

#include <string>

namespace hubflow::logistics::domain
{

    /**
     * @brief Código de seguimiento de un envío: el que se imprime en la etiqueta y se informa al cliente.
     *
     * Admite dos formatos:
     * - Público, @c HF- seguido de ocho símbolos y un verificador (12 caracteres). No es secuencial, así que no permite
     *   adivinar el código de otro envío, y el verificador permite descubrir un error de tipeo sin consultar nada.
     * - Secuencial, @c PKG-NNNN (8 caracteres), el que usa el dataset del trabajo práctico.
     *
     * Dentro de cada formato el largo es fijo, por lo que el orden lexicográfico es consistente y el código sirve
     * directamente como clave del árbol binario, del B+ y de la búsqueda binaria. Entre formatos el orden es el del
     * prefijo (@c HF- antes que @c PKG-). El orden lexicográfico del formato público no coincide con el de alta.
     */
    class TrackingCode final
    {
    public:
        static constexpr std::size_t PUBLIC_LENGTH {12};
        static constexpr std::size_t SEQUENTIAL_LENGTH {8};
        static constexpr const char* PUBLIC_PREFIX {"HF-"};
        static constexpr const char* SEQUENTIAL_PREFIX {"PKG-"};

        /// @throws business_error (Validation) si el formato es inválido o el verificador no coincide.
        explicit TrackingCode(const std::string& value);

        /// @brief Indica si @p value respeta alguno de los formatos sin construir el objeto.
        static bool isValid(const std::string& value);

        /// @brief Indica si @p value es un código público con verificador correcto.
        static bool isPublicFormat(const std::string& value);

        /// @brief Indica si @p value es un código secuencial @c PKG-NNNN.
        static bool isSequentialFormat(const std::string& value);

        const std::string& value() const noexcept;

        bool operator<(const TrackingCode& other) const noexcept;

        bool operator==(const TrackingCode& other) const noexcept;

    private:
        std::string m_value;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_TRACKING_CODE_HPP
