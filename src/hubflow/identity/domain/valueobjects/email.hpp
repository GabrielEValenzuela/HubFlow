#ifndef HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_EMAIL_HPP
#define HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_EMAIL_HPP

#include <string>

namespace hubflow::identity::domain
{

    /// @brief Email normalizado a minúsculas con formato local@dominio.tld (máx. 120 caracteres).
    class Email final
    {
    public:
        /// @throws BusinessError (Validation) si el formato es inválido.
        explicit Email(const std::string& value);

        const std::string& value() const noexcept;

    private:
        std::string m_value;
    };

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_VALUEOBJECTS_EMAIL_HPP
