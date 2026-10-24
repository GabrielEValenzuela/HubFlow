#ifndef HUBFLOW_TRACKING_DOMAIN_SERVICES_VERIFICATION_PROOF_HPP
#define HUBFLOW_TRACKING_DOMAIN_SERVICES_VERIFICATION_PROOF_HPP

#include <string>

namespace hubflow::tracking::domain
{

/// @brief Dato con el que alguien dice ser quien espera el paquete; lo que no informó queda vacío.
struct VerificationProof
{
    std::string phoneLast4;
    std::string postalCode;

    bool isEmpty() const noexcept
    {
        return phoneLast4.empty() && postalCode.empty();
    }
};

/**
 * @brief Decide si lo que informó el cliente coincide con los datos del destinatario.
 *
 * Todo lo que se informe tiene que coincidir (si manda teléfono y código postal, los dos) y debe haber al menos un
 * dato. El código postal se compara sin distinguir mayúsculas ni espacios. Las comparaciones tardan lo mismo
 * acierten o fallen en un carácter, para que el tiempo de respuesta no delate qué parte coincidió.
 */
class ProofCheck final
{
public:
    static bool matches(const VerificationProof& proof,
                        const std::string&       expectedPhoneLast4,
                        const std::string&       expectedPostalCode);

    /// @brief Mayúsculas y sin espacios.
    static std::string normalizePostalCode(const std::string& text);
};

} // namespace hubflow::tracking::domain

#endif // HUBFLOW_TRACKING_DOMAIN_SERVICES_VERIFICATION_PROOF_HPP
