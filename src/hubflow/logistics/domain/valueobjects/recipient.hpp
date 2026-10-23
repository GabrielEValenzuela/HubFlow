#ifndef HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_RECIPIENT_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_RECIPIENT_HPP

#include <string>

namespace hubflow::logistics::domain
{

/// @brief Destinatario del envío: nombre de 2 a 80 caracteres, sin espacios en los extremos.
class Recipient final
{
public:
    /// @throws business_error (Validation) si el nombre no cumple el largo.
    explicit Recipient(const std::string& name);

    const std::string& name() const noexcept;

private:
    std::string m_name;
};

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_VALUEOBJECTS_RECIPIENT_HPP
