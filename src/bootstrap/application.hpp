#ifndef HUBFLOW_BOOTSTRAP_APPLICATION_HPP
#define HUBFLOW_BOOTSTRAP_APPLICATION_HPP

#include "hubflow/platform/config/settings.hpp"

#include <memory>

namespace hubflow::bootstrap
{

    /**
     * @brief Raíz de composición: conecta los contextos de negocio con la infraestructura y pone el servicio en marcha.
     *
     * Al crearse arma primero la infraestructura compartida y el estado en memoria de la planta, y después un módulo
     * por contexto (identidad, auditoría, logística, mesa de clasificación, ruteo, análisis y seguimiento), en ese
     * orden. Los módulos dependen entre sí sólo por los puertos y modelos que reciben al construirse; sumar un
     * contexto es sumar un módulo a la lista.
     *
     * Los suscriptores de eventos se registran por etapas, no por módulo: primero las proyecciones del modelo en
     * memoria, después la persistencia, luego la exportación del plan de ruta, los avisos al cliente y al final la
     * auditoría. Después se reconstruye el modelo desde la base de datos, se cargan los datos iniciales si hacen
     * falta y se declaran los endpoints REST.
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

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /// @brief Atiende pedidos hasta que se pida el apagado. @return 0 si terminó con normalidad.
        int run();

    private:
        struct Parts;

        std::unique_ptr<Parts> m_spParts;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_APPLICATION_HPP
