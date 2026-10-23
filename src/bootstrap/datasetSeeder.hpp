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
 * @brief Datos de arranque para poder usar el sistema desde la primera ejecución.
 *
 * - Si no existe ningún usuario crea admin, operador, pum1 y pum2 (@c @hubflow.local), todos con la contraseña
 *   de arranque configurada.
 * - Si el catálogo de envíos está vacío registra ocho paquetes de ejemplo, uno para cada punto de entrega
 *   C1, N2, S3, C2, N3, S5, C4 y N5, de modo que se pueda armar un lote y una ruta enseguida.
 *
 * Si ya hay datos no toca nada, por lo que reiniciar el servicio nunca duplica ni pisa información.
 */
class DatasetSeeder final
{
public:
    DatasetSeeder(identity::application::RegisterUser&             registerUser,
                  const identity::application::UserRepository&     users,
                  logistics::application::RegisterShipmentUseCase& registerShipment,
                  const logistics::application::LogisticsModel&    logistics,
                  const shared::application::Clock&                clock,
                  std::string                                      seedPassword);

    /// @brief Crea los usuarios iniciales si todavía no hay ninguno.
    void seedUsers();

    /// @brief Registra el dataset inicial si todavía no hay envíos.
    void seedShipments();

private:
    identity::application::RegisterUser&             m_registerUser;
    const identity::application::UserRepository&     m_users;
    logistics::application::RegisterShipmentUseCase& m_registerShipment;
    const logistics::application::LogisticsModel&    m_logistics;
    const shared::application::Clock&                m_clock;
    std::string                                      m_seedPassword;
};

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_DATASET_SEEDER_HPP
