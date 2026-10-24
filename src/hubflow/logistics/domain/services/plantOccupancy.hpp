#ifndef HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PLANT_OCCUPANCY_HPP
#define HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PLANT_OCCUPANCY_HPP

#include <string>

namespace hubflow::logistics::domain
{

    /**
     * @brief Qué hay hoy en la operación que impide dar de baja una zona o un nivel de servicio.
     *
     * Es un puerto del dominio porque la respuesta involucra a más de un contexto: los paquetes que esperan en la
     * planta son de logística, y las rutas en curso y los puntos del mapa son de ruteo.
     */
    class PlantOccupancy
    {
    public:
        virtual ~PlantOccupancy() = default;

        /// @brief true si hay paquetes esperando en la planta destinados a la zona.
        virtual bool hasShipmentsInZone(const std::string& zoneCode) const = 0;

        /// @brief true si hay paquetes esperando en la planta con ese nivel de servicio.
        virtual bool hasShipmentsWithService(const std::string& serviceCode) const = 0;

        /// @brief true si hay un reparto en curso con paradas pendientes en la zona.
        virtual bool hasRoutesInProgressInZone(const std::string& zoneCode) const = 0;

        /// @brief true si la zona tiene puntos del mapa activos asignados.
        virtual bool hasActiveMapNodesInZone(const std::string& zoneCode) const = 0;
    };

} // namespace hubflow::logistics::domain

#endif // HUBFLOW_LOGISTICS_DOMAIN_SERVICES_PLANT_OCCUPANCY_HPP
