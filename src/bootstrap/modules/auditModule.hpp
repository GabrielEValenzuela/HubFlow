#ifndef HUBFLOW_BOOTSTRAP_MODULES_AUDIT_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_MODULES_AUDIT_MODULE_HPP

#include "bootstrap/boundedContextModule.hpp"
#include "bootstrap/sharedServices.hpp"

#include <memory>

namespace hubflow::bootstrap
{

    /**
     * @brief Arma el contexto de auditoría: registro de quién hizo qué y consulta de ese registro.
     *
     * Deja constancia de todos los hechos de negocio en la última etapa de reacción, para que sólo se audite lo que
     * ya se aplicó y se guardó.
     */
    class AuditModule final : public BoundedContextModule
    {
    public:
        explicit AuditModule(const SharedServices& shared);

        ~AuditModule() override;

        AuditModule(const AuditModule&) = delete;
        AuditModule& operator=(const AuditModule&) = delete;

    private:
        void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus) override;

        void registerRoutes(platform::ApiRouter& router) override;

        struct Parts;

        std::unique_ptr<Parts> m_spParts;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULES_AUDIT_MODULE_HPP
