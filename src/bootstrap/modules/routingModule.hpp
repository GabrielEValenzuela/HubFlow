#ifndef HUBFLOW_BOOTSTRAP_MODULES_ROUTING_MODULE_HPP
#define HUBFLOW_BOOTSTRAP_MODULES_ROUTING_MODULE_HPP

#include "bootstrap/boundedContextModule.hpp"
#include "bootstrap/sharedServices.hpp"

#include <memory>

namespace hubflow::logistics::application
{
    struct LogisticsModel;
}

namespace hubflow::routing::application
{
    struct RoutingModel;
}

namespace hubflow::sorting::domain
{
    class SortingDesk;
}

namespace hubflow::identity::application
{
    class UserRepository;
}

namespace hubflow::bootstrap
{

    /// @brief Lo que ruteo necesita de otros contextos: los envíos de logística, la mesa y los usuarios.
    struct RoutingNeighbours
    {
        // cppcheck-suppress-begin uninitMemberVarNoCtor
        logistics::application::LogisticsModel& logistics;
        sorting::domain::SortingDesk& desk;
        const identity::application::UserRepository& users;
        // cppcheck-suppress-end uninitMemberVarNoCtor
    };

    /**
     * @brief Arma el contexto de ruteo: mapa vial, búsqueda de caminos, rutas de reparto y su resolución parada a
     * parada.
     *
     * Guarda rutas y mapa (etapa de persistencia), exporta el plan de cada ruta asignada (etapa de exportación) y
     * reconstruye mapa y rutas desde la base de datos al iniciar.
     */
    class RoutingModule final : public BoundedContextModule
    {
    public:
        RoutingModule(const SharedServices& shared,
                      routing::application::RoutingModel& model,
                      const RoutingNeighbours& neighbours);

        ~RoutingModule() override;

        RoutingModule(const RoutingModule&) = delete;
        RoutingModule& operator=(const RoutingModule&) = delete;

    private:
        void subscribe(SubscriptionPhase phase, shared::application::EventBus& bus) override;

        void hydrate() override;

        void registerRoutes(platform::ApiRouter& router) override;

        struct Parts;

        std::unique_ptr<Parts> m_spParts;
    };

} // namespace hubflow::bootstrap

#endif // HUBFLOW_BOOTSTRAP_MODULES_ROUTING_MODULE_HPP
