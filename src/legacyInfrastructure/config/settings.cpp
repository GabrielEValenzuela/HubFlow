#include "infrastructure/config/settings.hpp"

#include <cstdlib>
#include <stdexcept>

namespace hubflow::infrastructure
{
namespace
{
constexpr std::size_t MIN_SECRET_LENGTH {32};

std::string environment(const char* pName, const std::string& fallback)
{
    const char* pValue = std::getenv(pName);
    return pValue == nullptr || *pValue == '\0' ? fallback : std::string {pValue};
}

std::uint16_t port(const char* pName, const std::uint16_t fallback)
{
    return static_cast<std::uint16_t>(std::stoul(environment(pName, std::to_string(fallback))));
}
} // namespace

Settings Settings::fromEnvironment()
{
    Settings settings;
    settings.port              = port("HUBFLOW_PORT", settings.port);
    settings.webRoot           = environment("HUBFLOW_WEB_ROOT", settings.webRoot);
    settings.exportDirectory   = environment("HUBFLOW_EXPORT_DIR", settings.exportDirectory);
    settings.jwtSecret         = environment("HUBFLOW_JWT_SECRET", "");
    settings.seedPassword      = environment("HUBFLOW_SEED_PASSWORD", settings.seedPassword);
    settings.database.host     = environment("DB_HOST", settings.database.host);
    settings.database.port     = port("DB_PORT", settings.database.port);
    settings.database.user     = environment("DB_USER", settings.database.user);
    settings.database.password = environment("DB_PASSWORD", settings.database.password);
    settings.database.schema   = environment("DB_NAME", settings.database.schema);
    if (settings.jwtSecret.size() < MIN_SECRET_LENGTH)
    {
        throw std::invalid_argument("HUBFLOW_JWT_SECRET debe tener al menos 32 caracteres");
    }
    return settings;
}

} // namespace hubflow::infrastructure
