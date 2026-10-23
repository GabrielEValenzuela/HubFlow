#ifndef HUBFLOW_BOOTSTRAP_APPLICATION_HPP
#define HUBFLOW_BOOTSTRAP_APPLICATION_HPP

#include "hubflow/platform/config/settings.hpp"

#include <memory>

namespace hubflow::bootstrap
{

/**
 * @brief Raíz de composición: conecta los casos de uso con sus adaptadores y pone el servicio en marcha.
 *
 * Al crearse arma, en este orden: la conexión a MySQL, los adaptadores de salida (repositorios, hash de
 * contraseñas, tokens, reloj, exportación de planes), los modelos en memoria y los casos de uso. Los
 * suscriptores de eventos se registran en el orden en que deben reaccionar: primero las proyecciones del
 * modelo en memoria, después la persistencia, luego la exportación del plan de ruta y al final la
 * auditoría. Después reconstruye el modelo desde la base de datos, carga los datos iniciales si hacen falta
 * y declara los endpoints REST.
 *
 * Al ejecutarse sirve la API bajo @c /api/v1 y los archivos estáticos de la interfaz web en @c / hasta
 * recibir SIGINT o SIGTERM, momento en que termina de atender el pedido en curso y se detiene.
 */
class Application final
{
public:
    /// @throws std::exception si la base de datos no responde o la configuración es inválida.
    explicit Application(platform::Settings settings);

    ~Application();

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    /// @brief Atiende pedidos hasta que se pida el apagado. @return 0 si terminó con normalidad.
    int run();

private:
    struct Parts;

    std::unique_ptr<Parts> m_spParts;
};

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_APPLICATION_HPP
