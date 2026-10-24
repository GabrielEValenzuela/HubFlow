#ifndef HUBFLOW_BOOTSTRAP_INFRASTRUCTURE_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_INFRASTRUCTURE_MODULE_HPP

#include "bootstrap/sharedServices.hpp"

#include "hubflow/platform/config/settings.hpp"

#include <memory>

namespace hubflow::identity::application
{
    class AuthenticateUseCase;
}

namespace hubflow::platform
{
    class ApiRouter;
}

namespace hubflow::bootstrap
{

    /**
     * @brief Piezas técnicas compartidas por todos los contextos y el servidor HTTP que las atiende.
     *
     * Reúne la configuración, la conexión a MySQL, el reloj, el bus de eventos, el hash de contraseñas, el emisor de
     * tokens, el generador de identidades, el registro de claves de idempotencia y el servidor con su router. No
     * conoce ningún contexto de negocio, salvo por el caso de uso que autentica los pedidos.
     */
    class InfrastructureModule final
    {
    public:
        /// @throws std::exception si la base de datos no responde o la configuración es inválida.
        explicit InfrastructureModule(platform::Settings settings);

        ~InfrastructureModule();

        InfrastructureModule(const InfrastructureModule&) = delete;
        InfrastructureModule& operator=(const InfrastructureModule&) = delete;

        /// @brief Referencias a las piezas compartidas para entregar a los módulos de contexto.
        SharedServices services() noexcept;

        /// @brief Crea el router autenticando con @p authenticate y declara la consulta de salud. Una sola vez.
        platform::ApiRouter& openRouter(const identity::application::AuthenticateUseCase& authenticate);

        /// @brief Atiende pedidos hasta que se pida el apagado. @return 0 si terminó con normalidad.
        int serve();

    private:
        struct Parts;

        std::unique_ptr<Parts> m_spParts;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_INFRASTRUCTURE_MODULE_HPP
