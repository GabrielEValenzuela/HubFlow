#ifndef HUBFLOW_SHARED_DOMAIN_BUSINESS_ERROR_HPP
#define HUBFLOW_SHARED_DOMAIN_BUSINESS_ERROR_HPP

#include <cstdint>
#include <stdexcept>
#include <string>

namespace hubflow::shared::domain
{

    /// @brief Categoría de una violación de regla de negocio, independiente del transporte.
    enum class ErrorKind : std::uint8_t
    {
        Validation,
        NotFound,
        Conflict,
        InvalidState,
        Unauthorized,
        Forbidden,
        RateLimited
    };

    /**
     * @brief Excepción de negocio.
     *
     * @c code() es un identificador estable para clientes (p. ej. "shipment.duplicate") y
     * @c what() el mensaje legible en español. Un error de tipo RateLimited indica además cuántos segundos hay que
     * esperar antes de volver a intentar.
     */
    class BusinessError final : public std::runtime_error
    {
    public:
        BusinessError(ErrorKind kind, std::string code, const std::string& message);

        /// @brief Error de límite de intentos: informa cuántos segundos esperar antes de reintentar.
        BusinessError(ErrorKind kind, std::string code, const std::string& message, std::uint32_t retryAfterSeconds);

        ErrorKind errorKind() const noexcept;

        const std::string& code() const noexcept;

        /// @brief Segundos de espera sugeridos; 0 si el error no los indica.
        std::uint32_t retryAfterSeconds() const noexcept;

    private:
        ErrorKind m_kind;
        std::string m_code;
        std::uint32_t m_retryAfterSeconds {0};
    };

} // namespace hubflow::shared::domain

#endif // HUBFLOW_SHARED_DOMAIN_BUSINESS_ERROR_HPP
