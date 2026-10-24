#ifndef HUBFLOW_TRACKING_APPLICATION_TRACKING_ERRORS_HPP
#define HUBFLOW_TRACKING_APPLICATION_TRACKING_ERRORS_HPP

#include "hubflow/shared/domain/businessError.hpp"

namespace hubflow::tracking::application
{

    /**
     * @brief Errores del seguimiento público, redactados para que no sirvan para adivinar qué códigos existen.
     *
     * Un código inexistente, un código mal escrito y un dato de verificación equivocado producen exactamente el mismo
     * error, con el mismo código y el mismo mensaje.
     */
    class TrackingErrors final
    {
    public:
        /// @brief "No encontramos un paquete con esos datos": para código inexistente y verificación fallida por igual.
        static shared::domain::business_error notFound();

        /// @brief Hace falta verificar antes de ver el detalle: falta la constancia, es inválida, venció o es de otro
        /// paquete.
        static shared::domain::business_error verificationRequired();

        /// @brief Demasiados intentos; conviene esperar @p retryAfterSeconds.
        static shared::domain::business_error tooManyAttempts(std::uint32_t retryAfterSeconds);
    };

} // namespace hubflow::tracking::application

#endif // HUBFLOW_TRACKING_APPLICATION_TRACKING_ERRORS_HPP
