#ifndef HUBFLOW_SHARED_APPLICATION_USE_CASE_HPP
#define HUBFLOW_SHARED_APPLICATION_USE_CASE_HPP

namespace hubflow::shared::application
{

/**
 * @brief Marca cada caso de uso según qué pasa si el mismo pedido se ejecuta más de una vez.
 *
 * Idempotent: repetir el pedido deja el sistema en el mismo estado que ejecutarlo una vez.
 * NonIdempotent: cada ejecución produce un efecto nuevo (otro recurso, otro movimiento, otro
 * token); el borde HTTP lo protege con un Idempotency-Key cuando el cliente puede reintentar.
 */
enum class Idempotency
{
    Idempotent,
    NonIdempotent
};

} // namespace hubflow::shared::application

#endif // HUBFLOW_SHARED_APPLICATION_USE_CASE_HPP
