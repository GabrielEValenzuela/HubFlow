#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_RECIPIENT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_RECIPIENT_HPP

#include "hubflow/logistics/domain/valueobjects/phone.hpp"
#include "hubflow/logistics/domain/valueobjects/postalCode.hpp"

#include <string>

namespace hubflow::logistics::domain
{

    /**
     * @brief Destinatario del envío: nombre de 2 a 80 caracteres sin espacios en los extremos, teléfono y código
     * postal.
     *
     * El teléfono y el código postal permiten avisarle y confirmar su identidad cuando consulta el paquete.
     */
    class Recipient final
    {
    public:
        /// @throws BusinessError (Validation) si el nombre, el teléfono o el código postal no son válidos.
        Recipient(const std::string& name, const std::string& phone, const std::string& postalCode);

        const std::string& name() const noexcept;
        const Phone& phone() const noexcept;
        const PostalCode& postalCode() const noexcept;

        /// @brief Primer nombre (lo que está antes del primer espacio).
        std::string firstName() const;

    private:
        std::string m_name;
        Phone m_phone;
        PostalCode m_postalCode;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_RECIPIENT_HPP
