#include "hubflow/platform/config/settings.hpp"

#include <cstdlib>
#include <stdexcept>

namespace hubflow::platform
{
    namespace
    {
        constexpr std::size_t MIN_SECRET_LENGTH {32};

        std::string environmentValue(const char* pName, const std::string& fallback)
        {
            const char* pValue = std::getenv(pName);
            return pValue == nullptr || *pValue == '\0' ? fallback : std::string {pValue};
        }

        std::uint16_t portFrom(const char* pName, const std::uint16_t fallback)
        {
            return static_cast<std::uint16_t>(std::stoul(environmentValue(pName, std::to_string(fallback))));
        }
    } // namespace

    Settings Settings::fromEnvironment()
    {
        Settings settings;
        settings.port = portFrom("HUBFLOW_PORT", settings.port);
        settings.webRoot = environmentValue("HUBFLOW_WEB_ROOT", settings.webRoot);
        settings.exportDirectory = environmentValue("HUBFLOW_EXPORT_DIR", settings.exportDirectory);
        settings.jwtSecret = environmentValue("HUBFLOW_JWT_SECRET", "");
        settings.seedPassword = environmentValue("HUBFLOW_SEED_PASSWORD", settings.seedPassword);
        settings.trackingCodes = environmentValue("HUBFLOW_TRACKING_CODES", settings.trackingCodes);
        settings.publicBaseUrl = environmentValue("HUBFLOW_PUBLIC_BASE_URL", settings.publicBaseUrl);
        settings.database.host = environmentValue("DB_HOST", settings.database.host);
        settings.database.port = portFrom("DB_PORT", settings.database.port);
        settings.database.user = environmentValue("DB_USER", settings.database.user);
        settings.database.password = environmentValue("DB_PASSWORD", settings.database.password);
        settings.database.schema = environmentValue("DB_NAME", settings.database.schema);
        if (settings.jwtSecret.size() < MIN_SECRET_LENGTH)
        {
            throw std::invalid_argument("HUBFLOW_JWT_SECRET debe tener al menos 32 caracteres");
        }
        if (settings.trackingCodes != "public" && settings.trackingCodes != "sequential")
        {
            throw std::invalid_argument("HUBFLOW_TRACKING_CODES debe ser public o sequential");
        }
        while (!settings.publicBaseUrl.empty() && settings.publicBaseUrl.back() == '/')
        {
            settings.publicBaseUrl.pop_back();
        }
        return settings;
    }

    std::string Settings::trackingLinkBase() const
    {
        return publicBaseUrl + "/seguimiento/";
    }

} // namespace hubflow::platform
