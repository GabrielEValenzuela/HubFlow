#ifndef HUBFLOW_SHARED_DOMAIN_BUSINESS_ERROR_HPP
#define HUBFLOW_SHARED_DOMAIN_BUSINESS_ERROR_HPP

#include <stdexcept>
#include <string>

namespace hubflow::shared::domain
{

/// @brief Categoría de una violación de regla de negocio, independiente del transporte.
enum class ErrorKind
{
    Validation,
    NotFound,
    Conflict,
    InvalidState,
    Unauthorized,
    Forbidden
};

/**
 * @brief Excepción de negocio.
 *
 * @c code() es un identificador estable para clientes (p. ej. "shipment.duplicate") y
 * @c what() el mensaje legible en español.
 */
class business_error final : public std::runtime_error
{
public:
    business_error(ErrorKind kind, std::string code, const std::string& message);

    ErrorKind kind() const noexcept;

    const std::string& code() const noexcept;

private:
    ErrorKind   m_kind;
    std::string m_code;
};

} // namespace hubflow::shared::domain

#endif // HUBFLOW_SHARED_DOMAIN_BUSINESS_ERROR_HPP
