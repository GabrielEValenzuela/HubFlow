#include "bootstrap/datasetSeeder.hpp"

#include "hubflow/shared/application/principal.hpp"

#include <utility>
#include <vector>

namespace hubflow::bootstrap
{
namespace
{
using identity::application::RegisterUserRequest;
using identity::domain::Role;
using logistics::domain::ServiceLevel;
using logistics::domain::ShipmentDraft;
using logistics::domain::Zone;

constexpr const char* OPERATOR_EMAIL {"operador@hubflow.local"};
constexpr const char* SYSTEM_NAME {"Sistema"};

struct SeedUser
{
    const char* email;
    const char* name;
    Role        role;
};

const SeedUser SEED_USERS[] {{"admin@hubflow.local", "Marta Ibáñez", Role::Admin},
                             {OPERATOR_EMAIL, "Laura Gómez", Role::Operador},
                             {"pum1@hubflow.local", "Carlos Medina", Role::Pum},
                             {"pum2@hubflow.local", "Diego Ferreyra", Role::Pum}};

std::vector<ShipmentDraft> initialShipments()
{
    return {
        {"PKG-1001", "Ana Torres", Zone::Centro, 1.20, ServiceLevel::Estandar, "C1"},
        {"PKG-1002", "Bruno Díaz", Zone::Norte, 0.75, ServiceLevel::Express, "N2"},
        {"PKG-1003", "Carla Ruiz", Zone::Sur, 4.10, ServiceLevel::Prioritario, "S3"},
        {"PKG-1004", "Diego López", Zone::Centro, 2.30, ServiceLevel::Estandar, "C2"},
        {"PKG-1005", "Elena Castro", Zone::Norte, 1.90, ServiceLevel::Prioritario, "N3"},
        {"PKG-1006", "Franco Gómez", Zone::Sur, 0.50, ServiceLevel::Express, "S5"},
        {"PKG-1007", "Gabriela Soto", Zone::Centro, 6.20, ServiceLevel::Estandar, "C4"},
        {"PKG-1008", "Hugo Pérez", Zone::Norte, 3.40, ServiceLevel::Prioritario, "N5"},
    };
}
} // namespace

DatasetSeeder::DatasetSeeder(identity::application::RegisterUser&             registerUser,
                             const identity::application::UserRepository&     users,
                             logistics::application::RegisterShipmentUseCase& registerShipment,
                             const logistics::application::LogisticsModel&    logistics,
                             const shared::application::Clock&                clock,
                             std::string                                      seedPassword)
    : m_registerUser {registerUser}
    , m_users {users}
    , m_registerShipment {registerShipment}
    , m_logistics {logistics}
    , m_clock {clock}
    , m_seedPassword {std::move(seedPassword)}
{
}

void DatasetSeeder::seedUsers()
{
    if (m_users.count() == 0)
    {
        const shared::domain::ActorContext system {0, SYSTEM_NAME, m_clock.now()};
        for (const auto& seed : SEED_USERS)
        {
            m_registerUser.bootstrap(RegisterUserRequest {seed.email, seed.name, seed.role, m_seedPassword}, system);
        }
    }
}

void DatasetSeeder::seedShipments()
{
    if (m_logistics.catalog.size() == 0)
    {
        const auto                           operador = m_users.byEmail(OPERATOR_EMAIL);
        const shared::application::Principal author {
            operador.has_value() ? operador->id : 0, SYSTEM_NAME, Role::Operador};
        for (const auto& draft : initialShipments())
        {
            m_registerShipment.execute(author, draft);
        }
    }
}

} // namespace hubflow::bootstrap
