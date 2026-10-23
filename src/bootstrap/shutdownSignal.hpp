#ifndef HUBFLOW_BOOTSTRAP_SHUTDOWN_SIGNAL_HPP
#define HUBFLOW_BOOTSTRAP_SHUTDOWN_SIGNAL_HPP

namespace hubflow::bootstrap
{

/**
 * @brief Pedido de apagado ordenado del proceso (Ctrl+C o señal de terminación del orquestador).
 *
 * El manejador de la señal sólo levanta una bandera; quien sirve los pedidos la consulta y cierra el
 * servidor de forma ordenada, dejando terminar el pedido en curso.
 */
class ShutdownSignal final
{
public:
    /// @brief Empieza a escuchar SIGINT y SIGTERM.
    static void install();

    /// @brief Verdadero cuando se pidió terminar el proceso.
    static bool requested() noexcept;
};

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_SHUTDOWN_SIGNAL_HPP
