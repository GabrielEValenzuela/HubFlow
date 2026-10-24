#ifndef HUBFLOW_BOOTSTRAP_MODULES_IDENTITY_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_MODULES_IDENTITY_MODULE_HPP

#include "bootstrap/boundedContextModule.hpp"
#include "bootstrap/sharedServices.hpp"

#include <memory>

namespace hubflow::routing::application
{
    struct RoutingModel;
}

namespace hubflow::identity::application
{
    class AuthenticateUseCase;
    class UserRepository;
} // namespace hubflow::identity::application

namespace hubflow::identity::domain
{
    class AccessPolicy;
}

namespace hubflow::bootstrap
{

    /**
     * @brief Arma el contexto de identidad: usuarios, sesiones y permisos por rol.
     *
     * Reacciona a los cambios de usuarios guardándolos (etapa de persistencia), hidrata la política de permisos desde
     * la base de datos y, si no hay ningún usuario, crea los de arranque. Entrega a los demás contextos sólo lo que
     * necesitan: el repositorio de usuarios, la política de permisos y el caso de uso que autentica pedidos.
     */
    class IdentityModule final : public BoundedContextModule
    {
    public:
        IdentityModule(const SharedServices& shared, const routing::application::RoutingModel& routing);

        ~IdentityModule() override;

        IdentityModule(const IdentityModule&) = delete;
        IdentityModule& operator=(const IdentityModule&) = delete;

        /// @brief Usuarios registrados, para validar choferes y buscar quién actúa.
        const identity::application::UserRepository& users() const noexcept;

        /// @brief Permisos por rol, para identificar a quien actúa en el sistema.
        const identity::domain::AccessPolicy& policy() const noexcept;

        /// @brief Caso de uso que reconoce al usuario de cada pedido a partir de su token.
        const identity::application::AuthenticateUseCase& authenticator() const noexcept;

    private:
        void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus) override;

        void hydrate() override;

        void seed() override;

        void registerRoutes(platform::ApiRouter& router) override;

        struct Parts;

        std::unique_ptr<Parts> m_spParts;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULES_IDENTITY_MODULE_HPP
