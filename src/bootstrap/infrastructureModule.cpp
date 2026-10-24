#include "bootstrap/infrastructureModule.hpp"

#include "bootstrap/shutdownSignal.hpp"

#include "hubflow/identity/adapters/out/security/argon2PasswordHasher.hpp"
#include "hubflow/identity/adapters/out/security/jwtTokenIssuer.hpp"
#include "hubflow/platform/http/apiRouter.hpp"
#include "hubflow/platform/http/healthController.hpp"
#include "hubflow/platform/mysql/mySqlConnection.hpp"
#include "hubflow/shared/adapters/out/generation/uuidV7IdentityGenerator.hpp"
#include "hubflow/shared/adapters/out/mysql/mySqlIdempotencyStore.hpp"
#include "hubflow/shared/adapters/out/system/systemClock.hpp"
#include "hubflow/shared/application/eventBus.hpp"

#include <chrono>
#include <filesystem>
#include <httplib.h>
#include <iostream>
#include <mutex>
#include <thread>

namespace hubflow::bootstrap
{
    namespace
    {
        constexpr std::chrono::milliseconds POLL_INTERVAL {100};
    } // namespace

    struct InfrastructureModule::Parts
    {
        explicit Parts(platform::Settings configuration)
            : settings {std::move(configuration)}
            , connection {settings.database}
            , tokens {settings.jwtSecret}
            , idempotency {connection}
        {
        }

        platform::Settings settings;
        platform::MySqlConnection connection;
        shared::adapters::SystemClock clock;
        shared::application::EventBus bus;
        shared::adapters::UuidV7IdentityGenerator identities;
        identity::adapters::Argon2PasswordHasher hasher;
        identity::adapters::JwtTokenIssuer tokens;
        shared::adapters::MySqlIdempotencyStore idempotency;
        httplib::Server server;
        std::mutex mutex;
        std::unique_ptr<platform::ApiRouter> spRouter;
        std::unique_ptr<platform::Controller> spHealth;
    };

    InfrastructureModule::InfrastructureModule(platform::Settings settings)
        : m_spParts {std::make_unique<Parts>(std::move(settings))}
    {
    }

    InfrastructureModule::~InfrastructureModule() = default;

    SharedServices InfrastructureModule::services() noexcept
    {
        Parts& parts = *m_spParts;
        return SharedServices {
            parts.settings, parts.connection, parts.clock, parts.bus, parts.identities, parts.hasher, parts.tokens};
    }

    platform::ApiRouter&
    InfrastructureModule::openRouter(const identity::application::AuthenticateUseCase& authenticate)
    {
        Parts& parts = *m_spParts;
        parts.spRouter =
            std::make_unique<platform::ApiRouter>(parts.server, authenticate, parts.idempotency, parts.mutex);
        parts.spHealth = std::make_unique<platform::HealthController>(
            [&connection = parts.connection]
            {
                connection.query("SELECT 1");
                return true;
            });
        parts.spHealth->registerRoutes(*parts.spRouter);
        return *parts.spRouter;
    }

    int InfrastructureModule::serve()
    {
        int status {0};
        httplib::Server& server = m_spParts->server;
        const auto& settings = m_spParts->settings;
        if (std::filesystem::is_directory(settings.webRoot))
        {
            server.set_mount_point("/", settings.webRoot);
        }
        if (!server.bind_to_port(settings.bindAddress, settings.port))
        {
            std::cerr << "[hubflow] no se pudo abrir " << settings.bindAddress << ':' << settings.port << '\n';
            status = 1;
        }
        else
        {
            ShutdownSignal::install();
            std::thread listener {[&server] { server.listen_after_bind(); }};
            std::cout << "[hubflow] escuchando en " << settings.bindAddress << ':' << settings.port << std::endl;
            while (!ShutdownSignal::requested())
            {
                std::this_thread::sleep_for(POLL_INTERVAL);
            }
            std::cout << "[hubflow] apagando" << std::endl;
            server.stop();
            listener.join();
        }
        return status;
    }

} // namespace hubflow::bootstrap
