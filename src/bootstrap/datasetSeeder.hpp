#ifndef HUBFLOW_BOOTSTRAP_DATASET_SEEDER_HPP
#define HUBFLOW_BOOTSTRAP_DATASET_SEEDER_HPP

#include "hubflow/identity/application/ports/out/userRepository.hpp"
#include "hubflow/identity/application/usecases/commands/registerUser.hpp"
#include "hubflow/logistics/application/logisticsModel.hpp"
#include "hubflow/logistics/application/ports/in/registerShipmentUseCase.hpp"

#include <string>

namespace hubflow::bootstrap
{

    /**
     * @brief Usuarios de arranque para poder iniciar sesión desde la primera ejecución.
     *
     * Si no existe ningún usuario crea admin, operador, pum1 y pum2 (dominio hubflow.local), todos con la contraseña
     * de arranque configurada. Si ya hay usuarios no toca nada, por lo que reiniciar el servicio nunca duplica ni
     * pisa información.
     */
    class UserSeeder final
    {
    public:
        UserSeeder(identity::application::RegisterUser& registerUser,
                   const identity::application::UserRepository& users,
                   const shared::application::Clock& clock,
                   std::string seedPassword);

        /// @brief Crea los usuarios iniciales si todavía no hay ninguno.
        void seedUsers();

    private:
        identity::application::RegisterUser& m_registerUser;
        const identity::application::UserRepository& m_users;
        const shared::application::Clock& m_clock;
        std::string m_seedPassword;
    };

    /// @brief Colaboradores que necesita el registro del dataset inicial de envíos.
    struct ShipmentSeedCollaborators
    {
        const identity::application::UserRepository& users;
        const identity::domain::AccessPolicy& policy;
        const logistics::application::LogisticsModel& logistics;
        const shared::application::Clock& clock;
    };

    /**
     * @brief Envíos de ejemplo para poder armar un lote y una ruta enseguida.
     *
     * Si el catálogo de envíos está vacío registra ocho paquetes de ejemplo, uno para cada punto de entrega
     * C1, N2, S3, C2, N3, S5, C4 y N5. Llevan los códigos fijos PKG-1001 a PKG-1008 del trabajo práctico, sea cual
     * sea el formato de código elegido para los nuevos.
     *
     * Las zonas, niveles de servicio, características y permisos por rol iniciales no se cargan acá: los siembra la
     * migración del esquema, porque son datos de configuración y no del dataset de ejemplo.
     *
     * Si ya hay envíos no toca nada.
     */
    class ShipmentSeeder final
    {
    public:
        ShipmentSeeder(logistics::application::RegisterShipmentUseCase& registerShipment,
                       const ShipmentSeedCollaborators& collaborators);

        /// @brief Registra el dataset inicial si todavía no hay envíos.
        void seedShipments();

    private:
        logistics::application::RegisterShipmentUseCase& m_registerShipment;
        ShipmentSeedCollaborators m_collaborators;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_DATASET_SEEDER_HPP
