#include "hubflow/tracking/application/trackingErrors.hpp"

namespace hubflow::tracking::application
{

    shared::domain::BusinessError TrackingErrors::notFound()
    {
        return shared::domain::BusinessError {shared::domain::ErrorKind::NotFound,
                                              "tracking.not_found",
                                              "No encontramos un paquete con esos datos. Revisá el código y lo que "
                                              "ingresaste."};
    }

    shared::domain::BusinessError TrackingErrors::verificationRequired()
    {
        return shared::domain::BusinessError {shared::domain::ErrorKind::Unauthorized,
                                              "tracking.verification_required",
                                              "Para ver el detalle confirmá tus datos de nuevo."};
    }

    shared::domain::BusinessError TrackingErrors::tooManyAttempts(const std::uint32_t retryAfterSeconds)
    {
        return shared::domain::BusinessError {shared::domain::ErrorKind::RateLimited,
                                              "tracking.too_many_attempts",
                                              "Hiciste varios intentos seguidos. Esperá unos minutos y volvé a probar.",
                                              retryAfterSeconds};
    }

} // namespace hubflow::tracking::application
