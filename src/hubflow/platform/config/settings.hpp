#ifndef HUBFLOW_PLATFORM_CONFIG_SETTINGS_HPP
#define HUBFLOW_PLATFORM_CONFIG_SETTINGS_HPP

#include <cstdint>
#include <string>

namespace hubflow::platform
{

/// @brief Parámetros de conexión a MySQL.
struct DatabaseSettings
{
    std::string   host {"127.0.0.1"};
    std::uint16_t port {3306};
    std::string   user {"hubflow"};
    std::string   password {"hubflow"};
    std::string   schema {"hubflow"};
    unsigned      connectRetries {30};
};

/// @brief Configuración del proceso, leída de variables de entorno.
struct Settings
{
    std::string      bindAddress {"0.0.0.0"};
    std::uint16_t    port {8080};
    std::string      webRoot {"web"};
    std::string      exportDirectory {"exports"};
    std::string      jwtSecret;
    std::string      seedPassword {"hubflow123"};
    std::string      trackingCodes {"public"};
    DatabaseSettings database;

    /**
     * @brief Lee HUBFLOW_PORT, HUBFLOW_WEB_ROOT, HUBFLOW_EXPORT_DIR, HUBFLOW_JWT_SECRET,
     * HUBFLOW_SEED_PASSWORD, HUBFLOW_TRACKING_CODES (@c public o @c sequential, por defecto @c public) y DB_HOST,
     * DB_PORT, DB_USER, DB_PASSWORD, DB_NAME.
     * @throws std::invalid_argument si falta el secreto JWT o tiene menos de 32 caracteres, o si
     * HUBFLOW_TRACKING_CODES no es @c public ni @c sequential.
     */
    static Settings fromEnvironment();
};

} // namespace hubflow::platform

#endif // HUBFLOW_PLATFORM_CONFIG_SETTINGS_HPP
