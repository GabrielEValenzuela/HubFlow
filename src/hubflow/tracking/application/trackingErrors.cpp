#include "hubflow/tracking/application/trackingErrors.hpp"

namespace hubflow::tracking::application
{

shared::domain::business_error TrackingErrors::notFound()
{
    return shared::domain::business_error {shared::domain::ErrorKind::NotFound,
                                           "tracking.not_found",
                                           "No encontramos un paquete con esos datos. Revisá el código y lo que "
                                           "ingresaste."};
}

shared::domain::business_error TrackingErrors::verificationRequired()
{
    return shared::domain::business_error {shared::domain::ErrorKind::Unauthorized,
                                           "tracking.verification_required",
                                           "Para ver el detalle confirmá tus datos de nuevo."};
}

shared::domain::business_error TrackingErrors::tooManyAttempts(const std::uint32_t retryAfterSeconds)
{
    return shared::domain::business_error {shared::domain::ErrorKind::RateLimited,
                                           "tracking.too_many_attempts",
                                           "Hiciste varios intentos seguidos. Esperá unos minutos y volvé a probar.",
                                           retryAfterSeconds};
}

} // namespace hubflow::tracking::application
