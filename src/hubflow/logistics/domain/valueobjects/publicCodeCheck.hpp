#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_PUBLIC_CODE_CHECK_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_PUBLIC_CODE_CHECK_HPP

#include <string>

namespace hubflow::logistics::domain
{

/**
 * @brief Alfabeto y dígito verificador de los códigos de seguimiento públicos (@c HF- seguido de nueve símbolos).
 *
 * El cuerpo son ocho símbolos de un alfabeto de 32 que no confunde letras con números (sin I, L, O ni U). El noveno
 * símbolo es un verificador: la suma de cada símbolo multiplicado por su posición (1 a 8), módulo 37. Detecta un
 * símbolo mal tipeado y dos símbolos cambiados de lugar. Como el módulo es 37 y el alfabeto tiene 32 símbolos, los
 * cuerpos cuyo resto cae entre 32 y 36 no tienen símbolo verificador y el generador los descarta.
 */
class PublicCodeCheck final
{
public:
    static constexpr std::size_t BODY_LENGTH {8};
    static constexpr unsigned    MODULUS {37};
    static constexpr const char* SYMBOLS {"0123456789ABCDEFGHJKMNPQRSTVWXYZ"};
    static constexpr unsigned    SYMBOL_COUNT {32};

    /// @brief Valor de @p symbol en el alfabeto, o -1 si no pertenece.
    static int valueOf(char symbol);

    /// @brief Resto módulo 37 del cuerpo; -1 si el cuerpo no tiene el largo o los símbolos correctos.
    static int remainderOf(const std::string& body);

    /// @brief Símbolo verificador del cuerpo, o el carácter nulo si su resto no tiene símbolo.
    static char checkSymbolOf(const std::string& body);

    /// @brief Indica si @p body seguido de @p check forma un código consistente.
    static bool isConsistent(const std::string& body, char check);
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_PUBLIC_CODE_CHECK_HPP
