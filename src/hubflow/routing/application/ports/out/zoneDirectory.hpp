#ifndef HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ZONE_DIRECTORY_HPP
#define HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ZONE_DIRECTORY_HPP

#include <string>

namespace hubflow::routing::application
{

    /**
     * @brief Consulta qué zonas de reparto existen y están habilitadas, sin conocer el contexto de logística.
     *
     * Quien implemente este puerto resuelve la respuesta en el catálogo de zonas.
     */
    class ZoneDirectory
    {
    public:
        virtual ~ZoneDirectory() = default;

        /// @brief true si la zona existe y está habilitada.
        virtual bool isActive(const std::string& zoneCode) const = 0;
    };

} // namespace hubflow::routing::application

#endif // HUBFLOW_ROUTING_APPLICATION_PORTS_OUT_ZONE_DIRECTORY_HPP
