#ifndef HUBFLOW_IDENTITY_DOMAIN_SERVICES_COURIER_WORKLOAD_HPP
#define HUBFLOW_IDENTITY_DOMAIN_SERVICES_COURIER_WORKLOAD_HPP

#include <cstdint>

namespace hubflow::identity::domain
{

    /**
     * @brief Consulta si un chofer tiene trabajo sin terminar.
     *
     * Es un puerto del dominio porque la respuesta vive en el contexto de ruteo: quien lo implemente mira las hojas de
     * ruta. Evita que se desactive a un chofer con un reparto en curso y queden paquetes sin nadie que los lleve.
     */
    class CourierWorkload
    {
    public:
        virtual ~CourierWorkload() = default;

        /// @brief true si el chofer tiene al menos una ruta con paradas pendientes.
        virtual bool hasRouteInProgress(std::uint64_t courierId) const = 0;
    };

} // namespace hubflow::identity::domain

#endif // HUBFLOW_IDENTITY_DOMAIN_SERVICES_COURIER_WORKLOAD_HPP
