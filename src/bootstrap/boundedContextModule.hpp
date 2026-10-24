#ifndef HUBFLOW_BOOTSTRAP_BOUNDED_CONTEXT_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_BOUNDED_CONTEXT_MODULE_HPP

#include <array>
#include <utility>
#include <vector>

namespace hubflow::shared::application
{
    class EventBus;
}

namespace hubflow::shared::domain
{
    class DomainEventVisitor;
}

namespace hubflow::platform
{
    class ApiRouter;
}

namespace hubflow::bootstrap
{

    /**
     * @brief Etapa de reacción a un hecho de negocio.
     *
     * Los hechos se reparten siempre en este orden: primero se actualizan las proyecciones en memoria (para que
     * cualquier lectura posterior ya vea el cambio), después se persiste, luego se exporta el plan de ruta, se avisa
     * al cliente y, al final, se deja constancia en la auditoría. Cada contexto declara en qué etapa reacciona; el
     * orden entre contextos dentro de una misma etapa es el orden en que se registraron los módulos.
     */
    enum class SubscriptionPhase
    {
        Projection,
        Persistence,
        Export,
        CustomerNotice,
        Audit
    };

    /// @brief Las etapas de reacción en el orden exacto en que se reparten los hechos.
    constexpr std::array<SubscriptionPhase, 5> SUBSCRIPTION_ORDER {SubscriptionPhase::Projection,
                                                                   SubscriptionPhase::Persistence,
                                                                   SubscriptionPhase::Export,
                                                                   SubscriptionPhase::CustomerNotice,
                                                                   SubscriptionPhase::Audit};

    /// @brief Suscriptores de un módulo agrupados por etapa; conserva el orden de alta dentro de cada etapa.
    class PhasedSubscribers final
    {
    public:
        /// @brief Declara que @p subscriber reacciona en la etapa @p phase. Debe vivir tanto como el módulo.
        void add(SubscriptionPhase phase, shared::domain::DomainEventVisitor& subscriber);

        /// @brief Suscribe al bus los reactores de la etapa @p phase, en el orden en que se declararon.
        void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus) const;

    private:
        std::vector<std::pair<SubscriptionPhase, shared::domain::DomainEventVisitor*>> m_entries;
    };

    /**
     * @brief Pieza de arranque de un contexto de negocio.
     *
     * Cada contexto arma sus propios adaptadores, casos de uso, reactores y controladores y los expone a través de
     * esta interfaz. La raíz de composición sólo recorre la lista de módulos, de modo que sumar un contexto no
     * obliga a tocar a los demás.
     */
    class BoundedContextModule
    {
    public:
        virtual ~BoundedContextModule() = default;

        /// @brief Registra en el bus los reactores del contexto que corresponden a la etapa @p phase. Por defecto
        /// ninguno.
        virtual void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus);

        /// @brief Reconstruye el estado en memoria del contexto desde la base de datos. Por defecto no hace nada.
        virtual void hydrate();

        /// @brief Carga los datos iniciales del contexto si todavía no existen. Por defecto no hace nada.
        virtual void seed();

        /// @brief Declara los endpoints REST del contexto.
        virtual void registerRoutes(platform::ApiRouter& router) = 0;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_BOUNDED_CONTEXT_MODULE_HPP
